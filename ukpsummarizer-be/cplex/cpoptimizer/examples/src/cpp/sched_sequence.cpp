// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_sequence.cpp
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

This is a problem of building five houses in different locations. The
masonry, roofing, painting, etc. must be scheduled. Some tasks must
necessarily take place before others and these requirements are
expressed through precedence constraints.

There are two workers, and each task requires a specific worker.  The
time required for the workers to travel between houses must be taken
into account.

Moreover, there are tardiness costs associated with some tasks as well
as a cost associated with the length of time it takes to build each
house.  The objective is to minimize these costs.

------------------------------------------------------------ */

#include <ilcp/cp.h>

const IloInt NbTasks = 10;

enum Tasks {
  masonry   = 0,
  carpentry = 1,
  plumbing  = 2,
  ceiling   = 3,
  roofing   = 4,
  painting  = 5,
  windows   = 6,
  facade    = 7,
  garden    = 8,
  moving    = 9
};

const char* TaskNames [] = {
  "masonry  ",
  "carpentry",
  "plumbing ",
  "ceiling  ",
  "roofing  ",
  "painting ",
  "windows  ",
  "facade   ",
  "garden   ",
  "moving   "
};

const IloInt TaskDurations [] = {
  35,
  15,
  40,
  15,
  05,
  10,
  05,
  10,
  05,
  05,
};

IloNumExpr TardinessCost(IloIntervalVar task, IloInt dd, IloNum weight) {
  return weight * IloMax(0, IloEndOf(task)-dd);
}

void MakeHouse(IloModel model,
      IloNumExpr cost,
      IloIntervalVarArray allTasks,
      IloIntervalVarArray joeTasks,
      IloIntervalVarArray jimTasks,
      IloIntArray joeLocations,
      IloIntArray jimLocations,
      IloInt loc,
      IloInt rd,
      IloInt dd,
      IloNum weight) {
  IloEnv env = model.getEnv();

  /* CREATE THE INTERVALS VARIABLES. */
  char name[128];

  IloIntervalVarArray tasks(env, NbTasks);
  for (IloInt i=0; i<NbTasks; ++i) {
    sprintf(name, "H%ld-%s", (long)loc, TaskNames[i]);
    tasks[i] = IloIntervalVar(env, TaskDurations[i], name);
    allTasks.add(tasks[i]);
  }

  /* SPAN CONSTRAINT. */
  sprintf(name, "H%ld", (long)loc);
  IloIntervalVar house(env, name);
  model.add(IloSpan(env, house, tasks));

  /* ADDING PRECEDENCE CONSTRAINTS. */
  house.setStartMin(rd);
  model.add(IloEndBeforeStart(env, tasks[masonry],   tasks[carpentry]));
  model.add(IloEndBeforeStart(env, tasks[masonry],   tasks[plumbing]));
  model.add(IloEndBeforeStart(env, tasks[masonry],   tasks[ceiling]));
  model.add(IloEndBeforeStart(env, tasks[carpentry], tasks[roofing]));
  model.add(IloEndBeforeStart(env, tasks[ceiling],   tasks[painting]));
  model.add(IloEndBeforeStart(env, tasks[roofing],   tasks[windows]));
  model.add(IloEndBeforeStart(env, tasks[roofing],   tasks[facade]));
  model.add(IloEndBeforeStart(env, tasks[plumbing],  tasks[facade]));
  model.add(IloEndBeforeStart(env, tasks[roofing],   tasks[garden]));
  model.add(IloEndBeforeStart(env, tasks[plumbing],  tasks[garden]));
  model.add(IloEndBeforeStart(env, tasks[windows],   tasks[moving]));
  model.add(IloEndBeforeStart(env, tasks[facade],    tasks[moving]));
  model.add(IloEndBeforeStart(env, tasks[garden],    tasks[moving]));
  model.add(IloEndBeforeStart(env, tasks[painting],  tasks[moving]));

  /* ALLOCATING TASKS TO WORKERS. */
  joeTasks.add(tasks[masonry]);
  joeLocations.add(loc);
  joeTasks.add(tasks[carpentry]);
  joeLocations.add(loc);
  jimTasks.add(tasks[plumbing]);
  jimLocations.add(loc);
  jimTasks.add(tasks[ceiling]);
  jimLocations.add(loc);
  joeTasks.add(tasks[roofing]);
  joeLocations.add(loc);
  jimTasks.add(tasks[painting]);
  jimLocations.add(loc);
  jimTasks.add(tasks[windows]);
  jimLocations.add(loc);
  joeTasks.add(tasks[facade]);
  joeLocations.add(loc);
  joeTasks.add(tasks[garden]);
  joeLocations.add(loc);
  jimTasks.add(tasks[moving]);
  jimLocations.add(loc);

  /* DEFINING MINIMIZATION OBJECTIVE. */
  cost += TardinessCost(house, dd, weight);
  cost += IloLengthOf(house);
}

