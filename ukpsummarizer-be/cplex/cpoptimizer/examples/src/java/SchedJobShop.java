// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedJobShop.java
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

In the classical Job-Shop Scheduling problem a finite set of jobs is
processed on a finite set of machines. Each job is characterized by a
fixed order of operations, each of which is to be processed on a
specific machine for a specified duration.  Each machine can process
at most one operation at a time and once an operation initiates
processing on a given machine it must complete processing
uninterrupted.  The objective of the problem is to find a schedule
that minimizes the makespan of the schedule.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.ArrayList;
import java.util.List;

public class SchedJobShop {

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

        String filename = "../../../examples/data/jobshop_default.data";
        int nbJobs, nbMachines;

        if (args.length > 0)
            filename = args[0];

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
                    int m, d;
                    m = data.next();
                    d = data.next();
                    IloIntervalVar ti = cp.intervalVar(d);
                    machines[m].add(ti);
                    if (j > 0) {
                        cp.add(cp.endBeforeStart(prec, ti));
                    }
                    prec = ti;
                }
                ends.add(cp.endOf(prec));
            }

            for (int j = 0; j < nbMachines; j++)
                cp.add(cp.noOverlap(machines[j].toArray()));

            IloObjective objective = cp.minimize(cp.max(arrayFromList(ends)));
            cp.add(objective);

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
