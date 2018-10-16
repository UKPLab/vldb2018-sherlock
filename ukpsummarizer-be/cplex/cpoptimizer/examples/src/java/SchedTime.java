// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedTime.java
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

This is a problem of building a house. The masonry, roofing, painting,
etc. must be scheduled.  Some tasks must necessarily take place before
others and these requirements are expressed through precedence
constraints.

Moreover, there are earliness and tardiness costs associated with some
tasks. The objective is to minimize these costs.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

public class SchedTime {

    public static IloNumExpr EarlinessCost(IloCP cp, IloIntervalVar task, int rd, double weight, int useFunction) throws IloException {
        if (useFunction != 0) {
            double[] arrX = {rd};
            double[] arrV = {-weight, 0.0};
            IloNumToNumSegmentFunction f = cp.piecewiseLinearFunction(arrX, arrV, rd, 0.0);
            return cp.startEval(task,f);
        } else {
            return cp.prod(weight, cp.max(0, cp.diff(rd, cp.startOf(task))));
        }
    }

    public static IloNumExpr TardinessCost(IloCP cp, IloIntervalVar task, int dd, double weight, int useFunction) throws IloException {
        if (useFunction != 0) {
            double[] arrX = {(double)dd};
            double[] arrV = {0.0, weight};
            IloNumToNumSegmentFunction f = cp.piecewiseLinearFunction(arrX, arrV, dd, 0.0);
            return cp.endEval(task,f);
        } else {
            return cp.prod(weight, cp.max(0, cp.diff(cp.endOf(task), dd)));
        }
    }

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
        
            /* CREATE THE INTERVAL VARIABLES. */
            IloIntervalVar masonry   = cp.intervalVar( 35, "masonry   ");
            IloIntervalVar carpentry = cp.intervalVar( 15, "carpentry ");
            IloIntervalVar plumbing  = cp.intervalVar( 40, "plumbing  ");
            IloIntervalVar ceiling   = cp.intervalVar( 15, "ceiling   ");
            IloIntervalVar roofing   = cp.intervalVar(  5, "roofing   ");
            IloIntervalVar painting  = cp.intervalVar( 10, "painting  ");
            IloIntervalVar windows   = cp.intervalVar(  5, "windows   ");
            IloIntervalVar facade    = cp.intervalVar( 10, "facade    ");
            IloIntervalVar garden    = cp.intervalVar(  5, "garden    ");
            IloIntervalVar moving    = cp.intervalVar(  5, "moving    ");

            /* ADDING PRECEDENCE CONSTRAINTS. */
            cp.add(cp.endBeforeStart(masonry,   carpentry));
            cp.add(cp.endBeforeStart(masonry,   plumbing));
            cp.add(cp.endBeforeStart(masonry,   ceiling));
            cp.add(cp.endBeforeStart(carpentry, roofing));
            cp.add(cp.endBeforeStart(ceiling,   painting));
            cp.add(cp.endBeforeStart(roofing,   windows));
            cp.add(cp.endBeforeStart(roofing,   facade));
            cp.add(cp.endBeforeStart(plumbing,  facade));
            cp.add(cp.endBeforeStart(roofing,   garden));
            cp.add(cp.endBeforeStart(plumbing,  garden));
            cp.add(cp.endBeforeStart(windows,   moving));
            cp.add(cp.endBeforeStart(facade,    moving));
            cp.add(cp.endBeforeStart(garden,    moving));
            cp.add(cp.endBeforeStart(painting,  moving));

            /* DEFINING MINIMIZATION OBJECTIVE. */
            int useFunction = 1;
            IloNumExpr cost = cp.numExpr();
            cost = cp.sum(cost, EarlinessCost(cp, masonry,   25, 200.0, useFunction));
            cost = cp.sum(cost, EarlinessCost(cp, carpentry, 75, 300.0, useFunction));
            cost = cp.sum(cost, EarlinessCost(cp, ceiling,   75, 100.0, useFunction));
            cost = cp.sum(cost, TardinessCost(cp, moving,   100, 400.0, useFunction));
            cp.add(cp.minimize(cost));

            /* SOLVING. */
            if ( cp.solve() ) {
                System.out.println("Cost Value: " + cp.getObjValue());
                System.out.println(cp.getDomain(masonry));
                System.out.println(cp.getDomain(carpentry));
                System.out.println(cp.getDomain(plumbing));
                System.out.println(cp.getDomain(ceiling));
                System.out.println(cp.getDomain(roofing));
                System.out.println(cp.getDomain(painting));
                System.out.println(cp.getDomain(windows));
                System.out.println(cp.getDomain(facade));
                System.out.println(cp.getDomain(garden));
                System.out.println(cp.getDomain(moving));
            } else {
                System.out.print("No solution found. ");
            }
        } catch (IloException e) {
                System.err.println("Error " + e );
        }
    }
}

/*
Optimal Value: 5000
masonry   [1: 20 -- 35 --> 55]
carpentry [1: 75 -- 15 --> 90]
plumbing  [1: 55 -- 40 --> 95]
ceiling   [1: 75 -- 15 --> 90]
roofing   [1: 90 -- 5 --> 95]
painting  [1: 90 -- 10 --> 100]
windows   [1: 95 -- 5 --> 100]
facade    [1: 95 -- 10 --> 105]
garden    [1: 95 -- 5 --> 100]
moving    [1: 105 -- 5 --> 110]
*/

