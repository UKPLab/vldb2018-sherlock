// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_square.cpp
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

The aim of the square example is to place a set of small squares of
different sizes into a large square.

------------------------------------------------------------ */

#include <ilcp/cp.h>

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt i,j;
    IloInt sizeSquare = 112;
    IloInt nbSquares  = 21;
    IloIntArray size(env, nbSquares, 50,42,37,35,33,29,27,25,24,19,18,17,16,15,11,9,8,7,6,4,2);
    IloIntervalVarArray x(env, nbSquares);
    IloIntervalVarArray y(env, nbSquares);
    IloCumulFunctionExpr rx(env);
    IloCumulFunctionExpr ry(env);
    char name[16];
    for (i=0; i<nbSquares; ++i) {
      sprintf(name, "X%ld", (long)i);
      x[i] = IloIntervalVar(env, size[i], name);
      x[i].setEndMax(sizeSquare);
      sprintf(name, "Y%ld", (long)i);
      y[i] = IloIntervalVar(env, size[i], name);
      y[i].setEndMax(sizeSquare);
      rx += IloPulse(x[i], size[i]);
      ry += IloPulse(y[i], size[i]);
      for (j=0; j<i; ++j)
        model.add((IloEndOf(x[i]) <= IloStartOf(x[j])) ||
                  (IloEndOf(x[j]) <= IloStartOf(x[i])) ||
                  (IloEndOf(y[i]) <= IloStartOf(y[j])) ||
                  (IloEndOf(y[j]) <= IloStartOf(y[i])));
    }
    model.add(IloAlwaysIn(env, rx, 0, sizeSquare, sizeSquare, sizeSquare));
    model.add(IloAlwaysIn(env, ry, 0, sizeSquare, sizeSquare, sizeSquare));
    IloCP cp(model);
    IloSearchPhaseArray phases(env);
    phases.add(IloSearchPhase(env, x));
    phases.add(IloSearchPhase(env, y));
    cp.setSearchPhases(phases);
    if (cp.solve()) {
      for (i=0; i<nbSquares; ++i) {
        cp.out() << "Square " << i << ": ["
                 << cp.getStart(x[i]) << "," << cp.getEnd(x[i])
                 << "] x ["
                 << cp.getStart(y[i]) << "," << cp.getEnd(y[i])
                 << "]" << std::endl;
      }
    }
  } catch (IloException& ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
