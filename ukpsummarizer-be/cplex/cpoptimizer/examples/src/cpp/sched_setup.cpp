// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_setup.cpp
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

This example solves a scheduling problem on two alternative
heterogeneous machines. A set of tasks {a_1,...,a_n} has to be
executed on either one of the two machines. Different types of tasks
are distinguished, the type of task a_i is denoted tp_i.  

A machine m needs a sequence dependent setup time setup(tp,tp') to
switch from a task of type tp to the next task of type
tp'. Furthermore some transitions tp->tp' are forbidden.

The two machines are different: they process tasks with different
speed and have different setup times and forbidden transitions.

The objective is to minimize the makespan.

The model uses transition distances and noOverlap constraints to model
machines setup times. The noOverlap constraint is specified to enforce
transition distance between immediate successors on the
sequence. Forbidden transitions are modeled with a very large
transition distance.

------------------------------------------------------------ */

#include <ilcp/cp.h>

const IloInt NbTypes = 5;

// Setup times of machine M1; -1 means forbidden transition
const IloInt SetupM1[] = {
  0,  26,  8,  3, -1,
  22,  0, -1,  4, 22,
  28,  0,  0, 23,  9,
  29, -1, -1,  0,  8,
  26, 17, 11,  7,  0
};

// Setup times of machine M2; -1 means forbidden transition
const IloInt SetupM2[] = {
  0,  5, 28, -1,  2,
  -1, 0, -1,  7, 10,
  19, 22,  0, 28, 17,
  7, 26, 13,  0, -1,
  13, 17, 26, 20, 0
};

const IloInt NbTasks = 50;

// Task type
const IloInt TaskType[] = {
  3, 3, 1, 1, 1, 1, 2, 0, 0, 2,
  4, 4, 3, 3, 2, 3, 1, 4, 4, 2,
  2, 1, 4, 2, 2, 0, 3, 3, 2, 1,
  2, 1, 4, 3, 3, 0, 2, 0, 0, 3,
  2, 0, 3, 2, 2, 4, 1, 2, 4, 3
};

// Task duration if executed on machine M1
const IloInt TaskDurM1[] = {
  4, 17,  4,  7, 17, 14,  2, 14,  2,  8,
  11, 14,  4, 18,  3,  2,  9,  2,  9, 17,
  18, 19,  5,  8, 19, 12, 17, 11,  6,  3,
  13,  6, 19,  7,  1,  3, 13,  5,  3,  6,
  11, 16, 12, 14, 12, 17,  8,  8,  6,  6
};

// Task duration if executed on machine M2
const IloInt TaskDurM2[] = {
  12,  3, 12, 15,  4,  9, 14,  2,  5,  9,
  10, 14,  7,  1, 11,  3, 15, 19,  8,  2,
  18, 17, 19, 18, 15, 14,  6,  6,  1,  2,
  3, 19, 18,  2,  7, 16,  1, 18, 10, 14,
  2,  3, 14,  1,  1,  6, 19,  5, 17,  4
};

int main(int, const char*[]){
  IloEnv env;
  try {
    IloModel model(env);
    IloTransitionDistance setup1(env, NbTypes);
    IloTransitionDistance setup2(env, NbTypes);
    IloInt i, j;
    for (i=0; i<NbTypes; ++i) {
      for (j=0; j<NbTypes; ++j) {
        IloInt d1 = SetupM1[NbTypes*i+j];
        if (d1<0)
          d1 = IloIntervalMax; // Forbidden transition
        setup1.setValue(i,j,d1);
        IloInt d2 = SetupM2[NbTypes*i+j];
        if (d2<0)
          d2 = IloIntervalMax; // Forbidden transition
        setup2.setValue(i,j,d2);
      }
    }
    IloIntArray tp(env, NbTasks);
    IloIntervalVarArray a (env, NbTasks);
    IloIntervalVarArray a1(env, NbTasks);
    IloIntervalVarArray a2(env, NbTasks);
    IloIntExprArray ends(env, NbTasks);
    char name[64];

    for (i=0; i<NbTasks; ++i) {
      IloInt type = TaskType [i];
      IloInt d1   = TaskDurM1[i];
      IloInt d2   = TaskDurM2[i];
      tp[i] = type;
      sprintf(name, "A%ld_TP%ld", (long)i, (long)type);
      a[i] = IloIntervalVar(env, name);
      IloIntervalVarArray alt(env, 2);
      sprintf(name, "A%ld_M1_TP%ld", (long)i, (long)type);
      a1[i] = IloIntervalVar(env, d1, name);  
      a1[i].setOptional();
      alt[0]=a1[i];
      sprintf(name, "A%ld_M2_TP%ld", (long)i, (long)type);
      a2[i] = IloIntervalVar(env, d2, name);  
      a2[i].setOptional();
      alt[1]=a2[i];
      model.add(IloAlternative(env, a[i], alt));
      ends[i]=IloEndOf(a[i]);
    }

    IloIntervalSequenceVar s1(env, a1, tp);
    IloIntervalSequenceVar s2(env, a2, tp);
    model.add(IloNoOverlap(env, s1, setup1, IloTrue));
    model.add(IloNoOverlap(env, s2, setup2, IloTrue));
    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, 100000);
    cp.setParameter(IloCP::LogPeriod, 10000);
    if (cp.solve()) {
      cp.out() << "Machine 1: " << std::endl;
      IloIntervalVar act;
      for (act = cp.getFirst(s1); act.getImpl() != 0; act = cp.getNext(s1, act))
        cp.out() << cp.domain(act) << std::endl;
      cp.out() << "Machine 2: " << std::endl;
      for (act = cp.getFirst(s2); act.getImpl() != 0; act = cp.getNext(s2, act))
        cp.out() << cp.domain(act) << std::endl;
      cp.out() << "Makespan \t: " << cp.getObjValue() << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }

  } catch(IloException& e){
    env.out() << " Error: " << e << std::endl;
  }
  env.end();
  return 0;
}
