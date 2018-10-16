// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/networkcfg.cpp
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

Problem Description
-------------------
The problem is to configure a network such that:
- a node is either a supplier or a normal node
- k suppliers are needed
- an arc between nodes i and j can exist only when exactly one of i or j is
  a supplier
- the network is connected
- the network need to respect an upper bound on the distances between some nodes
- the network must have exactly numArcs arcs

Only some arcs are possible and the cost associated to each arc are known.
Some additional constraints have the form "arc1 or arc2", and force at
least one arc to be present in the solution.

------------------------------------------------------------ */

#include <ilcp/cpext.h>

const IloInt CostRescalingRatio = 6400;
const IloInt NumSkipped = 500;
typedef enum { AUTO = 0, MANUAL = 1, CONSTRAINT = 2} Mode;

typedef IloArray<IloIntVarArray> IloIntVarArray2;

IlcIntVar ** MakeIlcMatrix(IloCPEngine cp, IloIntVarArray2 m) {
  IlcInt dim = m.getSize();
  IlcIntVar ** x = new (cp.getHeap()) IlcIntVar * [dim];
  for (IlcInt i = 0; i < dim; i++) {
    IlcInt dim2 = m[i].getSize();
    x[i] = new (cp.getHeap()) IlcIntVar[dim2];
    for (IlcInt j = 0; j < dim2; j++) {
      x[i][j] = cp.getIntVar(m[i][j]);
    }
  }
  return x;
}

IlcInt ** MakeIlcMatrix(IloCPEngine cp, IloIntArray2 m) {
  IlcInt dim = m.getSize();
  IlcInt ** x = new (cp.getHeap()) IlcInt * [dim];
  for (IlcInt i = 0; i < dim; i++) {
    IlcInt dim2 = m[i].getSize();
    x[i] = new (cp.getHeap()) IlcInt[dim2];
    for (IlcInt j = 0; j < dim2; j++) {
      x[i][j] = m[i][j];
    }
  }
  return x;
}

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

//------ A custom inferencer for maintaining the distance constraint ----------

// A custom inferencer is a subclass of IlcCustomInferencerI

// Ensure that the graph given by aa satisfies:
//   for all i,j dist[i][j] = min distance between i and j
class IlcMinDistanceInferencerI: public IlcCustomInferencerI {
  IlcInt       _n;
  IlcIntVar ** _a;
  IlcIntVar ** _dist;
  IlcInt     **_d;
public:
  IlcMinDistanceInferencerI(
    IloCPEngine cp,
    IlcInt n,
    IlcIntVar ** aa,
    IlcIntVar ** ddist, 
    IlcBool manual,
    IlcInt skipped
  ) : IlcCustomInferencerI(cp, manual, skipped), _n(n), _a(aa), _dist(ddist) {
    _d = new (cp.getHeap()) IlcInt *[_n];
    for (IlcInt i = 0; i < _n; i++)
      _d[i] = new (cp.getHeap()) IlcInt[_n];
  }
  ~IlcMinDistanceInferencerI() {}
  void initialiseDistance() {
    for (IlcInt i = 0; i < _n; i++) {
      for (IlcInt j = 0; j < _n; j++) {
        if (i==j) {
          _d[i][j]= 0;
        } else if (_a[i][j].getMax()==0) {
          _d[i][j]= _n+1;
        } else {
          _d[i][j]= 1;
        }
      }
    }
  }

  void computeDistance() {
    for (IlcInt k = 0; k < _n; k++)
      for (IlcInt i = 0; i < _n; i++)
        for (IlcInt j = 0; j < _n; j++)
          _d[i][j]=IlcMin(_d[i][j], _d[i][k] + _d[k][j]);
  }
  void constrainDistance() {
    for (IlcInt i = 0; i < _n; i++) {
      for (IlcInt j = 0; j < _n; j++)  {
        _dist[i][j].setMin(_d[i][j]);
      }
    }
  }
  IlcBool isCompatibleDistance() {
    for (IlcInt i = 0; i < _n; i++) 
      for (IlcInt j = 0; j < _n; j++) 
        if (_d[i][j] > _dist[i][j].getMax())
          return IlcFalse;
    return IlcTrue;
  }

