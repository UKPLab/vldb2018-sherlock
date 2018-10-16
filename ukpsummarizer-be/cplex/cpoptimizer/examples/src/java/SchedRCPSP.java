// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedRCPSP.java
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

The RCPSP (Resource-Constrained Project Scheduling Problem) is a
generalization of the production-specific Job-Shop (see
SchedJobShop.java), Flow-Shop (see SchedFlowShop.java) and Open-Shop
(see SchedOpenShop.java) scheduling problems. Given:

- a set of q resources with given capacities,
- a network of precedence constraints between the activities, and
- for each activity and each resource the amount of the resource
  required by the activity over its execution,

the goal of the RCPSP is to find a schedule meeting all the
constraints whose makespan (i.e., the time at which all activities are
finished) is minimal.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;
import java.io.*;
import java.util.*;

public class SchedRCPSP {

    private static class DataReader {

        private StreamTokenizer st;

        public DataReader(String filename) throws IOException {
            FileInputStream fstream = new FileInputStream(filename);
            Reader r = new BufferedReader(new InputStreamReader(fstream));
            st = new StreamTokenizer(r);
        }

        public int next() throws IOException {
            st.nextToken();
            return (int) st.nval;
        }
    }

    static IloIntExpr[] arrayFromList(List<IloIntExpr> list) {
        return (IloIntExpr[]) list.toArray(new IloIntExpr[list.size()]);
    }

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/rcpsp_default.data";
        int failLimit = 10000;
        int nbTasks, nbResources;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        IloCP cp = new IloCP();
        DataReader data = new DataReader(filename);
        try {
            nbTasks = data.next();
            nbResources = data.next();
            List<IloIntExpr> ends = new ArrayList<IloIntExpr>();
            IloCumulFunctionExpr[] resources = new IloCumulFunctionExpr[nbResources];
            int[] capacities = new int[nbResources];

            for(int j = 0; j < nbResources; j++) {
                capacities[j] = data.next();
                resources[j] = cp.cumulFunctionExpr();
            }
            IloIntervalVar[] tasks = new IloIntervalVar[nbTasks];
            for (int i = 0; i < nbTasks; i++) {
                tasks[i] = cp.intervalVar();
            }
            for (int i = 0; i < nbTasks; i++) {
                IloIntervalVar task = tasks[i];
                int d, nbSucc;
                d = data.next();
                task.setSizeMin(d);
                task.setSizeMax(d);
                ends.add(cp.endOf(task));
                for (int j = 0; j < nbResources; j++ ) {
                    int q = data.next();
                    if (q > 0)
                        resources[j] = cp.sum(resources[j], cp.pulse(task, q));
                }
                nbSucc = data.next();
                for (int s = 0; s < nbSucc; s++ ) {
                    int succ = data.next();
                    cp.add(cp.endBeforeStart(task, tasks[succ - 1]));
                }
            }

            for (int j = 0; j < nbResources; j++) {
                cp.add(cp.le(resources[j], capacities[j]));
            }

            IloObjective objective = cp.minimize(cp.max(arrayFromList(ends)));
            cp.add(objective);

            cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
            System.out.println("Instance \t: " + filename);
            if (cp.solve()) {
                System.out.println("Makespan \t: " + cp.getObjValue());
            }
            else {
                System.out.println("No solution found.");
            }
        } catch (IloException e) {
            System.err.println("Error: " + e);
        }
    }
}
