// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_calendar.cpp
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
painting, etc. must be scheduled.  Some tasks must necessarily take
place before others and these requirements are expressed through
precedence constraints.

There are two workers and each task requires a specific worker.  The
worker has a calendar of days off that must be taken into account. The
objective is to minimize the overall completion date.

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

void MakeHouse(IloModel model,
               IloInt id,
               IloIntExprArray ends,
               IloIntervalVarArray allTasks,
               IloIntervalVarArray joeTasks,
               IloIntervalVarArray jimTasks) {
  IloEnv env = model.getEnv();

  /* CREATE THE INTERVAL VARIABLES. */
  char name[128];
  IloIntervalVarArray tasks(env, NbTasks);
  for (IloInt i=0; i<NbTasks; ++i) {
    sprintf(name, "H%ld-%s", (long)id, TaskNames[i]);
    tasks[i] = IloIntervalVar(env, TaskDurations[i], name);
    allTasks.add(tasks[i]);
  }

  /* ADDING PRECEDENCE CONSTRAINTS. */
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

  /* ADDING WORKER TASKS. */
  joeTasks.add(tasks[masonry]);
  joeTasks.add(tasks[carpentry]);
  jimTasks.add(tasks[plumbing]);
  jimTasks.add(tasks[ceiling]);
  joeTasks.add(tasks[roofing]);
  jimTasks.add(tasks[painting]);
  jimTasks.add(tasks[windows]);
  joeTasks.add(tasks[facade]);
  joeTasks.add(tasks[garden]);
  jimTasks.add(tasks[moving]);

  /* DEFINING MINIMIZATION OBJECTIVE. */
  ends.add(IloEndOf(tasks[moving]));
}

int main(int, const char * []) {
  IloEnv env;
  try {

    IloInt nbHouses = 5;
    IloModel model(env);
    IloIntExprArray ends(env);
    IloIntervalVarArray allTasks(env);
    IloIntervalVarArray joeTasks(env);
    IloIntervalVarArray jimTasks(env);

    for (IloInt h=0; h<nbHouses; ++h) {
      MakeHouse(model, h, ends, allTasks, joeTasks, jimTasks);
    }

    model.add(IloNoOverlap(env, joeTasks));
    model.add(IloNoOverlap(env, jimTasks));

    IloNumToNumStepFunction joeCalendar(env);
    joeCalendar.setValue(0, 2*365, 100);
    IloNumToNumStepFunction jimCalendar(env);
    jimCalendar.setValue(0, 2*365, 100);
    

    /* WEEK ENDS. */
    for (IloInt w=0; w<2*52; ++w) {
      joeCalendar.setValue(5+(7.0*w), 7+(7.0*w), 0);
      jimCalendar.setValue(5+(7.0*w), 7+(7.0*w), 0);
    }

    /* HOLIDAYS. */
    joeCalendar.setValue(  5,  12, 0);
    joeCalendar.setValue(124, 131, 0);
    joeCalendar.setValue(215, 236, 0);
    joeCalendar.setValue(369, 376, 0);
    joeCalendar.setValue(495, 502, 0);
    joeCalendar.setValue(579, 600, 0);
    jimCalendar.setValue( 26,  40, 0);
    jimCalendar.setValue(201, 225, 0);
    jimCalendar.setValue(306, 313, 0);
    jimCalendar.setValue(397, 411, 0);
    jimCalendar.setValue(565, 579, 0);

    IloInt i;
    for (i=0; i<joeTasks.getSize(); ++i) {
      joeTasks[i].setIntensity(joeCalendar);
      model.add(IloForbidStart(env, joeTasks[i], joeCalendar));
      model.add(IloForbidEnd(env, joeTasks[i], joeCalendar));
    }
    for (i=0; i<jimTasks.getSize(); ++i) {
      jimTasks[i].setIntensity(jimCalendar);
      model.add(IloForbidStart(env, jimTasks[i], jimCalendar));
      model.add(IloForbidEnd(env, jimTasks[i], jimCalendar));
    }

    model.add(IloMinimize(env, IloMax(ends)));

    /* EXTRACTING THE MODEL AND SOLVING. */
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, 10000);
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
Solution with objective 638:
H0-masonry  [1: 0 -- (35)54 --> 54]
H0-carpentry[1: 301 -- (15)19 --> 320]
H0-plumbing [1: 77 -- (40)54 --> 131]
H0-ceiling  [1: 56 -- (15)19 --> 75]
H0-roofing  [1: 399 -- (5)5 --> 404]
H0-painting [1: 589 -- (10)14 --> 603]
H0-windows  [1: 498 -- (5)7 --> 505]
H0-facade   [1: 483 -- (10)12 --> 495]
H0-garden   [1: 441 -- (5)5 --> 446]
H0-moving   [1: 603 -- (5)7 --> 610]
H1-masonry  [1: 210 -- (35)68 --> 278]
H1-carpentry[1: 280 -- (15)19 --> 299]
H1-plumbing [1: 428 -- (40)56 --> 484]
H1-ceiling  [1: 316 -- (15)21 --> 337]
H1-roofing  [1: 392 -- (5)5 --> 397]
H1-painting [1: 526 -- (10)14 --> 540]
H1-windows  [1: 484 -- (5)7 --> 491]
H1-facade   [1: 511 -- (10)12 --> 523]
H1-garden   [1: 504 -- (5)5 --> 509]
H1-moving   [1: 631 -- (5)7 --> 638]
H2-masonry  [1: 105 -- (35)54 --> 159]
H2-carpentry[1: 364 -- (15)26 --> 390]
H2-plumbing [1: 337 -- (40)56 --> 393]
H2-ceiling  [1: 393 -- (15)35 --> 428]
H2-roofing  [1: 413 -- (5)5 --> 418]
H2-painting [1: 554 -- (10)28 --> 582]
H2-windows  [1: 582 -- (5)7 --> 589]
H2-facade   [1: 448 -- (10)12 --> 460]
H2-garden   [1: 462 -- (5)5 --> 467]
H2-moving   [1: 610 -- (5)7 --> 617]
H3-masonry  [1: 161 -- (35)47 --> 208]
H3-carpentry[1: 343 -- (15)19 --> 362]
H3-plumbing [1: 253 -- (40)63 --> 316]
H3-ceiling  [1: 232 -- (15)21 --> 253]
H3-roofing  [1: 420 -- (5)5 --> 425]
H3-painting [1: 540 -- (10)14 --> 554]
H3-windows  [1: 519 -- (5)7 --> 526]
H3-facade   [1: 469 -- (10)12 --> 481]
H3-garden   [1: 525 -- (5)5 --> 530]
H3-moving   [1: 624 -- (5)7 --> 631]
H4-masonry  [1: 56 -- (35)47 --> 103]
H4-carpentry[1: 322 -- (15)19 --> 341]
H4-plumbing [1: 133 -- (40)54 --> 187]
H4-ceiling  [1: 189 -- (15)43 --> 232]
H4-roofing  [1: 406 -- (5)5 --> 411]
H4-painting [1: 505 -- (10)14 --> 519]
H4-windows  [1: 491 -- (5)7 --> 498]
H4-facade   [1: 427 -- (10)12 --> 439]
H4-garden   [1: 532 -- (5)5 --> 537]
H4-moving   [1: 617 -- (5)7 --> 624]
*/
