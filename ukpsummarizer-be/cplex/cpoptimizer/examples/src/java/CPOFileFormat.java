// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/CPOFileFormat.java
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

// This example illustrate importModel/dumpModel functionality on a simple map
// coloring problem. Function createModel builds a model using Concert C++ API
// and then, instead of solving it, it dumps the model into text file (the
// default file name is CPOFileFormat.cpo). After that, function solveModel
// imports the model from the file into another instance of IloCP, modifies
// domain of one of the variables (France must be blue), solves the model and
// prints the solution. 

import ilog.cp.*;
import ilog.concert.*;

// Problem Description
// -------------------
// The problem involves choosing colors for the countries on a map in such a way
// that at most four colors are used and no neighboring countries are the same
// color. In this exercise, you will find a solution for a map coloring problem
// with six countries: Belgium, Denmark, France, Germany, Luxembourg, and the
// Netherlands.

public class CPOFileFormat {
    public static String[] Names = {"blue", "white", "yellow", "green"};

    public static void createModel(String filename) {
        try {
            IloCP cp = new IloCP();
            IloIntVar Belgium = cp.intVar(0, 3, "Belgium");
            IloIntVar Denmark = cp.intVar(0, 3, "Denmark");
            IloIntVar France = cp.intVar(0, 3, "France");
            IloIntVar Germany = cp.intVar(0, 3, "Germany");
            IloIntVar Luxembourg = cp.intVar(0, 3, "Luxembourg");
            IloIntVar Netherlands = cp.intVar(0, 3, "Netherlands");
            
            cp.add(cp.neq(Belgium , France)); 
            cp.add(cp.neq(Belgium , Germany)); 
            cp.add(cp.neq(Belgium , Netherlands));
            cp.add(cp.neq(Belgium , Luxembourg));
            cp.add(cp.neq(Denmark , Germany)); 
            cp.add(cp.neq(France , Germany)); 
            cp.add(cp.neq(France , Luxembourg)); 
            cp.add(cp.neq(Germany , Luxembourg));
            cp.add(cp.neq(Germany , Netherlands));

            cp.dumpModel(filename);
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }

    public static void solveModel(String filename) {
        try {
            IloCP cp = new IloCP();
            cp.importModel(filename);
            // Force blue color (zero) for France:
            cp.getIloIntVar("France").setUB(0);
            if (cp.solve()) {    
                 System.out.println("Solution:");
                 IloIntVar[] vars = cp.getAllIloIntVars();
                 for (int i = 0; i < vars.length; i++)
                     System.out.println(vars[i].getName() + ": " + Names[(int)cp.getValue(vars[i])]);
            }
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }

    public static void main(String[] args) {
        String filename = (args.length > 0 ? args[0] : "CPOFileFormat.cpo");
        createModel(filename);
        solveModel(filename);
    }
}

