// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/Ppp.java
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

 For a description of the problem and resolution methods:

    The Progressive Party Problem: Integer Linear Programming
    and Constraint Programming Compared

    Proceedings of the First International Conference on Principles
    and Practice of Constraint Programming table of contents

    Lecture Notes In Computer Science; Vol. 976, pages 36-52, 1995
    ISBN:3-540-60299-2

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Ppp {

    //
    // Matrix operations
    //
    
    public static IloIntVar[][] Transpose(IloIntVar[][] x) {
        int n = x[0].length;
        int m = x.length;
        IloIntVar[][] y = new IloIntVar[n][];
        for (int i = 0; i < n; i++) {
            y[i] = new IloIntVar[m];
            for (int j = 0; j < m; j++)
                y[i][j] = x[j][i];
        }
        return y;
    }

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            
            //
            // Data
            //
            int numBoats = 42;
            int[] boatSize= {
                7, 8, 12, 12, 12, 12, 12, 10, 10, 10,
                10, 10, 8, 8, 8, 12, 8, 8, 8, 8,
                8, 8, 7, 7, 7, 7, 7, 7, 6, 6,
                6, 6, 6, 6, 6, 6, 6, 6, 9, 2,
                3, 4
                };
            int[] crewSize = {
                2, 2, 2, 2, 4, 4, 4, 1, 2, 2,
                2, 3, 4, 2, 3, 6, 2, 2, 4, 2,
                4, 5, 4, 4, 2, 2, 4, 5, 2, 4,
                2, 2, 2, 2, 2, 2, 4, 5, 7, 2,
                3, 4
            };
            int numPeriods = 6;
            if (args.length > 0)
                numPeriods = Integer.parseInt(args[0]);

            //
            // Variables
            //

            // Host boat choice
            IloIntVar[] host = cp.intVarArray(numBoats,0,1, "host");
            
            // Who is where each time period (time- and boat-based views)
            IloIntVar[][] timePeriod = new IloIntVar[numPeriods][];
            for (int i = 0; i < numPeriods; i++)
                timePeriod[i] = cp.intVarArray(numBoats, 0, numBoats - 1,
                                               cp.arrayEltName("timePeriod", i));
            IloIntVar[][] visits = Transpose(timePeriod);

            //
            // Objective
            //
            IloIntVar numHosts = cp.intVar(numPeriods, numBoats);
            cp.add(cp.eq(numHosts, cp.sum(host)));
            cp.add(cp.minimize(numHosts));

            //
            // Constraints
            //
            
            // Stay in my boat (host) or only visit other boats (guest)
            for (int i = 0; i < numBoats; i++)
                cp.add(cp.eq(cp.count(visits[i], i), cp.prod(host[i], numPeriods)));

            // Capacity constraints: only hosts have capacity
            for (int p = 0; p < numPeriods; p++) {
                IloIntVar[] load = new IloIntVar[numBoats];
                for (int j = 0; j < numBoats; j++) {
                    load[j]= cp.intVar(0, boatSize[j]);
                    cp.add(cp.le(load[j], cp.prod(host[j], boatSize[j])));
                }
                cp.add(cp.pack(load, timePeriod[p], crewSize, numHosts));
            }
        
            // No two crews meet more than once
            for (int i = 0; i < numBoats; i++) {
                for (int j = i + 1; j < numBoats; j++) {
                    IloIntExpr timesMet = cp.constant(0);
                    for (int p = 0; p < numPeriods; p++)
                      timesMet = cp.sum(timesMet,
                                        cp.eq(visits[i][p], visits[j][p]));
                    cp.add(cp.le(timesMet, 1));
                }
            }
            
            // Host and guest boat constraints: given in problem spec
            cp.add(cp.eq(host[0] , 1));
            cp.add(cp.eq(host[1] , 1));
            cp.add(cp.eq(host[2] , 1));
            cp.add(cp.eq(host[39] , 0));
            cp.add(cp.eq(host[40] , 0));
            cp.add(cp.eq(host[41] , 0));
            
            //
            // Solving
            //
            boolean ok = false;
            cp.startNewSearch();
            if (cp.solve()) {
                ok = true;
                System.out.println("Solution at cost = " + (int)cp.getValue(numHosts));
                System.out.print("Hosts: ");
                System.out.print("[");
                for (int i=0; i< numBoats; i++)
                    System.out.print("["+(int)cp.getValue(host[i])+"]");
                System.out.println("]");
                for (int i = 0; i < numBoats; i++) {
                    System.out.print("Boat " + i + " (size = " + crewSize[i] + "):\t");
                    for (int j = 0; j < numPeriods; j++)
                        System.out.print((int)cp.getValue(visits[i][j]) + "\t");
                    System.out.println();
                }
                for (int p = 0; p < numPeriods; p++) {
                    System.out.println("Period " + p);
                    for (int h = 0; h < numBoats; h++) {
                        if (cp.getValue(host[h])==1) {
                            System.out.print("\tHost " + h + " : ");
                            int load = 0;
                            for (int i = 0; i < numBoats; i++) {
                                if (cp.getValue(visits[i][p]) == h) {
                                    load += crewSize[i];
                                    System.out.print(i + " (" + crewSize[i] + ") ");
                                }
                            }
                            System.out.println(" --- " + load + " / " + boatSize[h]);
                        }
                    }
                }
                System.out.println();
            }
            if (!ok)
                System.out.println("No solution");
            cp.end();
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }
}
