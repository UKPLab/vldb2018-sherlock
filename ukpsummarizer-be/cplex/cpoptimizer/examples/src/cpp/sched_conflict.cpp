// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_conflict.cpp
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

/* --------------------------------------------------------------------------

Problem Description
-------------------

This model illustrates the use of the CP Optimizer conflict refiner
on an infeasible scheduling problem. 
The problem is an instance of RCPSP (Resource-Constrained Project Scheduling 
Problem) with time windows. Given:

- a set of q resources with given capacities,
- a network of precedence constraints between the activities,
- a set of activities to be executed within a given time window and
- for each activity and each resource the amount of the resource
  required by the activity over its execution,

the goal of the problem is to find a schedule satisfying all the
constraints whose makespan (i.e., the time at which all activities are
finished) is minimal.

The instance is infeasible. The example illustrates 5 scenarios using the 
conflict refiner:

- Scenario 1: Identify a minimal conflict (no matter which one).
- Scenario 2: Identify a minimal conflict preferably using resource capacity 
              constraints.
- Scenario 3: Identify a minimal conflict preferably using precedence 
              constraints.
- Scenario 4: Find a minimal conflict partition that is, a set of disjoint 
              minimal conflicts S1,...,Sn such that when all constraints in 
              S1 U S2 U... U Sn are removed from the model, the model becomes 
              feasible.
- Scenario 5: Identify all minimal conflicts of the problem.

----------------------------------------------------------------------------- */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

//----- RCPSP Model creation ------------------------------------------------

IloModel ReadModel(IloEnv env, const char* filename,
                   IloConstraintArray& capacityCts,
                   IloConstraintArray& precedenceCts) {
  std::ifstream file(filename);
  if (!file){
    env.out() << "usage: sched_conflict <file> <failLimit>" << std::endl;
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
  char name[32];
  for (i=0; i<nbTasks; i++) {
    sprintf(name, "ACT%ld", (long)i);
    tasks[i] = IloIntervalVar(env, name);
    ILOSETLOCATION(tasks[i]);
  }
  IloIntExprArray ends(env);
  for (i=0; i<nbTasks; i++) {
    IloIntervalVar task = tasks[i];
    IloInt d, smin, emax, nbSucc;
    file >> d >> smin >> emax;
    task.setSizeMin(d);
    task.setSizeMax(d);
    task.setStartMin(smin);
    task.setEndMax(emax);
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
      IloConstraint pct = IloEndBeforeStart(env, task, tasks[succ]);
      ILOADD(model, pct);
      precedenceCts.add(pct);
    }
  }
  for (j=0; j<nbResources; j++) {
    sprintf(name, "RES%ld", (long)j);
    resources[j].setName(name);
    IloConstraint cct = (resources[j] <= capacities[j]);
    ILOADD(model, cct);
    capacityCts.add(cct);
  }
  model.add(IloMinimize(env, IloMax(ends)));
  return model;
}                 

//----- Basic run of conflict refiner  --------------------------------------

void runBasicConflictRefiner(IloCP cp) {
  if (cp.refineConflict()) {
    cp.writeConflict(cp.out());
  } 
}

//----- Run conflict refiner with a subset of preferred constraints  --------

void runConflictRefinerWithPreferences(IloCP cp, 
                                       IloConstraintArray preferredCts, 
                                       IloConstraintArray otherCts) {
  IloEnv env = cp.getEnv();
  IloConstraintArray cts(env);  
  IloNumArray prefs(env);
  IloInt i;
  for (i=0; i<otherCts.getSize(); ++i) {
    cts.add(otherCts[i]);
    prefs.add(1.0); // Normal preference
  }
  for (i=0; i<preferredCts.getSize(); ++i) {
    cts.add(preferredCts[i]);
    prefs.add(2.0); // Higher preference
  }
  if (cp.refineConflict(cts, prefs)) {
    cp.writeConflict(cp.out());
  } 
  cts.end();
  prefs.end();
}

//----- Use conflict refiner to compute a minimal conflict partition --------