  virtual void execute() {
    initialiseDistance();
    computeDistance();
    constrainDistance();
    // try to remove each arc and force those arcs that lead to a
    // distance greater than the max allowed to be present
    for (IlcInt t = 0; t < _n; t++) {
      for (IlcInt v = 0; v < _n; v++) {
        if (v!=t && !_a[t][v].isFixed()) {
          initialiseDistance();
          _d[t][v]=_n+1;
          _d[v][t]=_n+1;
          computeDistance();
          // the CPU time spent in the execute() must be told to the solver. 
          // This can be done thanks to the addPropagationCost() function.
          addPropagationCost(_n*_n*_n/CostRescalingRatio);
          if (!isCompatibleDistance()) {
            _a[t][v].setValue(1);
          }
        }
      }
    }
  }

  // This function is used in the automatic mode to estimate the cost of
  // a call to execute(). The units in which the cost is expressed should
  // roughly correspond to the cost of an average propagation.
  // Here, it is given by the constant CostRescalingRatio.  This
  // constant allows the automatic mode to be tuned by trying different
  // values for the constant.
  virtual IlcFloat estimateCost(IlcFloat bound){
    IlcFloat c = _n*_n*_n / CostRescalingRatio;
    if (c >= bound)
      return bound;
    for (IlcInt t = 0; t < _n; t++) {
      for (IlcInt v = 0; v < _n; v++) {
        if (v != t && !_a[t][v].isFixed()) {
          c += 1 + _n*_n*_n / CostRescalingRatio;
          if (c >= bound)
            return bound;
        }
      }
    }
    return c;
  }
};

// The function that returns the custom inferencer in a handle class
IlcCustomInferencer IlcMinDistanceInferencer(
  IloCPEngine cp, IlcInt n, IlcIntVar ** a, IlcIntVar ** dist, 
  IlcBool manual, IlcInt skipped)
{
  return new (cp.getHeap()) IlcMinDistanceInferencerI(
    cp, n, a, dist, manual, skipped
  );
}

// The custom inferencer is an object of the solver level, i.e. an "Ilc" object.
// Its representation in the model is an IloConstraint. 
// We use the same macro for building an IloConstraint from an
// IlcCustomInferencer or from an IlcConstraint.
ILOCPCONSTRAINTWRAPPER4(IloMyDistanceConstraint, cp, IloIntVarArray2, a, 
                        IloIntVarArray2, dist, IloBool, manual, IloInt, skipped) {
  for (IlcInt i = 0; i < a.getSize(); i++)
    use(cp, a[i]);
  for (IlcInt i = 0; i < dist.getSize(); i++)
    use(cp, dist[i]);
  IlcIntVar ** sa = MakeIlcMatrix(cp, a);
  IlcIntVar ** sdist = MakeIlcMatrix(cp, dist);
  return IlcMinDistanceInferencer(cp, a.getSize(), sa, sdist, manual, skipped);
}


//--------- Maintaining distance constraint with a real constraint ------------

// This is simply a constraint that is pushed and that calls the execute()
// function of the above custom inferencer

class IlcMinDistanceConstraintI : public IlcConstraintI {
protected:
  IlcInt              _n;
  IlcIntVar **        _a;
  IlcIntVar **        _dist;
  IlcCustomInferencer _custInf;
 public:
  IlcMinDistanceConstraintI(
    IloCPEngine cp, IlcInt n, IlcIntVar ** aa, IlcIntVar ** ddist
  ) : IlcConstraintI(cp), _n(n), _a(aa), _dist(ddist) {
    _custInf = IlcMinDistanceInferencer(cp, _n, _a, _dist, IlcTrue, 0);
  }
  ~IlcMinDistanceConstraintI() {}
  virtual void post();
  virtual void propagate();
  void varDemon();
};

ILCCTDEMON0(DistanceDemon, IlcMinDistanceConstraintI, varDemon)

void IlcMinDistanceConstraintI::post () {
  IloCPEngine cp = getCPEngine();
  for (IlcInt i = 0; i < _n; i++)
    for (IlcInt j = 0; j < _n; j++)
      _a[i][j].whenValue(DistanceDemon(cp, this));
  for (IlcInt i = 0; i < _n; i++)
    for (IlcInt j = 0; j < _n; j++)
      _dist[i][j].whenRange(DistanceDemon(cp, this));
}

void IlcMinDistanceConstraintI::propagate () {
  _custInf.getImpl()->execute();
}

void IlcMinDistanceConstraintI::varDemon () {
  push();
}

IlcConstraint IlcMinDistanceConstraint(
  IloCPEngine cp, IlcInt n, IlcIntVar **a, IlcIntVar ** dist
) {
  return new (cp.getHeap()) IlcMinDistanceConstraintI(cp, n, a, dist);
}

