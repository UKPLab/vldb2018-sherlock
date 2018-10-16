// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_intro.cpp
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

This is a basic problem that involves building a house. The masonry,
roofing, painting, etc.  must be scheduled. Some tasks must
necessarily take place before others, and these requirements are
expressed through precedence constraints.

------------------------------------------------------------ */

#include <ilcp/cp.h>

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);

    /* CREATE THE INTERVAL VARIABLES. */
    IloIntervalVar masonry  (env, 35, "masonry   ");
    IloIntervalVar carpentry(env, 15, "carpentry ");
    IloIntervalVar plumbing (env, 40, "plumbing  ");
    IloIntervalVar ceiling  (env, 15, "ceiling   ");
    IloIntervalVar roofing  (env, 5,  "roofing   ");
    IloIntervalVar painting (env, 10, "painting  ");
    IloIntervalVar windows  (env, 5,  "windows   ");
    IloIntervalVar facade   (env, 10, "facade    ");
    IloIntervalVar garden   (env, 5,  "garden    ");
    IloIntervalVar moving   (env, 5,  "moving    ");

  /* ADDING TEMPORAL CONSTRAINTS. */
    model.add(IloEndBeforeStart(env, masonry,   carpentry));
    model.add(IloEndBeforeStart(env, masonry,   plumbing));
    model.add(IloEndBeforeStart(env, masonry,   ceiling));
    model.add(IloEndBeforeStart(env, carpentry, roofing));
    model.add(IloEndBeforeStart(env, ceiling,   painting));
    model.add(IloEndBeforeStart(env, roofing,   windows));
    model.add(IloEndBeforeStart(env, roofing,   facade));
    model.add(IloEndBeforeStart(env, plumbing,  facade));
    model.add(IloEndBeforeStart(env, roofing,   garden));
    model.add(IloEndBeforeStart(env, plumbing,  garden));
    model.add(IloEndBeforeStart(env, windows,   moving));
    model.add(IloEndBeforeStart(env, facade,    moving));
    model.add(IloEndBeforeStart(env, garden,    moving));
    model.add(IloEndBeforeStart(env, painting,  moving));

  /* EXTRACTING THE MODEL AND SOLVING. */
    IloCP cp(model);
    if (cp.solve()) {
      cp.out() << cp.domain(masonry)   << std::endl;
      cp.out() << cp.domain(carpentry) << std::endl;
      cp.out() << cp.domain(plumbing)  << std::endl;
      cp.out() << cp.domain(ceiling)   << std::endl;
      cp.out() << cp.domain(roofing)   << std::endl;
      cp.out() << cp.domain(painting)  << std::endl;
      cp.out() << cp.domain(windows)   << std::endl;
      cp.out() << cp.domain(facade)    << std::endl;
      cp.out() << cp.domain(garden)    << std::endl;
      cp.out() << cp.domain(moving)    << std::endl;
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
 ! ----------------------------------------------------------------------------
 ! Satisfiability problem - 10 variables, 14 constraints
 ! Initial process time : 0.00s (0.00s extraction + 0.00s propagation)
 !  . Log search space  : 33.2 (before), 33.2 (after)
 !  . Memory usage      : 315.4 Kb (before), 315.4 Kb (after)
 ! ----------------------------------------------------------------------------
 !   Branches  Non-fixed                Branch decision
 *         12      0.00s                  on moving
 ! ----------------------------------------------------------------------------
 ! Solution status        : Terminated normally, solution found
 ! Number of branches     : 12
 ! Number of fails        : 0
 ! Total memory usage     : 347.2 Kb (331.4 Kb CP Optimizer + 15.8 Kb Concert)
 ! Time spent in solve    : 0.00s (0.00s engine + 0.00s extraction)
 ! Search speed (br. / s) : 1200.0
 ! ----------------------------------------------------------------------------
masonry   [1: 0 -- 35 --> 35]
carpentry [1: 35 -- 15 --> 50]
plumbing  [1: 35 -- 40 --> 75]
ceiling   [1: 35 -- 15 --> 50]
roofing   [1: 50 -- 5 --> 55]
painting  [1: 50 -- 10 --> 60]
windows   [1: 55 -- 5 --> 60]
facade    [1: 75 -- 10 --> 85]
garden    [1: 75 -- 5 --> 80]
moving    [1: 85 -- 5 --> 90]
Number of branches      : 12
Number of fails         : 0
Number of choice points : 13
Number of variables     : 10
Number of constraints   : 2
Total memory usage      : 347.2 Kb (331.4 Kb CP + 15.8 Kb Concert)
Time in last solve      : 0.00s (0.00s engine + 0.00s extraction)
Total time spent in CP  : 0.02s
*/
