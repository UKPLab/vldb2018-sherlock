// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_rcpsp.cpp
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

The RCPSP (Resource-Constrained Project Scheduling Problem) is a
generalization of the production-specific Job-Shop (see
sched_jobshop.cpp), Flow-Shop (see sched_flowshop.cpp) and Open-Shop
(see sched_openshop.cpp) scheduling problems. Given:

- a set of q resources with given capacities,
- a network of precedence constraints between the activities, and
- for each activity and each resource the amount of the resource
  required by the activity over its execution,

the goal of the RCPSP is to find a schedule meeting all the
constraints whose makespan (i.e., the time at which all activities are
finished) is minimal.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/rcpsp_default.data";
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
    IloInt nbTasks, nbResources, i, j;
    file >> nbTasks;
    file >> nbResources;
    IloCumulFunctionExprArray resources(env, nbResources);
    IloIntArray capacities(env, nbResources);
    for (j=0; j<nbResources; j++) {
      IloInt c;
      file >> c;
      capacities[j] = c;
      resources[j] = IloCumulFunctionExpr(env);
    }
    IloIntervalVarArray tasks(env, nbTasks);
    for (i=0; i<nbTasks; i++) {
      tasks[i] = IloIntervalVar(env);
    }
    IloIntExprArray ends(env);
    for (i=0; i<nbTasks; i++) {
      IloIntervalVar task = tasks[i];
      IloInt d, nbSucc;
      file >> d;
      task.setSizeMin(d);
      task.setSizeMax(d);
      ends.add(IloEndOf(task));
      for (j = 0; j < nbResources; j++) {
        IloInt q;
        file >> q;
        if (q > 0) {
          resources[j] += IloPulse(task, q);
        }
      }
      file >> nbSucc;
      for (IloInt s=0; s<nbSucc; ++s) {
        IloInt succ;
        file >> succ;
        model.add(IloEndBeforeStart(env, task, tasks[succ-1]));
      }
    }

    for (j = 0; j < nbResources; j++) {
      model.add(resources[j] <= capacities[j]);
    }

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
