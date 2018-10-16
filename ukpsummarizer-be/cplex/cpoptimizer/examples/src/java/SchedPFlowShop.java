// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedPFlowShop.java
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

The general Flow-Shop scheduling problem is a production problem where
a set of n jobs have to be processed with identical flow pattern on m
machines (see SchedFlowShop.java). In permutation Flow-Shops the
sequence of jobs is the same on all machines.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.List;
import java.util.ArrayList;

public class SchedPFlowShop {

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

    static class IntervalVarList extends ArrayList<IloIntervalVar> {
        public IloIntervalVar[] toArray() {
            return (IloIntervalVar[]) this.toArray(new IloIntervalVar[this.size()]);
        }
    }

    static IloIntExpr[] arrayFromList(List<IloIntExpr> list) {
        return (IloIntExpr[]) list.toArray(new IloIntExpr[list.size()]);
    }

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/flowshop_default.data";
        int failLimit = 10000;
        int nbJobs, nbMachines;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        IloCP cp = new IloCP();
        DataReader data = new DataReader(filename);
        try {
            nbJobs = data.next();
            nbMachines = data.next();
            List<IloIntExpr> ends = new ArrayList<IloIntExpr>();
            IntervalVarList[] machines = new IntervalVarList[nbMachines];
            for (int j = 0; j < nbMachines; j++)
                machines[j] = new IntervalVarList();
            for (int i = 0; i < nbJobs; i++) {
                IloIntervalVar prec = cp.intervalVar();
                for (int j = 0; j < nbMachines; j++) {
                    int d = data.next();
                    IloIntervalVar ti = cp.intervalVar(d);
                    machines[j].add(ti);
                    if (j > 0) {
                        cp.add(cp.endBeforeStart(prec, ti));
                    }
                    prec = ti;
                }
                ends.add(cp.endOf(prec));
            }

            IloIntervalSequenceVar[] seqs = new IloIntervalSequenceVar[nbMachines];
            for (int j = 0; j < nbMachines; j++) {
                seqs[j] = cp.intervalSequenceVar(machines[j].toArray());
                cp.add(cp.noOverlap(seqs[j]));
                if (0<j) {
                    cp.add(cp.sameSequence(seqs[0],seqs[j]));
                }
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
