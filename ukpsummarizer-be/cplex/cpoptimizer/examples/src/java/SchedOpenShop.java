// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedOpenShop.java
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

This problem can be described as follows: a finite set of operations
has to be processed on a given set of machines. Each operation has a
specific processing time during which it may not be interrupted.
Operations are grouped in jobs, so that each operation belongs to
exactly one job. Furthermore, each operation requires exactly one
machine for processing.

The objective of the problem is to schedule all operations, i.e., to
determine their start time, so as to minimize the maximum completion
time (makespan) given the additional constraints that: operations
which belong to the same job and operations which use the same machine
cannot be processed simultaneously.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.*;

public class SchedOpenShop {

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

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/openshop_default.data";
        int failLimit = 10000;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        try {
            IloCP cp = new IloCP();

            DataReader data = new DataReader(filename);
            int nbJobs = data.next();
            int nbMachines = data.next();

            IntervalVarList[] jobs = new IntervalVarList[nbJobs];
            for (int i = 0; i < nbJobs; i++)
                jobs[i] = new IntervalVarList();
            IntervalVarList[] machines = new IntervalVarList[nbMachines];
            for (int j = 0; j < nbMachines; j++)
                machines[j] = new IntervalVarList();

            IloIntExpr[] ends = new IloIntExpr[nbJobs * nbMachines];
            for (int i = 0; i < nbJobs; i++) {
                for (int j = 0; j < nbMachines; j++) {
                    int pt = data.next();
                    IloIntervalVar ti = cp.intervalVar(pt);
                    jobs[i].add(ti);
                    machines[j].add(ti);
                    ends[i * nbMachines + j] = cp.endOf(ti);
                }
            }

            for (int i = 0; i < nbJobs; i++)
                cp.add(cp.noOverlap(jobs[i].toArray()));

            for (int j = 0; j < nbMachines; j++)
                cp.add(cp.noOverlap(machines[j].toArray()));

            IloObjective objective = cp.minimize(cp.max(ends));
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
