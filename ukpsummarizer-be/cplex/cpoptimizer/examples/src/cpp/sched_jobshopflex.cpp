// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_jobshopflex.cpp
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

This problem is an extension of the classical Job-Shop Scheduling
problem (see sched_jobshop.cpp) which allows an operation to be
processed by any machine from a given set. The operation processing
time depends on the allocated machine. The problem is to assign each
operation to a machine and to order the operations on the machines
such that the maximal completion time (makespan) of all operations is
minimized.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/jobshopflex_default.data";
    IloInt failLimit = 10000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    for (IloInt j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);

    for (IloInt i = 0; i < nbJobs; i++) {
      IloInt nbOperations;
      file >> nbOperations;
      IloIntervalVar prec;
      for (IloInt j = 0; j < nbOperations; j++) {
        IloInt nbOpMachines, k;
        file >> nbOpMachines;
        IloIntervalVar master(env);
        IloIntervalVarArray members(env);
        for (k = 0; k < nbOpMachines; k++) {
          IloInt m, d;
          file >> m;
          file >> d;
          IloIntervalVar member(env, d);
          member.setOptional();
          members.add(member);
          machines[m-1].add(member);
        }
        model.add(IloAlternative(env, master, members));
        if (0 != prec.getImpl())
          model.add(IloEndBeforeStart(env, prec, master));
        prec = master;
      }
      ends.add(IloEndOf(prec));
    }

    for (IloInt j = 0; j < nbMachines; j++)
      model.add(IloNoOverlap(env, machines[j]));

    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
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
