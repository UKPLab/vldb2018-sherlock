// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SearchPhases.java
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

How to write custom search phases
---------------------------------

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class SearchPhases {
    public static void main(String[] args) throws Exception {
        try {
            IloCP cp = new IloCP();
            IloIntVar[] x = new IloIntVar[10];
            for (int i = 0; i < 10; i++) {
                String name = "X" + Integer.toString(i);
                x[i] = cp.intVar(0, 100 - 2*(i / 2), name);
            }
            cp.add(cp.allDiff(x));

            IloIntVarChooser varChooser   = new SearchPhases.ChooseSmallestCentroid(cp);
            IloIntValueChooser valChooser = new SearchPhases.ChooseSmallestDistanceFromCentroid(cp);
            IloSearchPhase sp1 = cp.searchPhase(x, varChooser, valChooser);

            IloIntVarEval   varEval       = new SearchPhases.Centroid(cp);
            IloIntValueEval valEval       = new SearchPhases.DistanceFromCentroid(cp);
            IloSearchPhase sp2 = cp.searchPhase(x,
                                                cp.intVarChooser(cp.selectSmallest(varEval)),
                                                cp.intValueChooser(cp.selectSmallest(valEval)));

            // sp2 can have ties as two variable or values could evaluate
            // to the same values.  sp3 shows how to break these ties
            // choosing, for equivalent centroid and distance-to-centroid
            // evaluations, the lowest indexed variable in x and the 
            // lowest value.
            IloVarSelector[] selVar = new IloVarSelector[1];
            selVar[0] = cp.selectSmallest(varEval);

            IloValueSelector[] selValue = new IloValueSelector[2];
            selValue[0] = cp.selectSmallest(valEval);
            selValue[1] = cp.selectSmallest(cp.value()); // break ties on smallest

            IloSearchPhase sp3 = cp.searchPhase(x,
                                                cp.intVarChooser(selVar),
                                                cp.intValueChooser(selValue));

            cp.setParameter(IloCP.IntParam.Workers, 1);
            cp.setParameter(IloCP.IntParam.SearchType, IloCP.ParameterValues.DepthFirst);
            cp.setParameter(IloCP.IntParam.LogPeriod, 1);

            System.out.println("Choosers");
            cp.setSearchPhases(sp1);
            cp.solve();
            printIntVars(cp, x, "X");
            System.out.println("Evaluators");
            cp.setSearchPhases(sp2);
            cp.solve();
            printIntVars(cp, x, "X");
            System.out.println("Evaluators (with tie-break)");
            cp.setSearchPhases(sp3);
            cp.solve();
            printIntVars(cp, x, "X");

        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }

    // Example: variable is evaluated according to the average of
    //          the bounds of the variable
    public static double CalcCentroid(IloCPEngine engine, IloIntVar var) {
        //System.out.println("VarMin = " + engine.getMin(var) + ", VarMax = " + engine.getMax(var));
        return 0.5 * (engine.getMin(var) + engine.getMax(var));
    };
    
    // Custom variable evaluator
    static public class Centroid extends IloCustomIntVarEval {
        public Centroid(IloCP cp) throws IloException {
            super(cp);
        }
        public double eval(IloCPEngine engine, IloIntVar var) {
            return SearchPhases.CalcCentroid(engine, var);
        };
    }

    // Custom variable chooser
    static public class ChooseSmallestCentroid extends IloCustomIntVarChooser {
        public ChooseSmallestCentroid(IloCP cp) throws IloException {
            super(cp);
        };
        // Example: choose the variable with the smallest centroid
        //          Note that this class is merely for instruction
        //          In reality, it would be easier to use the above
        //          evaluator class and write:
        //            cp.selectSmallest(new Centroid(cp))
        //          rather than:
        //            ChooseSmallestCentroid(cp)
        public int choose(IloCPEngine engine, IloIntVar[] vars) {
            double best = Double.MAX_VALUE;
            int bestIndex = -1;
            int n = vars.length;
            for (int i = 0; i < n; i++) {
                if (!engine.isFixed(vars[i])) {
                    double c = SearchPhases.CalcCentroid(engine, vars[i]);
                    if (c < best) {
                        best = c;
                        bestIndex = i;
                    }
                }
            } 
            return bestIndex;
        }
    }

    // Custom value evaluator
    static public class DistanceFromCentroid extends IloCustomIntValueEval {
        public DistanceFromCentroid(IloCP cp) throws IloException {
            super(cp);
        }
        // Example: value's evaluation is its distance
        //          from the centroid
        public double eval(IloCPEngine engine, IloIntVar var, int value) {
            return Math.abs(SearchPhases.CalcCentroid(engine, var) - value);
        }
    }


    // Custom value chooser
    static public class ChooseSmallestDistanceFromCentroid extends IloCustomIntValueChooser {
        public ChooseSmallestDistanceFromCentroid(IloCP cp) throws IloException {
            super(cp);
        }
        // Example: choose the value with the smallest distance to
        //          the centroid.
        public int choose(IloCPEngine engine, IloIntVar[] vars, int i) {
            IloIntVar var = vars[i];
            double best = Double.MAX_VALUE;
            int bestValue = (int)engine.getMin(var);
            double centroid = SearchPhases.CalcCentroid(engine, var);
            java.util.Iterator it = engine.iterator(var); 
            while (it.hasNext()) {
                int curr = ((Integer)(it.next())).intValue();
                //System.out.println("    Examining value " + curr);
                double eval = Math.abs(centroid - curr);
                if (eval < best) {
                    best      = eval;
                    bestValue = curr;
                }
            }
            System.out.println("Centroid is " + centroid + ", choosing " + bestValue);
            return bestValue;
        }
    }

    public static void printIntVars(IloCP cp, IloIntVar[] x, String name) throws Exception {
        System.out.print("[");
        for (int i = 0; i < x.length; i++) {
            if (i>0)
                System.out.print(" ");
            int value = cp.getIntValue(x[i]);
            System.out.print(name + i + "[" + value + "]");
        }
        System.out.println("]");
    }
}