ILOCPCONSTRAINTWRAPPER2(IloMinDistanceConstraint, cp,
                        IloIntVarArray2, a, IloIntVarArray2, dist) {
  for (IlcInt i = 0; i < a.getSize(); i++)
    use(cp, a[i]);
  for (IlcInt i = 0; i < dist.getSize(); i++)
    use(cp, dist[i]);
  IlcIntVar ** sa = MakeIlcMatrix(cp, a);
  IlcIntVar ** sdist = MakeIlcMatrix(cp, dist);
  return IlcMinDistanceConstraint(cp, a.getSize(), sa, sdist);
}

//-------------------------- Model ---------------------------------------

void NetworkOptim(const char* filename, Mode mode){
  IloEnv env;
  try {
    std::ifstream file(filename);
    if (!file){
      env.out() << "file not found" << std::endl;
      throw FileError();
    }
  
    IloIntArray2 possibleArcs;
    IloIntArray2 cost;
    IloIntArray2 maxDistances;
    IloIntArray2 additionalConstraints;
    IloInt n; 
    IloInt k; // number of suppliers
    IloInt numArcs; //number of arcs in the solution
    IloInt nbPossibleArcs;
    IloInt nbMaxDistances;
    IloInt nbAdditionalConstraints;
  
    file >> n;
    file >> k;
    file >> numArcs;
    file >> nbPossibleArcs;
    possibleArcs = IloIntArray2(env, n);
    for (IloInt i = 0; i < n; i++) {
      possibleArcs[i] = IloIntArray(env, n);
      for (IloInt j = 0; j < n; j++) {
        possibleArcs[i][j]=0;
      }
      possibleArcs[i][i]=1;
    }
    cost = IloIntArray2(env, n);
    for (IloInt i = 0; i < n; i++) {
      cost[i] = IloIntArray(env, n);
      for (IloInt j = 0; j < n; j++) {
        cost[i][j]=IloIntMax;
      }
    }
    for(IloInt i = 0; i < nbPossibleArcs; i++){
      IloInt a, b, c;
      file >> a;  
      file >> b;
      file >> c;
      possibleArcs[a][b]=1;
      possibleArcs[b][a]=1;
      cost[a][b]=c;
      cost[b][a]=c;
    }
    file >> nbMaxDistances;
    maxDistances = IloIntArray2(env, n);
    for (IloInt i = 0; i < n; i++) {
      maxDistances[i] = IloIntArray(env, n);
      for (IloInt j = 0; j < n; j++) {
        maxDistances[i][j]=n-1;
      }
    }
    for(IloInt i = 0; i < nbMaxDistances; i++){
      IloInt a, b, c;
      file >> a;  
      file >> b;
      file >> c;
      maxDistances[a][b]=c;
      maxDistances[b][a]=c;
    }
  
    file >> nbAdditionalConstraints;
    additionalConstraints = IloIntArray2(env, nbAdditionalConstraints);
    for (IloInt i = 0; i < nbAdditionalConstraints; i++) {
      additionalConstraints[i] = IloIntArray(env, 4);
      file>>additionalConstraints[i][0];
      file>>additionalConstraints[i][1];
      file>>additionalConstraints[i][2];
      file>>additionalConstraints[i][3];
    }

    IloModel mdl(env);
    IloIntVarArray2 a(env, n);
    for (IloInt i = 0; i < n; i++) {
      a[i]  = IloIntVarArray(env);
      for (IloInt j=0; j<n; j++) {
        a[i].add(IloIntVar(env,0,possibleArcs[i][j]));
      }
    }

    IloIntVarArray2 dist(env, n);
    for (IloInt i = 0; i < n; i++) {
      dist[i]  = IloIntVarArray(env);
      for (IloInt j=0; j<n; j++) {
        if (i==j)
          dist[i].add(IloIntVar(env,0,0));
        else
          dist[i].add(IloIntVar(env,0,maxDistances[i][j]));
        mdl.add(dist[i][j]);
      }
    }

    IloIntVarArray supplier(env, n,0,1);

    // undirected graph
    for (IloInt i = 0; i < n-1; i++) {
      for (IloInt j = i+1; j < n; j++) {
        mdl.add(a[i][j]==a[j][i]);  
      }
    }
    for (IloInt i = 0; i < n; i++) {
      mdl.add(a[i][i]==1);  
    }

    // at least one connection for each node
    IloIntVarArray nbNeighbors(env,n,1, n-1);
    for (IloInt i = 0; i < n; i++) {
      mdl.add(nbNeighbors[i]==IloSum(a[i])-1); //a[i][i] is = 1
    }

    // exactly one supplier in each connection
    for (IloInt i = 0; i < n; i++) {
      for (IloInt j = 0; j < n; j++) {
        if (i!=j)
          mdl.add(a[i][j] <= (supplier[i]!=supplier[j]));  
      }
    }

    // Exactly k suppliers
    mdl.add(IloSum(supplier)==k);

    // a redundant constraint can be infered from the problem. As in each arc,
    // one node is a supplier, a simple path have at least k supliers and
    // thus 2*k arcs. Moreover, when one (resp. two) edge(s) 
    // of the path is a supplier, its length is at most 2*k-1 (resp. 2*k-2)
    for (IloInt i = 0; i < n; i++) {
      for (IloInt j = 0; j < n; j++) {
        if (i!=j) {
          mdl.add(dist[i][j] <= 2*k);
          mdl.add( (supplier[i]==0 && supplier[j] ==0) || dist[i][j] <=2*k-1);
          mdl.add( supplier[i]==0 || supplier[j] ==0 || dist[i][j] <=2*k-2);
        }
      }
    }

    //exactly numArcs undirected arcs
    mdl.add(IloSum(nbNeighbors) == numArcs); 

    // Three different ways for ensuring the properties on distances:
    // 1: a constraint, 
    if (mode==CONSTRAINT) {
      mdl.add(IloMinDistanceConstraint(env,a,dist,"IloMinDistanceConstraint"));
    }
    else if (mode==MANUAL) {
      // 2: a custom inferencer in a manual mode specifying the number
      // of nodes to explore between two invocation of the custom inferencer   
      mdl.add(IloMyDistanceConstraint(env,a,dist, IloTrue, NumSkipped, "IloMyMinDistanceConstraint"));
    }
    else { // mode==AUTO
      // 3: a custom inferencer in automatic mode
      mdl.add(IloMyDistanceConstraint(env,a,dist, IloFalse, 0, "IloMyMinDistanceConstraint"));
    }

    // Additional constraints
    for (IloInt i = 0; i < nbAdditionalConstraints; i++) {
      mdl.add(
        a[additionalConstraints[i][0]][additionalConstraints[i][1]]==1 || 
        a[additionalConstraints[i][2]][additionalConstraints[i][3]]==1);
    }
 
    // The objective is to minimize the cost of arcs.
    IloExpr objExp = IloIntExpr(env, 0);
    for (IloInt i = 0; i < n; i++) {
      for (IloInt j = i+1; j < n; j++) {
        objExp += a[i][j]*cost[i][j];
      }
    }
    IloObjective obj = IloMinimize(env, objExp);
    mdl.add(obj);

    IloCP cp(mdl);
    cp.setParameter(IloCP::TimeLimit, 20); 
    cp.setParameter(IloCP::LogPeriod, 10000); 

    IloIntVarArray decisionVars(env);
    for (IloInt i = 0; i < n; i++) {
      decisionVars.add(a[i]);
      decisionVars.add(supplier[i]);
    }

    // There is no need to assign the variables representing the distances
    IloSearchPhase userPhase = IloSearchPhase(env, decisionVars);
    cp.setSearchPhases(userPhase);
    if (cp.solve()) {
      cp.out() << "Cost= "<<cp.getValue(objExp) << std::endl;
      cp.out() << "Suppliers: ";
      for (IloInt i =0; i < n; i++) {
        if (cp.getValue(supplier[i])==1)
          cp.out() << i<< " ";
      }
      cp.out()  << std::endl << "Arcs: ";
      for (IloInt i = 0; i < n-1; i++) {    
        for (IloInt j = i+1; j < n; j++) {
          if (cp.getValue(a[i][j])==1)
            cp.out() << "("<<i<<" "<<j<<") ";
        }
      }
      cp.out() << std::endl;
    } else {
      cp.out() << "No solution found" << std::endl;
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}

int main(int argc, const char* argv[]){
  Mode mode = AUTO;
  if (argc > 1)
    mode = (Mode)atoi(argv[1]);
  const char* filename = "../../../examples/data/networkcfg.data";
  if (argc > 2)
    filename = argv[2];
  NetworkOptim(filename, mode);
  return 0;
}