int main(int, const char * []) {
  IloEnv env;
  try {
    IloInt i,j;
    IloModel model(env);

    IloNumExpr cost(env);
    IloIntervalVarArray allTasks(env);
    IloIntervalVarArray joeTasks(env);
    IloIntervalVarArray jimTasks(env);
    IloIntArray joeLocations(env);
    IloIntArray jimLocations(env);

    MakeHouse(model, cost, allTasks, joeTasks, jimTasks, joeLocations,
      jimLocations, 0, 0,   120, 100.0);
    MakeHouse(model, cost, allTasks, joeTasks, jimTasks, joeLocations,
      jimLocations, 1, 0,   212, 100.0);
    MakeHouse(model, cost, allTasks, joeTasks, jimTasks, joeLocations,
      jimLocations, 2, 151, 304, 100.0);
    MakeHouse(model, cost, allTasks, joeTasks, jimTasks, joeLocations,
      jimLocations, 3, 59,  181, 200.0);
    MakeHouse(model, cost, allTasks, joeTasks, jimTasks, joeLocations,
      jimLocations, 4, 243, 425, 100.0);

    IloTransitionDistance tt(env, 5);
    for (i=0; i<5; ++i)
      for (j=0; j<5; ++j)
        tt.setValue(i, j, IloAbs(i-j));

    IloIntervalSequenceVar joe(env, joeTasks, joeLocations, "Joe");
    IloIntervalSequenceVar jim(env, jimTasks, jimLocations, "Jim");

    model.add(IloNoOverlap(env, joe, tt));
    model.add(IloNoOverlap(env, jim, tt));

    model.add(IloMinimize(env, cost));

  /* EXTRACTING THE MODEL AND SOLVING. */
    IloCP cp(model);
    if (cp.solve()) {
      cp.out() << "Solution with objective " << cp.getObjValue() << ":" << std::endl;
      for (i=0; i<allTasks.getSize(); ++i) {
        cp.out() << cp.domain(allTasks[i]) << std::endl;
      }
    } else {
      cp.out() << "No solution found. " << std::endl;
    }
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}

/*
Solution with objective 13852:
H0-masonry  [1: 1 -- 35 --> 36]
H0-carpentry[1: 36 -- 15 --> 51]
H0-plumbing [1: 36 -- 40 --> 76]
H0-ceiling  [1: 76 -- 15 --> 91]
H0-roofing  [1: 51 -- 5 --> 56]
H0-painting [1: 91 -- 10 --> 101]
H0-windows  [1: 101 -- 5 --> 106]
H0-facade   [1: 97 -- 10 --> 107]
H0-garden   [1: 107 -- 5 --> 112]
H0-moving   [1: 112 -- 5 --> 117]
H1-masonry  [1: 138 -- 35 --> 173]
H1-carpentry[1: 192 -- 15 --> 207]
H1-plumbing [1: 197 -- 40 --> 237]
H1-ceiling  [1: 237 -- 15 --> 252]
H1-roofing  [1: 207 -- 5 --> 212]
H1-painting [1: 252 -- 10 --> 262]
H1-windows  [1: 262 -- 5 --> 267]
H1-facade   [1: 252 -- 10 --> 262]
H1-garden   [1: 262 -- 5 --> 267]
H1-moving   [1: 267 -- 5 --> 272]
H2-masonry  [1: 216 -- 35 --> 251]
H2-carpentry[1: 268 -- 15 --> 283]
H2-plumbing [1: 273 -- 40 --> 313]
H2-ceiling  [1: 313 -- 15 --> 328]
H2-roofing  [1: 283 -- 5 --> 288]
H2-painting [1: 328 -- 10 --> 338]
H2-windows  [1: 338 -- 5 --> 343]
H2-facade   [1: 333 -- 10 --> 343]
H2-garden   [1: 328 -- 5 --> 333]
H2-moving   [1: 343 -- 5 --> 348]
H3-masonry  [1: 59 -- 35 --> 94]
H3-carpentry[1: 115 -- 15 --> 130]
H3-plumbing [1: 120 -- 40 --> 160]
H3-ceiling  [1: 160 -- 15 --> 175]
H3-roofing  [1: 130 -- 5 --> 135]
H3-painting [1: 175 -- 10 --> 185]
H3-windows  [1: 185 -- 5 --> 190]
H3-facade   [1: 180 -- 10 --> 190]
H3-garden   [1: 175 -- 5 --> 180]
H3-moving   [1: 190 -- 5 --> 195]
H4-masonry  [1: 291 -- 35 --> 326]
H4-carpentry[1: 345 -- 15 --> 360]
H4-plumbing [1: 350 -- 40 --> 390]
H4-ceiling  [1: 390 -- 15 --> 405]
H4-roofing  [1: 360 -- 5 --> 365]
H4-painting [1: 405 -- 10 --> 415]
H4-windows  [1: 415 -- 5 --> 420]
H4-facade   [1: 390 -- 10 --> 400]
H4-garden   [1: 400 -- 5 --> 405]
H4-moving   [1: 420 -- 5 --> 425]
*/
