// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_state.cpp
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

This is a problem of building five houses. The masonry, roofing,
painting, etc. must be scheduled. Some tasks must necessarily take
place before others and these requirements are expressed through
precedence constraints.

A pool of two workers is available for building the houses. For a
given house, some tasks (namely: plumbing, ceiling and painting)
require the house to be clean whereas other tasks (namely: masonry,
carpentry, roofing and windows) put the house in a dirty state. A
transition time of 1 is needed to clean the house so to change from
state 'dirty' to state 'clean'.

The objective is to minimize the makespan.

------------------------------------------------------------ */

#include <ilcp/cp.h>
#include <sstream>

const IloInt NbHouses  = 5;
const IloInt NbTasks   = 10;
const IloInt NbWorkers = 2;

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

/* POSSIBLE HOUSE STATE. */
const IloInt Clean = 0;
const IloInt Dirty = 1;

void MakeHouse(IloModel model,
         IloInt id,
         IloIntExprArray ends,
         IloIntervalVarArray allTasks,
         IloCumulFunctionExpr& workers,
         IloStateFunction houseState) {
  IloEnv env = model.getEnv();

  /* CREATE THE INTERVAL VARIABLES. */
  IloIntervalVarArray tasks(env, NbTasks);
  for (IloInt i=0; i<NbTasks; ++i) {
    std::ostringstream name;
    name << "H"  << id << "-" << TaskNames[i];
    tasks[i] = IloIntervalVar(env, TaskDurations[i], name.str().c_str());
    workers += IloPulse(tasks[i], 1);
    allTasks.add(tasks[i]);
  }

  /* PRECEDENCE CONSTRAINTS. */
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

  /* STATE CONSTRAINTS. */
  model.add(IloAlwaysEqual(env, houseState, tasks[masonry],   Dirty));
  model.add(IloAlwaysEqual(env, houseState, tasks[carpentry], Dirty));
  model.add(IloAlwaysEqual(env, houseState, tasks[plumbing],  Clean));
  model.add(IloAlwaysEqual(env, houseState, tasks[ceiling],   Clean));
  model.add(IloAlwaysEqual(env, houseState, tasks[roofing],   Dirty));
  model.add(IloAlwaysEqual(env, houseState, tasks[painting],  Clean));
  model.add(IloAlwaysEqual(env, houseState, tasks[windows],   Dirty));

  /* MAKESPAN. */
  ends.add(IloEndOf(tasks[moving]));
}
int main(int, const char * []) {
  IloEnv env;
  try {

    IloInt i;
    IloModel model(env);
    IloIntExprArray ends(env);
    IloIntervalVarArray allTasks(env);
    IloCumulFunctionExpr workers(env);
    IloTransitionDistance ttime(env, 2);
    ttime.setValue(Dirty, Clean, 1);

    IloStateFunctionArray houseState(env, NbHouses);
    for (i=0; i<NbHouses; ++i) {
      houseState[i] = IloStateFunction(env, ttime);
      MakeHouse(model, i, ends, allTasks, workers, houseState[i]);
    }

    model.add(workers <= NbWorkers);
    model.add(IloMinimize(env, IloMax(ends)));

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, 10000);
    if (cp.solve()) {
      cp.out() << "Solution with objective " << cp.getObjValue() << ":" << std::endl;
      for (i=0; i<allTasks.getSize(); ++i) {
        cp.out() << cp.domain(allTasks[i]) << std::endl;
      }
      for (IloInt h = 0; h < NbHouses; h++) {
        for (i = 0; i < cp.getNumberOfSegments(houseState[h]); i++) {
          cp.out() << "House " << h << " has state ";
          IloInt s = cp.getSegmentValue(houseState[h], i);
          if      (s == Clean)          cp.out() << "Clean";
          else if (s == Dirty)          cp.out() << "Dirty";
          else if (s == IloCP::NoState) cp.out() << "None";
          else                          cp.out() << "Unknown (problem)";
          cp.out() << " from ";
                  if (cp.getSegmentStart(houseState[h], i) == IloIntervalMin)   cp.out() << "Min";
                  else  cp.out() << cp.getSegmentStart(houseState[h], i);
                  cp.out() << " to ";
                  if (cp.getSegmentEnd(houseState[h], i) == IloIntervalMax)     cp.out() << "Max";
                  else cp.out() << cp.getSegmentEnd(houseState[h], i) - 1;
                  cp.out() << std::endl;
        }
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
Solution with objective 365:
H0-masonry  [1: 0 -- 35 --> 35]
H0-carpentry[1: 70 -- 15 --> 85]
H0-plumbing [1: 145 -- 40 --> 185]
H0-ceiling  [1: 120 -- 15 --> 135]
H0-roofing  [1: 195 -- 5 --> 200]
H0-painting [1: 340 -- 10 --> 350]
H0-windows  [1: 290 -- 5 --> 295]
H0-facade   [1: 320 -- 10 --> 330]
H0-garden   [1: 285 -- 5 --> 290]
H0-moving   [1: 350 -- 5 --> 355]
H1-masonry  [1: 70 -- 35 --> 105]
H1-carpentry[1: 105 -- 15 --> 120]
H1-plumbing [1: 205 -- 40 --> 245]
H1-ceiling  [1: 130 -- 15 --> 145]
H1-roofing  [1: 175 -- 5 --> 180]
H1-painting [1: 310 -- 10 --> 320]
H1-windows  [1: 280 -- 5 --> 285]
H1-facade   [1: 305 -- 10 --> 315]
H1-garden   [1: 265 -- 5 --> 270]
H1-moving   [1: 360 -- 5 --> 365]
H2-masonry  [1: 35 -- 35 --> 70]
H2-carpentry[1: 115 -- 15 --> 130]
H2-plumbing [1: 245 -- 40 --> 285]
H2-ceiling  [1: 250 -- 15 --> 265]
H2-roofing  [1: 200 -- 5 --> 205]
H2-painting [1: 335 -- 10 --> 345]
H2-windows  [1: 345 -- 5 --> 350]
H2-facade   [1: 290 -- 10 --> 300]
H2-garden   [1: 305 -- 5 --> 310]
H2-moving   [1: 350 -- 5 --> 355]
H3-masonry  [1: 35 -- 35 --> 70]
H3-carpentry[1: 100 -- 15 --> 115]
H3-plumbing [1: 205 -- 40 --> 245]
H3-ceiling  [1: 180 -- 15 --> 195]
H3-roofing  [1: 245 -- 5 --> 250]
H3-painting [1: 325 -- 10 --> 335]
H3-windows  [1: 300 -- 5 --> 305]
H3-facade   [1: 315 -- 10 --> 325]
H3-garden   [1: 330 -- 5 --> 335]
H3-moving   [1: 355 -- 5 --> 360]
H4-masonry  [1: 0 -- 35 --> 35]
H4-carpentry[1: 85 -- 15 --> 100]
H4-plumbing [1: 135 -- 40 --> 175]
H4-ceiling  [1: 185 -- 15 --> 200]
H4-roofing  [1: 200 -- 5 --> 205]
H4-painting [1: 295 -- 10 --> 305]
H4-windows  [1: 285 -- 5 --> 290]
H4-facade   [1: 270 -- 10 --> 280]
H4-garden   [1: 335 -- 5 --> 340]
H4-moving   [1: 355 -- 5 --> 360]
*/
