// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_pflowshop.cpp
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

The general Flow-Shop scheduling problem is a production problem where
a set of n jobs have to be processed with identical flow pattern on m
machines (see sched_flowshop.cpp). In permutation Flow-Shops the
sequence of jobs is the same on all machines.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/flowshop_default.data";
    IloInt failLimit = IloIntMax;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloInt i,j;
    IloModel model(env);
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    for (j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (i = 0; i < nbJobs; i++) {
      IloIntervalVar prec;
      for (j = 0; j < nbMachines; j++) {
        IloInt d;
        file >> d;
        IloIntervalVar ti(env, d);
        machines[j].add(ti);
        if (0 != prec.getImpl())
          model.add(IloEndBeforeStart(env, prec, ti));
        prec = ti;
      }
      ends.add(IloEndOf(prec));
    }

    IloIntervalSequenceVarArray seqs(env,nbMachines);
    for (j = 0; j < nbMachines; j++) {
      seqs[j] = IloIntervalSequenceVar(env, machines[j]);
      model.add(IloNoOverlap(env,seqs[j]));
      if (0<j) {
        model.add(IloSameSequence(env, seqs[0],seqs[j]));
      }
    }
    
    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
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
