// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedTCost.java
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2017. All Rights Reserved.
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

import ilog.concert.*;
import ilog.cp.*;

public class SchedTCost {

    static final int NbTypes = 10;

    static final int[] SetupM1 = {
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

    static final int[] SetupM2 = {
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

    static final int NbTasks = 50;

    static final int[] TaskDur = {
        19, 18, 16, 11, 16, 15, 19, 18, 17, 17, 
        20, 16, 16, 14, 19, 11, 10, 16, 12, 20, 
        14, 14, 20, 12, 18, 16, 10, 15, 11, 13,
        15, 11, 11, 13, 19, 17, 11, 20, 19, 17,
        15, 19, 13, 16, 20, 13, 13, 13, 13, 15
    };

    static final int[] TaskType = {
        8,  1,  6,  3,  4,  8,  8,  4,  3,  5, 
        9,  4,  1,  5,  8,  8,  4,  1,  9,  2,
        6,  0,  8,  9,  1,  0,  1,  7,  5,  9,
        3,  1,  9,  3,  0,  7,  0,  7,  1,  4, 
        5,  7,  4,  0,  9,  1,  5,  4,  5,  1
    };
    
    public static void main(String[] args) {

        try {

            IloCP cp = new IloCP();

            IloTransitionDistance setup1 = cp.transitionDistance(NbTypes);
            IloTransitionDistance setup2 = cp.transitionDistance(NbTypes);
            int i, j;
            for (i=0; i<NbTypes; ++i) {
                for (j=0; j<NbTypes; ++j) {
                    setup1.setValue(i,j,SetupM1[NbTypes*i+j]);
                    setup2.setValue(i,j,SetupM2[NbTypes*i+j]);
                }
            }
            int[] tp = new int[NbTasks];
            IloIntervalVar[] a  = new IloIntervalVar[NbTasks];
            IloIntervalVar[] a1 = new IloIntervalVar[NbTasks];
            IloIntervalVar[] a2 = new IloIntervalVar[NbTasks];

            String name;
            for (i=0; i<NbTasks; ++i) {
                int type = TaskType[i];
                int d    = TaskDur [i];
                tp[i] = type;
                name = "A" + i + "_TP" + type;
                a[i] = cp.intervalVar(d, name);
                IloIntervalVar[] alt = new IloIntervalVar[2];
                name = "A" + i + "_M1_TP" + type;
                a1[i] = cp.intervalVar(name);
                a1[i].setOptional();
                alt[0]=a1[i];
                name = "A" + i + "_M2_TP" + type;
                a2[i] = cp.intervalVar(name);
                a2[i].setOptional();
                alt[1]=a2[i];
                cp.add(cp.alternative(a[i], alt));
            }

            IloIntervalSequenceVar s1 = cp.intervalSequenceVar(a1, tp);
            IloIntervalSequenceVar s2 = cp.intervalSequenceVar(a2, tp);
            cp.add(cp.noOverlap(s1, setup1, true));
            cp.add(cp.noOverlap(s2, setup2, true));

            IloIntExpr nbLongSetups = cp.intExpr();
            for (i=0; i<NbTasks; ++i) {
                int tpi = TaskType[i];
                int[] isLongSetup1 = new int[NbTypes+1];
                int[] isLongSetup2 = new int[NbTypes+1];
                for (j=0; j<NbTypes; ++j) {
                    isLongSetup1[j] = (30<=SetupM1[NbTypes*tpi+j])?1:0;
                    isLongSetup2[j] = (30<=SetupM2[NbTypes*tpi+j])?1:0;
                }
                isLongSetup1[NbTypes] = 0; // Last on resource or resource not selected
                isLongSetup2[NbTypes] = 0; // Last on resource or resource not selected
                nbLongSetups = cp.sum(nbLongSetups, 
                                      cp.element(isLongSetup1, cp.typeOfNext(s1, a1[i], NbTypes, NbTypes)));
                nbLongSetups = cp.sum(nbLongSetups, 
                                      cp.element(isLongSetup2, cp.typeOfNext(s2, a2[i], NbTypes, NbTypes)));
            }
            cp.add(cp.minimize(nbLongSetups));
            
            cp.setParameter(IloCP.IntParam.FailLimit, 100000);
            cp.setParameter(IloCP.IntParam.LogPeriod, 10000);
            if (cp.solve()) {
                System.out.println("Machine 1: ");
                IloIntervalVar x;
                for (x = cp.getFirst(s1); !x.equals(cp.getLast(s1)); x = cp.getNext(s1, x))
                    System.out.println(cp.getDomain(x));
                System.out.println(cp.getDomain(x));
                System.out.println("Machine 2: ");
                for (x = cp.getFirst(s2); !x.equals(cp.getLast(s2)); x = cp.getNext(s2, x))
                    System.out.println(cp.getDomain(x));
                System.out.println(cp.getDomain(x));
                System.out.println("Number of long transition times \t: " + cp.getObjValue());
            } else {
                System.out.println("No solution found.");
            }
            
        } catch (IloException e) {
            System.out.println("Error: " + e);
        }
    }
}
