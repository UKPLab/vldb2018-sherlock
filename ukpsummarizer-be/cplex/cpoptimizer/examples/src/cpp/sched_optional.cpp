// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_optional.cpp
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

There are three workers, and each worker has a given non-negative
skill level for each task.  Each task requires one worker that will
have to be selected among the ones who have a non null skill level for
that task.  A worker can be assigned to only one task at a time.  Each
house has a deadline. The objective is to maximize the skill levels of
the workers assigned to the tasks while respecting the deadlines.

------------------------------------------------------------ */

#include <ilcp/cp.h>

const IloInt NbWorkers = 3;
const IloInt NbTasks = 10;

enum Workers {
  joe       = 0,
  jack      = 1,
  jim       = 2
};

const char* WorkerNames [] = {
  "Joe",
  "Jack",
  "Jim"
};

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
  "masonry",
  "carpentry",
  "plumbing",
  "ceiling",
  "roofing",
  "painting",
  "windows",
  "facade",
  "garden",
  "moving"
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

const IloInt SkillsMatrix [] = {
  // Joe, Jack, Jim
  9, 5, 0, // masonry
  7, 0, 5, // carpentry
  0, 7, 0, // plumbing
  5, 8, 0, // ceiling
  6, 7, 0, // roofing
  0, 9, 6, // painting
  8, 0, 5, // windows
  5, 5, 0, // facade
  5, 5, 9, // garden
  6, 0, 8  // moving
};

IloBool HasSkill(IloInt w, IloInt i) {
  return (0<SkillsMatrix[NbWorkers*i + w]);
}
IloInt SkillLevel(IloInt w, IloInt i) {
  return SkillsMatrix[NbWorkers*i + w];
}

void MakeHouse(IloModel model,
               IloIntExpr skill,
               IloIntervalVarArray allTasks,
               IloIntervalVarArray2 workerTasks,
               IloInt id,
               IloInt deadline) {
  IloEnv env = model.getEnv();

  /* CREATE THE INTERVAL VARIABLES. */
  char name[128];
  IloIntervalVarArray  tasks(env, NbTasks);
  IloIntervalVarArray2 taskMatrix(env, NbTasks);

  for (IloInt i=0; i<NbTasks; ++i) {
    sprintf(name, "H%ld-%s ", (long)id, TaskNames[i]);
    tasks[i] = IloIntervalVar(env, TaskDurations[i], name);
    taskMatrix[i] = IloIntervalVarArray(env, NbWorkers);

    /* ALLOCATING TASKS TO WORKERS. */
    IloIntervalVarArray alttasks(env);
    for (IloInt w=0; w<NbWorkers; ++w) {
      if (HasSkill(w, i)) {
        sprintf(name, "H%ld-%s-%s ", (long)id, TaskNames[i], WorkerNames[w]);
        IloIntervalVar wtask(env, TaskDurations[i], name);
        wtask.setOptional();
        alttasks.add(wtask);
        taskMatrix[i][w]=wtask;
        workerTasks[w].add(wtask);
        allTasks.add(wtask);
        /* DEFINING MAXIMIZATION OBJECTIVE. */
        skill += SkillLevel(w, i)*IloPresenceOf(env, wtask);
      }
    }
    model.add(IloAlternative(env, tasks[i], alttasks));
  }

  /* ADDING PRECEDENCE CONSTRAINTS. */
  tasks[moving].setEndMax(deadline);
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

  /* ADDING CONTINUITY CONSTRAINTS. */
  model.add(IloPresenceOf(env, taskMatrix[masonry][joe]) ==
            IloPresenceOf(env, taskMatrix[carpentry][joe]));
  model.add(IloPresenceOf(env, taskMatrix[roofing][jack]) ==
            IloPresenceOf(env, taskMatrix[facade][jack]));
  model.add(IloPresenceOf(env, taskMatrix[carpentry][joe]) ==
            IloPresenceOf(env, taskMatrix[roofing][joe]));
  model.add(IloPresenceOf(env, taskMatrix[garden][jim]) ==
            IloPresenceOf(env, taskMatrix[moving][jim]));
}

