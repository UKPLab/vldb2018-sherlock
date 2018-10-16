// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_learningeffect.cpp
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

This example is an extension of the classical Job-Shop Scheduling
problem (see sched_jobshop.cpp) with a learning effect on machines: 
because of experience acquired by the machine, executing an
operation at position i on the machine will require less time than 
if it were executed earlier at a position k < i. 

More formally, each machine M_j has a learning factor alpha_j in [0,1] 
such that the actual processing time of the operation executed at the 
ith position on machine M_j is the decreasing function
d_j(i) = D * pow(alpha_j,i) where D is the nominal processing time of
operation.

The model for a resource, except for the classical no-overlap constraint, 
consists of a chain of intervals of unknown size that forms a one-to-one
correspondance with the actual operations. The correspondance (made using
an isomorphism constraint) associates an integer variable (the position)
with each operation of the resource.  The position variable is used to
define the processing time of an operation subject to the learning effect.

This example illustrates the typical usage of the isomorphism constraint
to express relations according to the rank order of operations and to
get the position of interval variables in a sequence.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/learningeffect_default.data";
    if (argc > 1)
      filename = argv[1];
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    IloIntArray2 sizes(env, nbMachines);
    for (IloInt j = 0; j < nbMachines; j++) {
      machines[j] = IloIntervalVarArray(env);
      sizes[j] = IloIntArray(env);
    }
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbJobs; i++) {
      IloIntervalVar prec;
      for (IloInt j = 0; j < nbMachines; j++) {
        IloInt m, d;
        file >> m;
        file >> d;
        IloIntervalVar ti(env);
        machines[m].add(ti);
        sizes[m].add(d);
        if (0 != prec.getImpl())
          model.add(IloEndBeforeStart(env, prec, ti));
        prec = ti;
      }
      ends.add(IloEndOf(prec));
    }
    for (IloInt j = 0; j < nbMachines; j++) {
      IloInt lef;
      file >> lef;
      IloNum alpha = lef/100.0;
      IloIntervalVarArray chain(env);
      IloIntVarArray indices(env);
      IloIntervalVar prec;
      for(IloInt i = 0; i < nbJobs; ++i) {
        IloIntervalVar ti = machines[j][i];
        IloInt d = sizes[j][i];
        ti.setSizeMax(d);
        // Building of the chain of intervals for the machine.
        IloIntervalVar syncti(env);
        if (prec.getImpl()) 
          model.add(IloEndBeforeStart(env, prec, syncti));
        prec = syncti;
        IloIntVar index(env, 0, nbJobs - 1);
        // Learning effect captured by the decreasing function
        // of the position (0 <= alpha <= 1).
        // At first position in the sequence index = 0; there is no
        // learning effect and duration of the task is its nominal duration
        IloNumExpr floatDuration = d * IloPower(alpha, index);
        model.add(IloAbs(floatDuration - IloSizeOf(ti)) <= 0.5);
        chain.add(syncti);
        indices.add(index);
      }
      model.add(IloIsomorphism(env, chain, machines[j], indices, nbJobs));

      // The no-overlap is a redundant constraint in this quite simple
      // model - it is used only to provide stronger inference.
      model.add(IloNoOverlap(env, machines[j]));
    }
    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);
    
    IloCP cp(model);
    cp.setParameter(IloCP::LogPeriod, 10000);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
      cp.out() << "Makespan \t: " << cp.getObjValue() << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}
