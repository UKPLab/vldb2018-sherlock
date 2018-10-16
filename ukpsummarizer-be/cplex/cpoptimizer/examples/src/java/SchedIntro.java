// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedIntro.java
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

This is a basic problem that involves building a house. The masonry,
roofing, painting, etc.  must be scheduled. Some tasks must
necessarily take place before others, and these requirements are
expressed through precedence constraints.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

public class SchedIntro {

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            
            /* CREATE THE INTERVAL VARIABLES. */
            IloIntervalVar masonry   = cp.intervalVar(35, "masonry   ");
            IloIntervalVar carpentry = cp.intervalVar(15, "carpentry ");
            IloIntervalVar plumbing  = cp.intervalVar(40, "plumbing  ");
            IloIntervalVar ceiling   = cp.intervalVar(15, "ceiling   ");
            IloIntervalVar roofing   = cp.intervalVar(5,  "roofing   ");
            IloIntervalVar painting  = cp.intervalVar(10, "painting  ");
            IloIntervalVar windows   = cp.intervalVar(5,  "windows   ");
            IloIntervalVar facade    = cp.intervalVar(10, "facade    ");
            IloIntervalVar garden    = cp.intervalVar(5,  "garden    ");
            IloIntervalVar moving    = cp.intervalVar(5,  "moving    ");

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

            /* EXTRACTING THE MODEL AND SOLVING. */
            if (cp.solve()) {
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