int main(int, const char * []) {
  IloEnv env;
  try {

    IloInt nbHouses = 5;
    IloInt deadline = 318;
    IloModel model(env);
    IloIntExpr skill(env);
    IloIntervalVarArray allTasks(env);
    IloIntervalVarArray2 workerTasks(env, NbWorkers);
    IloInt h, w;
    for (w=0; w<NbWorkers; ++w)
      workerTasks[w] = IloIntervalVarArray(env);

    for (h=0; h<nbHouses; ++h)
      MakeHouse(model, skill, allTasks, workerTasks, h, deadline);

    for (w=0; w<NbWorkers; ++w) {
      IloIntervalSequenceVar seq(env, workerTasks[w], WorkerNames[w]);
      model.add(IloNoOverlap(env, seq));
    }

    model.add(IloMaximize(env, skill));

    /* EXTRACTING THE MODEL AND SOLVING. */
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, 10000);
    if (cp.solve()) {
      cp.out() << "Solution with objective " << cp.getObjValue() << ":" << std::endl;
      for (IloInt i=0; i<allTasks.getSize(); ++i) {
        if (cp.isPresent(allTasks[i]))
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
Solution with objective 357:
H0-masonry-Joe [1: 0 -- 35 --> 35]
H0-carpentry-Joe [1: 170 -- 15 --> 185]
H0-plumbing-Jack [1: 65 -- 40 --> 105]
H0-ceiling-Jack [1: 50 -- 15 --> 65]
H0-roofing-Joe [1: 215 -- 5 --> 220]
H0-painting-Jim [1: 65 -- 10 --> 75]
H0-windows-Joe [1: 250 -- 5 --> 255]
H0-facade-Joe [1: 265 -- 10 --> 275]
H0-garden-Jim [1: 220 -- 5 --> 225]
H0-moving-Jim [1: 275 -- 5 --> 280]
H1-masonry-Joe [1: 35 -- 35 --> 70]
H1-carpentry-Joe [1: 140 -- 15 --> 155]
H1-plumbing-Jack [1: 215 -- 40 --> 255]
H1-ceiling-Jack [1: 105 -- 15 --> 120]
H1-roofing-Joe [1: 225 -- 5 --> 230]
H1-painting-Jim [1: 120 -- 10 --> 130]
H1-windows-Joe [1: 240 -- 5 --> 245]
H1-facade-Joe [1: 275 -- 10 --> 285]
H1-garden-Jim [1: 255 -- 5 --> 260]
H1-moving-Jim [1: 285 -- 5 --> 290]
H2-masonry-Joe [1: 105 -- 35 --> 140]
H2-carpentry-Joe [1: 155 -- 15 --> 170]
H2-plumbing-Jack [1: 175 -- 40 --> 215]
H2-ceiling-Joe [1: 185 -- 15 --> 200]
H2-roofing-Joe [1: 230 -- 5 --> 235]
H2-painting-Jim [1: 200 -- 10 --> 210]
H2-windows-Joe [1: 245 -- 5 --> 250]
H2-facade-Joe [1: 255 -- 10 --> 265]
H2-garden-Jim [1: 235 -- 5 --> 240]
H2-moving-Jim [1: 265 -- 5 --> 270]
H3-masonry-Joe [1: 70 -- 35 --> 105]
H3-carpentry-Joe [1: 200 -- 15 --> 215]
H3-plumbing-Jack [1: 255 -- 40 --> 295]
H3-ceiling-Jack [1: 120 -- 15 --> 135]
H3-roofing-Joe [1: 220 -- 5 --> 225]
H3-painting-Jim [1: 135 -- 10 --> 145]
H3-windows-Joe [1: 235 -- 5 --> 240]
H3-facade-Joe [1: 295 -- 10 --> 305]
H3-garden-Jim [1: 295 -- 5 --> 300]
H3-moving-Jim [1: 305 -- 5 --> 310]
H4-masonry-Jack [1: 0 -- 35 --> 35]
H4-carpentry-Jim [1: 103 -- 15 --> 118]
H4-plumbing-Jack [1: 135 -- 40 --> 175]
H4-ceiling-Jack [1: 35 -- 15 --> 50]
H4-roofing-Jack [1: 295 -- 5 --> 300]
H4-painting-Jim [1: 93 -- 10 --> 103]
H4-windows-Joe [1: 305 -- 5 --> 310]
H4-facade-Jack [1: 300 -- 10 --> 310]
H4-garden-Jim [1: 300 -- 5 --> 305]
H4-moving-Jim [1: 310 -- 5 --> 315]
*/

// Dates
// 0:  1/1/2007
// 31: 1/2/2007
// 59: 1/3/2007
// 90: 1/4/2007
// 120: 1/5/2007
// 151: 1/6/2007
// 181: 1/7/2007
// 212: 1/8/2007
// 243: 1/9/2007
// 273: 1/10/2007
// 304: 1/11/2007
// 334: 1/12/2007
