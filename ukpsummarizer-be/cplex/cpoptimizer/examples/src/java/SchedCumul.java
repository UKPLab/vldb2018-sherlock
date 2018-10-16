// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedCumul.java
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2017. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// -----------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses in different locations. The
masonry, roofing, painting, etc. must be scheduled. Some tasks must
necessarily take place before others and these requirements are
expressed through precedence constraints.

There are three workers, and each task requires a worker.  There is
also a cash budget which starts with a given balance.  Each task costs
a given amount of cash per day which must be available at the start of
the task.  A cash payment is received periodically.  The objective is
to minimize the overall completion date.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;
import java.util.List;
import java.util.ArrayList;

public class SchedCumul {
    static final int nbWorkers = 3;
    static final int nbTasks   = 10;

    static final int masonry   = 0;
    static final int carpentry = 1;
    static final int plumbing  = 2;
    static final int ceiling   = 3;
    static final int roofing   = 4;
    static final int painting  = 5;
    static final int windows   = 6;
    static final int facade    = 7;
    static final int garden    = 8;
    static final int moving    = 9;

    static final String[] taskNames = {
        "masonry  ",
        "carpentry",
        "plumbing ",
        "ceiling  ",
        "roofing  ",
        "painting ",
        "windows  ",
        "facade   ",
        "garden   ",
        "moving   "
    };

    static final int[] taskDurations = {
        35,
        15,
        40,
        15,
        05,
        10,
        05,
        10,
        05,
        05,
    };

    static IloCP cp = new IloCP();

    static IloCumulFunctionExpr workersUsage;
    static IloCumulFunctionExpr cash;

    public static void makeHouse(int id,
                                 int rd,
                                 List<IloIntExpr> ends,
                                 List<IloIntervalVar> allTasks)
        throws IloException {
        /* CREATE THE INTERVAL VARIABLES. */
        String name;
        IloIntervalVar[] tasks = new IloIntervalVar[nbTasks];
        for (int i = 0; i < nbTasks; ++i) {
            name = "H" + id + "-" + taskNames[i];
            IloIntervalVar task = cp.intervalVar(taskDurations[i], name);
            tasks[i] = task;
            allTasks.add(task);
            workersUsage = cp.sum(workersUsage, cp.pulse(task,1));
            cash = cp.diff(cash, cp.stepAtStart(task, 200 * taskDurations[i]));
        }

        /* ADDING PRECEDENCE CONSTRAINTS. */
        tasks[masonry].setStartMin(rd);
        cp.add(cp.endBeforeStart(tasks[masonry],   tasks[carpentry]));
        cp.add(cp.endBeforeStart(tasks[masonry],   tasks[plumbing]));
        cp.add(cp.endBeforeStart(tasks[masonry],   tasks[ceiling]));
        cp.add(cp.endBeforeStart(tasks[carpentry], tasks[roofing]));
        cp.add(cp.endBeforeStart(tasks[ceiling],   tasks[painting]));
        cp.add(cp.endBeforeStart(tasks[roofing],   tasks[windows]));
        cp.add(cp.endBeforeStart(tasks[roofing],   tasks[facade]));
        cp.add(cp.endBeforeStart(tasks[plumbing],  tasks[facade]));
        cp.add(cp.endBeforeStart(tasks[roofing],   tasks[garden]));
        cp.add(cp.endBeforeStart(tasks[plumbing],  tasks[garden]));
        cp.add(cp.endBeforeStart(tasks[windows],   tasks[moving]));
        cp.add(cp.endBeforeStart(tasks[facade],    tasks[moving]));
        cp.add(cp.endBeforeStart(tasks[garden],    tasks[moving]));
        cp.add(cp.endBeforeStart(tasks[painting],  tasks[moving]));

        /* DEFINING MINIMIZATION OBJECTIVE. */
        ends.add(cp.endOf(tasks[moving]));
    }

    static IloIntExpr[] arrayFromList(List<IloIntExpr> list) {
        return (IloIntExpr[])list.toArray(new IloIntExpr[list.size()]);
    }

    public static void main(String[] args) {

        try {

            workersUsage = cp.cumulFunctionExpr();
            cash = cp.cumulFunctionExpr();
            List<IloIntExpr> ends = new ArrayList<IloIntExpr>();
            List<IloIntervalVar> allTasks = new ArrayList<IloIntervalVar>();

            /* CASH PAYMENTS. */
            for (int p=0; p<5; ++p)
                cash = cp.sum(cash, cp.step(60*p, 30000));

            makeHouse(0,  31, ends, allTasks);
            makeHouse(1,   0, ends, allTasks);
            makeHouse(2,  90, ends, allTasks);
            makeHouse(3, 120, ends, allTasks);
            makeHouse(4,  90, ends, allTasks);

            cp.add(cp.le(0, cash));

            cp.add(cp.le(workersUsage, nbWorkers));

            cp.add(cp.minimize(cp.max(arrayFromList(ends))));

            /* EXTRACTING THE MODEL AND SOLVING. */
            cp.setParameter(IloCP.IntParam.FailLimit, 10000);
            if (cp.solve()) {
                System.out.println("Solution with objective " + cp.getObjValue());
                for (int i = 0; i < allTasks.size(); i++) {
                    System.out.println(cp.getDomain(allTasks.get(i)));
                }
                int segs = cp.getNumberOfSegments(cash);
                for (int i = 0; i < segs; i++) {
                  System.out.println(
                    "Cash is " + cp.getSegmentValue(cash, i) +
                    " from " + cp.getSegmentStart(cash, i) +
                    " to " + (cp.getSegmentEnd(cash, i) - 1)
                  );
                }
            } else {
                System.out.println("No Solution found.");
            }
        } catch (IloException e) {
            System.err.println("Error " + e );
        }
    }
}