void runConflictRefinerPartition(IloCP cp, IloConstraintArray cts) {     
  IloEnv env = cp.getEnv();
  IloInt n = cts.getSize();
  IloNumArray prefs(env, n);   
  IloInt i;  
  for (i=0; i<n; ++i) {
    prefs[i]=1.0; // Normal preference
  }
  while (cp.refineConflict(cts, prefs)) {
    cp.writeConflict(cp.out());
    for (i=0; i<n; ++i) {
      if (cp.getConflict(cts[i])==IloCP::ConflictMember) {
        prefs[i]=-1.0; // Next run will ignore constraints of the current conflict
      }
    }
  }
  prefs.end();
}

//----- Use conflict refiner to compute all minimal conflicts  --------------
//----- Note: the simple search below is complete but not systematic: -------
//----- it may find the same conflict several times.                  -------

void runConflictRefinerAllConflicts(IloCP cp, IloConstraintArray cts) {     
  IloEnv env = cp.getEnv();
  IloInt n = cts.getSize();
  IloNumArray prefs(env, n);   
  IloInt i;  
  IloIntArray2 forbiddenCts(env);
  // Initial iteration: no forbidden constraints
  forbiddenCts.add(IloIntArray(env));
  while (0<forbiddenCts.getSize()) {
    IloIntArray forbidden = forbiddenCts[0];
    forbiddenCts.remove(0);
    for (i=0; i<n; ++i) {
      prefs[i]=1.0; // Initialize to normal preference
    }
    for (i=0; i<forbidden.getSize(); ++i) {
      prefs[forbidden[i]]=-1.0; // Remove constraint
    }
    if (cp.refineConflict(cts, prefs)) {
      cp.writeConflict(cp.out());
      // Next iterations
      for (i=0; i<n; ++i) {
        if (cp.getConflict(cts[i])==IloCP::ConflictMember) {
            IloIntArray newforbidden(env, 1, i);
            newforbidden.add(forbidden);           
            forbiddenCts.add(newforbidden);
        }
      }
    }
    forbidden.end();
  }
}
                                       
int main(int argc, const char* argv[]) {
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/sched_conflict.data";
    IloInt failLimit = 10000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    IloConstraintArray allCts(env);
    IloConstraintArray capacityCts(env);
    IloConstraintArray precedenceCts(env);
    IloModel model = ReadModel(env, filename, capacityCts, precedenceCts);
    allCts.add(capacityCts);
    allCts.add(precedenceCts);
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.setParameter(IloCP::CumulFunctionInferenceLevel, IloCP::Extended);
    cp.setParameter(IloCP::ConflictRefinerOnVariables, IloCP::On);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
      // A solution was found
      cp.out() << "Solution found with makespan : " << cp.getObjValue() << std::endl;
    } else {
      IloInt status = cp.getInfo(IloCP::FailStatus);
      if (status != IloCP::SearchHasFailedNormally) {
        // No solution found but problem was not proved infeasible
        cp.out() << "No solution found but problem was not proved infeasible." << std::endl;
      } else {
        // Run conflict refiner only if problem was proved infeasible
        cp.out() << "Infeasible problem, running conflict refiner ..." << std::endl;
        cp.out() << std::endl;
        cp.out() << "SCENARIO 1: Basic conflict refiner:" << std::endl;
        cp.out() << std::endl;
        runBasicConflictRefiner(cp);
        cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
        cp.out() << "SCENARIO 2: Conflict refiner with preference on resource capacity constraints:" << std::endl;
        cp.out() << std::endl;
        runConflictRefinerWithPreferences(cp, capacityCts, precedenceCts);
        cp.out() << "SCENARIO 3: Conflict refiner with preference on precedence constraints:" << std::endl;
        cp.out() << std::endl;
        runConflictRefinerWithPreferences(cp, precedenceCts, capacityCts);
        cp.out() << "SCENARIO 4: Conflict partition:" << std::endl; 
        cp.out() << std::endl;
        runConflictRefinerPartition(cp, allCts);
        cp.out() << "SCENARIO 5: All conflicts:" << std::endl;
        cp.out() << std::endl;
        runConflictRefinerAllConflicts(cp, allCts);
      }
    }
  } catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
