// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_time.cpp
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

This is a problem of building a house. The masonry, roofing, painting,
etc. must be scheduled.  Some tasks must necessarily take place before
others and these requirements are expressed through precedence
constraints.

Moreover, there are earliness and tardiness costs associated with some
tasks. The objective is to minimize these costs.

------------------------------------------------------------ */

#include <ilcp/cp.h>

IloNumExpr EarlinessCost(IloIntervalVar task, IloInt rd, IloNum weight, IloBool useFunction) {
  IloEnv env = task.getEnv();
  if (useFunction) {
    IloNumToNumSegmentFunction f =
      IloPiecewiseLinearFunction(env,
         IloNumArray(env, 1, IloNum(rd)),
         IloNumArray(env, 2, -weight, 0.0),
         IloNum(rd), 0.0);
    return IloStartEval(task, f);
  }
  else
    return weight * IloMax(0, rd-IloStartOf(task));
}

IloNumExpr TardinessCost(IloIntervalVar task, IloInt dd, IloNum weight, IloBool useFunction) {
  IloEnv env = task.getEnv();
  if (useFunction) {
   IloNumToNumSegmentFunction f =
      IloPiecewiseLinearFunction(env,
         IloNumArray(env, 1, IloNum(dd)),
         IloNumArray(env, 2, 0.0, weight),
         IloNum(dd), 0.0);
    return IloEndEval(task, f);
  }
  else
    return weight * IloMax(0, IloEndOf(task)-dd);
}

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

  /* ADDING PRECEDENCE CONSTRAINTS. */
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

  /* DEFINING MINIMIZATION OBJECTIVE. */
    IloBool useFunction = IloTrue;
    IloNumExpr cost(env);
    cost += EarlinessCost(masonry,   25, 200.0, useFunction);
    cost += EarlinessCost(carpentry, 75, 300.0, useFunction);
    cost += EarlinessCost(ceiling,   75, 100.0, useFunction);
    cost += TardinessCost(moving,   100, 400.0, useFunction);
    model.add(IloMinimize(env, cost));

  /* EXTRACTING THE MODEL AND SOLVING. */
    IloCP cp(model);
    if (cp.solve()) {
      cp.out() << "Cost Value: " << cp.getObjValue() << std::endl;
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
Cost Value: 5000
masonry   [1: 20 -- 35 --> 55]
carpentry [1: 75 -- 15 --> 90]
plumbing  [1: 55 -- 40 --> 95]
ceiling   [1: 75 -- 15 --> 90]
roofing   [1: 90 -- 5 --> 95]
painting  [1: 90 -- 10 --> 100]
windows   [1: 95 -- 5 --> 100]
facade    [1: 95 -- 10 --> 105]
garden    [1: 95 -- 5 --> 100]
moving    [1: 105 -- 5 --> 110]
*/



