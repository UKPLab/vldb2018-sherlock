// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_tcost.cpp
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

The problem is to schedule a set of tasks on two alternative
machines with different setup times.

The objective is to minimize the number of "long" setup times on
machines. A setup time is considered to be long if it is larger than
30.

------------------------------------------------------------ */

#include <ilcp/cp.h>

IloInt NbTypes = 10;

IloInt SetupM1[] = {
  22, 24,  7, 10, 9,  41, 14, 30, 24,  6,
  63, 21, 42,  1, 24, 17, 35, 25,  0, 68,
  60, 70, 37, 70, 39, 84, 44, 60, 67, 36,
  77, 57, 65, 33, 81, 74, 72, 82, 57, 83,
  51, 31, 18, 32, 48, 45, 51, 21, 28, 45,
  46, 42, 29, 11, 11, 21, 59,  8,  4, 51,
  35, 59, 42, 45, 44, 76, 37, 65, 59, 41,
  38, 62, 45, 14, 33, 24, 52, 32,  7, 44,
  63, 57, 44,  7, 26, 17, 55, 25, 21, 68,
  24, 34,  1, 34,  3, 48,  8, 24, 31, 30
};
  
IloInt SetupM2[] = {
  27, 48, 44, 52, 21, 61, 33,  5, 37, 64,
  42, 44, 42, 40, 17, 40, 49, 41, 66, 29,
  36, 53, 31, 56, 50, 56,  7, 41, 49, 60,
  6, 43, 46, 38, 16, 44, 39, 11, 43, 12,
  25, 27, 45, 67, 37, 67, 52, 30, 62, 56,
  6, 43,  2,  0, 16, 35,  9, 11, 43, 12,
  29, 70, 25, 62, 43, 62, 26, 34, 42, 61,
  22, 43, 53, 47, 16, 56, 28, 10, 32, 59,
  56, 93, 73, 76, 66, 82, 48, 61, 51, 50,
  18, 55, 34, 26, 28, 32, 40, 12, 44, 25
};

IloInt NbTasks = 50;

IloInt TaskDur[] = {
  19, 18, 16, 11, 16, 15, 19, 18, 17, 17, 
  20, 16, 16, 14, 19, 11, 10, 16, 12, 20, 
  14, 14, 20, 12, 18, 16, 10, 15, 11, 13,
  15, 11, 11, 13, 19, 17, 11, 20, 19, 17,
  15, 19, 13, 16, 20, 13, 13, 13, 13, 15
};

IloInt TaskType[]= {
  8,  1,  6,  3,  4,  8,  8,  4,  3,  5, 
  9,  4,  1,  5,  8,  8,  4,  1,  9,  2,
  6,  0,  8,  9,  1,  0,  1,  7,  5,  9,
  3,  1,  9,  3,  0,  7,  0,  7,  1,  4, 
  5,  7,  4,  0,  9,  1,  5,  4,  5,  1
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
        setup1.setValue(i,j,SetupM1[NbTypes*i+j]);
        setup2.setValue(i,j,SetupM2[NbTypes*i+j]);
      }
    }
    IloIntArray tp(env, NbTasks);
    IloIntervalVarArray a (env, NbTasks);
    IloIntervalVarArray a1(env, NbTasks);
    IloIntervalVarArray a2(env, NbTasks);
    char name[64];
    for (i=0; i<NbTasks; ++i) {
      IloInt type = TaskType[i];
      IloInt d    = TaskDur [i];
      tp[i] = type;
      sprintf(name, "A%ld_TP%ld", (long)i, (long)type);
      a[i] = IloIntervalVar(env, d, name);
      IloIntervalVarArray alt(env, 2);
      sprintf(name, "A%ld_M1_TP%ld", (long)i, (long)type);
      a1[i] = IloIntervalVar(env, name);  
      a1[i].setOptional();
      alt[0]=a1[i];
      sprintf(name, "A%ld_M2_TP%ld", (long)i, (long)type);
      a2[i] = IloIntervalVar(env, name);  
      a2[i].setOptional();
      alt[1]=a2[i];
      model.add(IloAlternative(env, a[i], alt));
    }

    IloIntervalSequenceVar s1(env, a1, tp);
    IloIntervalSequenceVar s2(env, a2, tp);
    model.add(IloNoOverlap(env, s1, setup1, IloTrue));
    model.add(IloNoOverlap(env, s2, setup2, IloTrue));

    IloIntExpr nbLongSetups(env);
    for (i=0; i<NbTasks; ++i) {
      IloInt tpi = TaskType[i];
      IloIntArray isLongSetup1(env, NbTypes+1);
      IloIntArray isLongSetup2(env, NbTypes+1);
      for (j=0; j<NbTypes; ++j) {
        isLongSetup1[j] = (30<=SetupM1[NbTypes*tpi+j])?1:0;
        isLongSetup2[j] = (30<=SetupM2[NbTypes*tpi+j])?1:0;
      }
      isLongSetup1[NbTypes] = 0; // Last on resource or resource not selected
      isLongSetup2[NbTypes] = 0; // Last on resource or resource not selected
      nbLongSetups += isLongSetup1[IloTypeOfNext(s1, a1[i], NbTypes, NbTypes)];
      nbLongSetups += isLongSetup2[IloTypeOfNext(s2, a2[i], NbTypes, NbTypes)];
    }
    IloObjective objective = IloMinimize(env,nbLongSetups);
    model.add(objective);

    IloCP cp(model);

    cp.setParameter(IloCP::LogPeriod, 10000);

    if (cp.solve()) {
      cp.out() << "Machine 1: " << std::endl;
      IloIntervalVar act;
      for (act = cp.getFirst(s1); act.getImpl() != 0; act = cp.getNext(s1, act))
        cp.out() << cp.domain(act) << std::endl;
      cp.out() << "Machine 2: " << std::endl;
      for (act = cp.getFirst(s2); act.getImpl() != 0; act = cp.getNext(s2, act))
        cp.out() << cp.domain(act) << std::endl;
      cp.out() << "Number of long transition times \t: " << cp.getObjValue() << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }

  } catch(IloException& e){
    env.out() << " Error: " << e << std::endl;
  }
  env.end();
  return 0;
}
