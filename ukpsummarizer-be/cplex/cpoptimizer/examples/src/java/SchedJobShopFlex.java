// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedJobShopFlex.java
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

This problem is an extension of the classical Job-Shop Scheduling
problem (see SchedJobShop.java) which allows an operation to be
processed by any machine from a given set. The operation processing
time depends on the allocated machine. The problem is to assign each
operation to a machine and to order the operations on the machines
such that the maximal completion time (makespan) of all operations is
minimized.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.List;
import java.util.ArrayList;

public class SchedJobShopFlex {

    static class DataReader {

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
        return (IloIntExpr[])list.toArray(new IloIntExpr[list.size()]);
    }

    static class IntervalVarList extends ArrayList<IloIntervalVar> {
        public IloIntervalVar[] toArray() {
            return (IloIntervalVar[]) this.toArray(new IloIntervalVar[this.size()]);
        }
    }

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/jobshopflex_default.data";
        int failLimit = 10000;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        IloCP cp = new IloCP();
        DataReader data = new DataReader(filename);

        try {
            int nbJobs = data.next();
            int nbMachines = data.next();

            IntervalVarList[] machines = new IntervalVarList[nbMachines];
            for (int j = 0; j < nbMachines; j++)
                machines[j] = new IntervalVarList();
            List<IloIntExpr> ends = new ArrayList<IloIntExpr>();

            for (int i = 0; i < nbJobs; i++) {
                int nbOperations = data.next();
                IloIntervalVar prec = cp.intervalVar();
                for (int j = 0; j < nbOperations; j++) {
                    int nbOpMachines = data.next();
                    IloIntervalVar master = cp.intervalVar();
                    IntervalVarList members = new IntervalVarList();
                    for (int k = 0; k < nbOpMachines; k++) {
                        int m = data.next();
                        int d = data.next();
                        IloIntervalVar member = cp.intervalVar(d);
                        member.setOptional();
                        members.add(member);
                        machines[m - 1].add(member);
                    }
                    cp.add(cp.alternative(master, members.toArray()));
                    if (j > 0)
                        cp.add(cp.endBeforeStart(prec, master));
                    prec = master;
                }
                ends.add(cp.endOf(prec));
            }

            for (int j = 0; j < nbMachines; j++) {
                cp.add(cp.noOverlap(machines[j].toArray()));
            }

            IloObjective objective = cp.minimize(cp.max(arrayFromList(ends)));
            cp.add(objective);

            cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
            System.out.println("Instance \t: " + filename);
            if (cp.solve()) {
                System.out.println("Makespan \t: " + cp.getObjValue());
            } else {
                System.out.println("No solution found.");
            }
        } catch (IloException e) {
            System.err.println("Error: " + e);
        }
    }
}
