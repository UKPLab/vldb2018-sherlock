// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_goalprog.cpp
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

This example solves a multi-machine assignment and scheduling problem
([1]), where jobs, with release dates and deadlines, have to be
processed on parallel unrelated machines where processing times depend
on machine assignment. Given a job/machine assignment cost matrix, the
objective is to minimize the total cost while keeping all machine
schedules feasible.

In the instances used in [1], two types of machines can be
distinguished: expensive machines (whose "cost" for a job is indicated
by a value of 1000) and regular machines (whose cost is typically less
than 50). This example implements a two-step approach:

- In a first step, the problem is solved by taking all the constraints
into account but neglecting the cost of regular machines and focusing
on minimizing the number of jobs allocated to expensive machines.

- A solution to the above problem is also a solution to the main
problem. In a second step, this solution is used as starting point
for solving the second part of the problem, which is to minimize the
cost of allocating jobs to regular machines.  During this second search,
the number of jobs allocated to expensive machines is upper-bounded
by the number found during the first step.

This two-step approach focusses the first phase on the most important
part of the cost (the allocation of jobs to expensive machines) and,
once a good (or optimal) solution has been found, to use this solution
as a starting point for minimizing the other part of the cost. This
approach can be used in most problems where the components of the
objective function are lexically ordered.

By default, the program executes the two-step approach. If the second
argument (useGoalProgramming) is set to 0, the program executes a
single-step approach assuming an allocation cost of 1000 for the expensive
machines.  With the same global limit, results with the two-step approach
are better than the ones of a single-step approach.

[1] R. Sadykov and L. Wolsey. Integer programming and constraint
programming in solving a multi-machine assignment scheduling problem
with deadlines and release dates.  INFORMS Journal on Computing. 2006.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

void CreateModel(const char* dataFile,
                 IloModel& model, IloIntArray2& costs, IloIntervalVarArray2& machines) {
  IloEnv env = model.getEnv();
  std::ifstream input(dataFile);
  if(!input) {
    env.out() << "usage: sched_goalprog <file> <useGoalProgramming> <branchLimit>" << std::endl;
    throw FileError();
  }
  IloInt nbJobs, nbMachines, i, j;
  input >> nbJobs >> nbMachines;
  IloIntArray rel(env, nbJobs);
  IloIntArray due(env, nbJobs);
  costs = IloIntArray2(env, nbMachines);
  IloIntArray2 dur(env, nbMachines);
  for (j=0; j<nbMachines; j++) {
    costs[j] = IloIntArray(env, nbJobs);
    dur[j]   = IloIntArray(env, nbJobs);
  }

  for (i=0; i<nbJobs; i++)
    input >> rel[i];

  for (i=0; i<nbJobs; i++)
    input >> due[i];

  for (j=0; j<nbMachines; j++)
    for (i=0; i<nbJobs; i++)
      input >> costs[j][i];

  for (j=0; j<nbMachines; j++)
    for (i=0; i<nbJobs; i++)
      input >> dur[j][i];

  input.close();

  machines = IloIntervalVarArray2(env, nbMachines);
  for (j = 0; j < nbMachines; j++)
    machines[j] = IloIntervalVarArray(env);
  char name[128];
  for (i = 0; i < nbJobs; i++) {
    IloIntervalVar job(env);
    job.setStartMin(rel[i]);
    job.setEndMax(due[i]);
    sprintf(name, "Op%ld",(long)i);
    job.setName(name);
    IloIntervalVarArray jobm(env, nbMachines);
    for (j=0; j<nbMachines; j++) {
      sprintf(name, "Alt%ld_%ld_C%ld",(long)i,(long)j,(long)costs[j][i]);
      jobm[j] = IloIntervalVar(env, dur[j][i]);
      jobm[j].setName(name);
      jobm[j].setOptional();
      machines[j].add(jobm[j]);
    }
    model.add(IloAlternative(env, job, jobm));
  }
  for (j = 0; j < nbMachines; j++)
    model.add(IloNoOverlap(env, machines[j]));
}

