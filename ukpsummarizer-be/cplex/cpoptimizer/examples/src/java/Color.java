// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/Color.java
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

The problem involves choosing colors for the countries on a map in 
such a way that at most four colors (blue, white, yellow, green) are 
used and no neighboring countries are the same color. In this exercise, 
you will find a solution for a map coloring problem with six countries: 
Belgium, Denmark, France, Germany, Luxembourg, and the Netherlands. 

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Color {
    public static String[] Names = {"blue", "white", "yellow", "green"}; 
    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            IloIntVar Belgium = cp.intVar(0, 3);
            IloIntVar Denmark = cp.intVar(0, 3);
            IloIntVar France = cp.intVar(0, 3);
            IloIntVar Germany = cp.intVar(0, 3);
            IloIntVar Luxembourg = cp.intVar(0, 3);
            IloIntVar Netherlands = cp.intVar(0, 3);
            
            cp.add(cp.neq(Belgium , France)); 
            cp.add(cp.neq(Belgium , Germany)); 
            cp.add(cp.neq(Belgium , Netherlands));
            cp.add(cp.neq(Belgium , Luxembourg));
            cp.add(cp.neq(Denmark , Germany)); 
            cp.add(cp.neq(France , Germany)); 
            cp.add(cp.neq(France , Luxembourg)); 
            cp.add(cp.neq(Germany , Luxembourg));
            cp.add(cp.neq(Germany , Netherlands)); 
            
            if (cp.solve())
                {    
                   System.out.println();
                   System.out.println( "Belgium:     " + Names[(int)cp.getValue(Belgium)]);
                   System.out.println( "Denmark:     " + Names[(int)cp.getValue(Denmark)]);
                   System.out.println( "France:      " + Names[(int)cp.getValue(France)]);
                   System.out.println( "Germany:     " + Names[(int)cp.getValue(Germany)]);
                   System.out.println( "Luxembourg:  " + Names[(int)cp.getValue(Luxembourg)]);
                   System.out.println( "Netherlands: " + Names[(int)cp.getValue(Netherlands)]);
                }
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }
}

