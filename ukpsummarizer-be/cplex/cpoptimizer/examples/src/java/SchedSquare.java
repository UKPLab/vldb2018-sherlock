// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedSquare.java
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

The aim of the square example is to place a set of small squares of
different sizes into a large square.

------------------------------------------------------------ */


import ilog.concert.*;
import ilog.cp.*;

import java.util.List;
import java.util.ArrayList;


public class SchedSquare {

    public static void main(String[] args) {

        try {
            IloCP cp = new IloCP();

            int sizeSquare = 112;
            int nbSquares = 21;
            int[] size = {50, 42, 37, 35, 33, 29, 27, 25, 24, 19, 18, 17, 16, 15, 11, 9, 8, 7, 6, 4, 2};
            IloIntervalVar[] x = new IloIntervalVar[nbSquares];
            IloIntervalVar[] y = new IloIntervalVar[nbSquares];
            IloCumulFunctionExpr rx = cp.cumulFunctionExpr();
            IloCumulFunctionExpr ry = cp.cumulFunctionExpr();

            for (int i = 0; i < nbSquares; ++i) {
                x[i] = cp.intervalVar(size[i], "X" + i);
                x[i].setEndMax(sizeSquare);
                y[i] = cp.intervalVar(size[i], "Y" + i);
                y[i].setEndMax(sizeSquare);
                rx = cp.sum(rx, cp.pulse(x[i], size[i]));
                ry = cp.sum(ry, cp.pulse(y[i], size[i]));

                for (int j = 0; j < i; ++j) {
                    cp.add(cp.or(cp.le(cp.endOf(x[i]), cp.startOf(x[j])),
                            cp.or(cp.le(cp.endOf(x[j]), cp.startOf(x[i])),
                                    cp.or(cp.le(cp.endOf(y[i]), cp.startOf(y[j])),
                                            cp.le(cp.endOf(y[j]), cp.startOf(y[i]))))));
                }
            }
            cp.add(cp.alwaysIn(rx, 0, sizeSquare, sizeSquare, sizeSquare));
            cp.add(cp.alwaysIn(ry, 0, sizeSquare, sizeSquare, sizeSquare));

            IloSearchPhase[] phases = new IloSearchPhase[2];
            phases[0] = cp.searchPhase(x);
            phases[1] = cp.searchPhase(y);

            cp.setSearchPhases(phases);
            if (cp.solve()) {
                for (int i = 0; i < nbSquares; ++i) {
                    System.out.println("Square " + i + ": ["
                            + cp.getStart(x[i]) + "," + cp.getEnd(x[i])
                            + "] x ["
                            + cp.getStart(y[i]) + "," + cp.getEnd(y[i])
                            + "]");
                }
            }

        } catch (IloException ex) {
            System.out.println("Caught: " + ex);
        }
    }
}
//$end:ALL