IloNum SolveGoalProgramming(const char* dataFile, IloInt branchLimit) {
  IloEnv env;
  IloModel model(env);
  IloIntArray2 costs;
  IloIntervalVarArray2 machines;
  CreateModel(dataFile, model, costs, machines);
  IloInt nbMachines = machines.getSize();
  IloInt nbJobs = machines[0].getSize();

  // Objective 1
  env.out() << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;
  env.out() << " ! STEP 1: Minimizing usage of expensive resources" << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;
  IloIntExpr costExpr(env);
  for (IloInt i = 0; i < nbJobs; i++) {
    for (IloInt j = 0; j < nbMachines; j++) {
      if (costs[j][i] == 1000) {
        costExpr += IloPresenceOf(env, machines[j][i]);
      }
    }
  }
  IloObjective obj1 = IloMinimize(env, costExpr);
  model.add(obj1);

  IloCP cp(model);
  cp.setParameter(IloCP::LogPeriod, IloIntMax);
  cp.setParameter(IloCP::BranchLimit, branchLimit/2);
  cp.setParameter(IloCP::NoOverlapInferenceLevel, IloCP::Extended);
  cp.solve();
  IloNum cost1 = cp.getObjValue();
  IloInt branchesLeft = branchLimit - cp.getInfo(IloCP::NumberOfBranches);

  IloSolution startsol(env);
  for (IloInt i = 0; i < nbJobs; i++) {
    for (IloInt j = 0; j < nbMachines; j++) {
      if (cp.isPresent(machines[j][i])) {
        startsol.setPresent(machines[j][i]);
        startsol.setStart(machines[j][i], cp.getStart(machines[j][i]));
      }
    }
  }

  // Objective 2
  env.out() << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;
  env.out() << " ! STEP 2: Minimizing cost on normal resources" << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;

  model.remove(obj1);
  model.add(costExpr <= cost1);

  IloIntExpr costExpr2(env);
  for (IloInt i = 0; i < nbJobs; i++) {
    for (IloInt j = 0; j < nbMachines; j++) {
      if (costs[j][i] < 1000)
        costExpr2 += costs[j][i] * IloPresenceOf(env, machines[j][i]);
    }
  }
  IloObjective obj2 = IloMinimize(env, costExpr2);
  model.add(obj2);
  cp.setParameter(IloCP::BranchLimit, branchesLeft);
  cp.setStartingPoint(startsol);
  cp.solve();
  IloNum cost2 = cp.getObjValue();

  env.end();
  return 1000 * cost1 + cost2;
}

IloNum SolveBasic(const char* dataFile, IloInt branchLimit) {
  IloEnv env;
  IloModel model(env);
  IloIntArray2 costs;
  IloIntervalVarArray2 machines;
  CreateModel(dataFile, model, costs, machines);

  // Objective
  env.out() << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;
  env.out() << " ! Minimizing total cost" << std::endl;
  env.out() << " ! ----------------------------------------------------------------------------" << std::endl;
  IloIntExpr costExpr(env);
  IloInt nbMachines = machines.getSize();
  IloInt nbJobs = machines[0].getSize();
  for (IloInt i = 0; i < nbJobs; i++) {
    for (IloInt j = 0; j < nbMachines; j++) {
      costExpr += costs[j][i] * IloPresenceOf(env, machines[j][i]);
    }
  }
  IloObjective obj = IloMinimize(env, costExpr);
  model.add(obj);

  IloCP cp(model);
  cp.setParameter(IloCP::LogPeriod, IloIntMax);
  cp.setParameter(IloCP::BranchLimit, branchLimit);
  cp.setParameter(IloCP::NoOverlapInferenceLevel, IloCP::Extended);
  cp.solve();
  IloNum cost = cp.getObjValue();
  env.end();
  return cost;
}

int main(int argc, const char* argv[]) {
  const char* filename = "../../../examples/data/goalprog_8_40_0.6_3.data";
  IloInt useGoalProgramming = IloTrue;
  IloInt branchLimit = 300000;
  if (argc > 1)
    filename = argv[1];
  if (argc > 2)
    useGoalProgramming = atoi(argv[2]);
  if (argc > 3)
    branchLimit = atoi(argv[3]);

  try {
    IloNum cost = 0;
    std::cout << "Data file: " << filename << std::endl;
    if (useGoalProgramming) {
      std::cout << "Solving in two steps using goal programming ..." << std::endl;
      cost = SolveGoalProgramming(filename, branchLimit);
    } else {
      std::cout << "Solving in a single step  ..." << std::endl;
      cost = SolveBasic(filename, branchLimit);
    }

    std::cout << std::endl;
    std::cout << " ! ----------------------------------------------------------------------------" << std::endl;
    std::cout << " ! Cost = " << cost << std::endl;
    std::cout << " ! ----------------------------------------------------------------------------" << std::endl;
  } catch(IloException& e) {
    std::cout << " ERROR: " << e << std::endl;
  }
  return 0;
}
