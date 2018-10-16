// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedSetup.java
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

import ilog.concert.*;
import ilog.cp.*;

public class SchedSetup {

    static final int NbTypes = 5;

    // Setup times of machine M1; -1 means forbidden transition
    static final int[] SetupM1 = {
        0,  26,  8,  3, -1,
        22,  0, -1,  4, 22,
        28,  0,  0, 23,  9,
        29, -1, -1,  0,  8,
        26, 17, 11,  7,  0
    };

    // Setup times of machine M2; -1 means forbidden transition
    static final int[] SetupM2 = {
        0,  5, 28, -1,  2,
        -1, 0, -1,  7, 10,
        19, 22,  0, 28, 17,
        7, 26, 13,  0, -1,
        13, 17, 26, 20, 0
    };

    static final int NbTasks = 50;

    // Task type
    static final int[] TaskType = {
        3, 3, 1, 1, 1, 1, 2, 0, 0, 2,
        4, 4, 3, 3, 2, 3, 1, 4, 4, 2,
        2, 1, 4, 2, 2, 0, 3, 3, 2, 1,
        2, 1, 4, 3, 3, 0, 2, 0, 0, 3,
        2, 0, 3, 2, 2, 4, 1, 2, 4, 3
    };

    // Task duration if executed on machine M1
    static final int[] TaskDurM1 = {
        4, 17,  4,  7, 17, 14,  2, 14,  2,  8,
        11, 14,  4, 18,  3,  2,  9,  2,  9, 17,
        18, 19,  5,  8, 19, 12, 17, 11,  6,  3,
        13,  6, 19,  7,  1,  3, 13,  5,  3,  6,
        11, 16, 12, 14, 12, 17,  8,  8,  6,  6
    };

    // Task duration if executed on machine M2
    static final int[] TaskDurM2 = {
        12,  3, 12, 15,  4,  9, 14,  2,  5,  9,
        10, 14,  7,  1, 11,  3, 15, 19,  8,  2,
        18, 17, 19, 18, 15, 14,  6,  6,  1,  2,
        3, 19, 18,  2,  7, 16,  1, 18, 10, 14,
        2,  3, 14,  1,  1,  6, 19,  5, 17,  4
    };

    public static void main(String[] args) {

        try {

            IloCP cp = new IloCP();

            IloTransitionDistance setup1 = cp.transitionDistance(NbTypes);
            IloTransitionDistance setup2 = cp.transitionDistance(NbTypes);
            int i, j;
            for (i=0; i<NbTypes; ++i) {
                for (j=0; j<NbTypes; ++j) {
                    int d1 = SetupM1[NbTypes*i+j];
                    if (d1<0)
                        d1 = IloCP.IntervalMax; // Forbidden transition
                    setup1.setValue(i,j,d1);
                    int d2 = SetupM2[NbTypes*i+j];
                    if (d2<0)
                        d2 = IloCP.IntervalMax; // Forbidden transition
                    setup2.setValue(i,j,d2);
                }
            }
            int[] tp = new int[NbTasks];
            IloIntervalVar[] a  = new IloIntervalVar[NbTasks];
            IloIntervalVar[] a1 = new IloIntervalVar[NbTasks];
            IloIntervalVar[] a2 = new IloIntervalVar[NbTasks];
            IloIntExpr[] ends = new IloIntExpr[NbTasks];

            String name;
            for (i=0; i<NbTasks; ++i) {
                int type = TaskType [i];
                int d1   = TaskDurM1[i];
                int d2   = TaskDurM2[i];
                tp[i] = type;
                name = "A" + i + "_TP" + type;
                a[i] = cp.intervalVar(name);
                IloIntervalVar[] alt = new IloIntervalVar[2];
                name = "A" + i + "_M1_TP" + type;
                a1[i] = cp.intervalVar(d1, name);
                a1[i].setOptional();
                alt[0]=a1[i];
                name = "A" + i + "_M2_TP" + type;
                a2[i] = cp.intervalVar(d2, name);
                a2[i].setOptional();
                alt[1]=a2[i];
                cp.add(cp.alternative(a[i], alt));
                ends[i]=cp.endOf(a[i]);
            }

            IloIntervalSequenceVar s1 = cp.intervalSequenceVar(a1, tp);
            IloIntervalSequenceVar s2 = cp.intervalSequenceVar(a2, tp);
            cp.add(cp.noOverlap(s1, setup1, true));
            cp.add(cp.noOverlap(s2, setup2, true));
            cp.add(cp.minimize(cp.max(ends)));

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
                System.out.println("Makespan \t: " + cp.getObjValue());
            } else {
                System.out.println("No solution found.");
            }
            
        } catch (IloException e) {
            System.out.println("Error: " + e);
        }
    }
}
