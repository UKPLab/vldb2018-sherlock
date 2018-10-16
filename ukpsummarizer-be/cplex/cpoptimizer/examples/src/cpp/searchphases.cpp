// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/searchphases.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2014. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

How to write custom search phases
---------------------------------

------------------------------------------------------------ */

#include <ilcp/cpext.h>

// Custom variable evaluator
//
// Example: variable is evaluated according to the average of
//          the bounds of the variable

IloNum CalcCentroid(IloCPEngine cp, IloIntVar var) {
  return 0.5 * (cp.getMin(var) + cp.getMax(var));
}

class CentroidI : public IloIntVarEvalI {
public:
  CentroidI(IloEnv env) : IloIntVarEvalI(env) { }
  IloNum eval(IloCPEngine cp, IloIntVar var) { return CalcCentroid(cp, var); }
};
IloIntVarEval Centroid(IloEnv env) { return new (env) CentroidI(env); }

// Custom variable chooser
//
// Example: choose the variable with the smallest centroid
//          Note that this class is merely for instruction
//          In reality, it would be easier to use the above
//          evaluator class and write:
//            IloSelectSmallest(Centroid(env))
//          rather than:
//            ChooseSmallestCentroid(env)
//
class ChooseSmallestCentroidI : public IloIntVarChooserI {
public:
  ChooseSmallestCentroidI(IloEnv env) : IloIntVarChooserI(env) { }
  IloInt choose(IloCPEngine cp, IloIntVarArray vars) {
    IloNum best = IloInfinity;
    IloInt bestIndex = -1;
    IloInt n = vars.getSize();
    for (IloInt i = 0; i < n; i++) {
      if (!cp.isFixed(vars[i])) {
        IloNum c = CalcCentroid(cp, vars[i]);
        if (c < best) {
          best = c;
          bestIndex = i;
        }
      }
    }
    return bestIndex;
  }
};
IloIntVarChooser ChooseSmallestCentroid(IloEnv env) {
  return new (env) ChooseSmallestCentroidI(env);
}

// Custom value evaluator
//
// Example: value's evaluation is its distance
//          from the centroid
class DistanceFromCentroidI : public IloIntValueEvalI {
public:
  DistanceFromCentroidI(IloEnv env) : IloIntValueEvalI(env) { }
  IloNum eval(IloCPEngine cp, IloIntVar var, IloInt value) {
    return IloAbs(CalcCentroid(cp, var) - value);
  }
};
IloIntValueEval DistanceFromCentroid(IloEnv env) {
  return new (env) DistanceFromCentroidI(env);
}

// Custom value chooser
//
// Example: choose the value with the smallest distance to
//          the centroid.

// Custom value chooser
class ChooseSmallestDistanceFromCentroidI : public IloIntValueChooserI {
public:
  ChooseSmallestDistanceFromCentroidI(IloEnv env)
    : IloIntValueChooserI(env) { }
  IloInt choose(IloCPEngine cp, IloIntVarArray vars, IloInt i) {
    IloIntVar var = vars[i];
    IloNum best = IloInfinity;
    IloInt bestValue = cp.getMin(var);
    IloNum centroid = CalcCentroid(cp, var);
    for (IloCPEngine::IntVarIterator it(cp, var); it.ok(); ++it) {
      IloNum eval = IloAbs(centroid - *it);
      if (eval < best) {
        best      = eval;
        bestValue = *it;
      }
    }
    return bestValue;
  }
};
IloIntValueChooser ChooseSmallestDistanceFromCentroid(IloEnv env) {
  return new (env) ChooseSmallestDistanceFromCentroidI(env);
}

int main(int, const char * []) {
  IloEnv env;
  try {
    IloIntVarArray x(env);
    for (IloInt i = 0; i < 10; i++) {
      char name[6];
      sprintf(name, "X%ld", (long)i);
      x.add(IloIntVar(env, 0, 100 - 2*(i / 2), name));
    }
    IloModel mdl(env);
    mdl.add(IloAllDiff(env, x));
    mdl.add(x);

    IloIntVarChooser varChooser   = ChooseSmallestCentroid(env);
    IloIntValueChooser valChooser = ChooseSmallestDistanceFromCentroid(env);
    IloSearchPhase sp1(env, x, varChooser, valChooser);

    IloIntVarEval   varEval       = Centroid(env);
    IloIntValueEval valEval       = DistanceFromCentroid(env);
    IloSearchPhase sp2(env, x, IloSelectSmallest(varEval),
                               IloSelectSmallest(valEval));

    // sp2 can have ties as two variable or values could evaluate
    // to the same values.  sp3 shows how to break these ties
    // choosing, for equivalent centroid and distance-to-centroid
    // evaluations, the lowest indexed variable in x and the
    // lowest value.
    IloVarSelectorArray selVar(env);
    selVar.add(IloSelectSmallest(varEval));
    selVar.add(IloSelectSmallest(IloVarIndex(env, x))); // break ties on index

    IloValueSelectorArray selValue(env);
    selValue.add(IloSelectSmallest(valEval));
    selValue.add(IloSelectSmallest(IloValue(env))); // break ties on smallest

    IloSearchPhase sp3(env, x, selVar, selValue);

    IloCP cp(mdl);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    cp.setParameter(IloCP::LogPeriod, 1);
    cp.out() << "Choosers" << std::endl;
    cp.setSearchPhases(sp1);
    cp.solve();
    cp.out() << cp.domain(x) << std::endl;

    cp.out() << "Evaluators" << std::endl;
    cp.setSearchPhases(sp2);
    cp.solve();
    cp.out() << cp.domain(x) << std::endl;
    cp.out() << "Evaluators (with tie-break)" << std::endl;
    cp.setSearchPhases(sp3);
    cp.solve();
    cp.out() << cp.domain(x) << std::endl;

    cp.end();
  } catch (IloException & ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
