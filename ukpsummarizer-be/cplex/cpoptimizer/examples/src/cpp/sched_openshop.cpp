// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_openshop.cpp
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

This problem can be described as follows: a finite set of operations
has to be processed on a given set of machines. Each operation has a
specific processing time during which it may not be interrupted.
Operations are grouped in jobs, so that each operation belongs to
exactly one job. Furthermore, each operation requires exactly one
machine for processing.

The objective of the problem is to schedule all operations, i.e., to
determine their start time, so as to minimize the maximum completion
time (makespan) given the additional constraints that: operations
which belong to the same job and operations which use the same machine
cannot be processed simultaneously.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/openshop_default.data";
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
    IloInt i, j;
    IloIntervalVarArray2 jobs(env, nbJobs);
    IloIntervalVarArray2 machines(env, nbMachines);
    for (i = 0; i < nbJobs; i++)
      jobs[i] = IloIntervalVarArray(env);
    for (j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (i = 0; i < nbJobs; i++) {
      for (j = 0; j < nbMachines; j++) {
        IloInt pt;
        file >> pt;
        IloIntervalVar ti(env, pt);
        jobs[i].add(ti);
        machines[j].add(ti);
        ends.add(IloEndOf(ti));
      }
    }
    for (i = 0; i < nbJobs; i++)
      model.add(IloNoOverlap(env, jobs[i]));
    for (j = 0; j < nbMachines; j++)
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
