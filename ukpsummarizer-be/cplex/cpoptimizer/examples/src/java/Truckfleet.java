// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/java/Truckfleet.java
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
The problem is to deliver some orders to several clients with a single truck. 
Each order consists of a given quantity of a product of a certain type (called
its color).
The truck must be configured in order to handle one, two or three different colors of products.
The cost for configuring the truck from a configuration A to a configuration B depends on A and B.
The configuration of the truck determines its capacity and its loading cost. 
A truck can only be loaded with orders for the same customer.
Both the cost (for configuring and loading the truck) and the number of travels needed to deliver all the 
orders must be minimized, the cost being the most important criterion. 

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

public class Truckfleet {
    public static int max(int[] x) {
        int m = x[0];
        for (int i = 1; i < x.length; i++) {
            if (m < x[i])
                m = x[i];
        }
        return m;
    }

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            int nbTruckConfigs = 7; // Number of possible configurations for the truck 
            int nbOrders       = 21;
            int nbCustomers    = 3; 
            int nbTrucks       = 15; // Max. number of travels of the truck
            int[]   maxTruckConfigLoad = { // Capacity of the truck depends on its config 
                11, 11, 11, 11, 10, 10, 10}; 
            int     maxLoad        = max(maxTruckConfigLoad) ; 
            int[]   customerOfOrder = {
                0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
                2};
            int[]   volumes = {
                3, 4, 3, 2, 5, 4, 11, 4, 5, 2,
                4, 7, 3, 5, 2, 5, 6, 11, 1, 6, 
                3};
            int[]   colors = {
                1, 2, 0, 1, 1, 1, 0, 0, 0, 0, 
                2, 2, 2, 0, 2, 1, 0, 2, 0, 0, 
                0};
            int[]   truckCost = { // Cost for loading a truck of a given config
                2, 2, 2, 3, 3, 3, 4}; 
            
            // Decision variables
            IloIntVar[] truckConfigs = cp.intVarArray(nbTrucks, 0, nbTruckConfigs-1); // Configuration of the truck
            IloIntVar[] where = cp.intVarArray(nbOrders, 0, nbTrucks - 1); // In which truck is an order
            IloIntVar[] load = cp.intVarArray(nbTrucks, 0, maxLoad); // Load of a truck
            IloIntVar numUsed = cp.intVar(0, nbTrucks); // Number of trucks used
            IloIntVar[] customerOfTruck = cp.intVarArray(nbTrucks, 0, nbCustomers);
            
            // Transition costs between trucks
            IloIntTupleSet costTuples = cp.intTable(3);  
            cp.addTuple(costTuples, new int[] {0,0,0});
            cp.addTuple(costTuples, new int[] {0,1,0});
            cp.addTuple(costTuples, new int[] {0,2,0});
            cp.addTuple(costTuples, new int[] {0,3,10});
            cp.addTuple(costTuples, new int[] {0,4,10});
            cp.addTuple(costTuples, new int[] {0,5,10});
            cp.addTuple(costTuples, new int[] {0,6,15});
            cp.addTuple(costTuples, new int[] {1,0,0});
            cp.addTuple(costTuples, new int[] {1,1,0});
            cp.addTuple(costTuples, new int[] {1,2,0});
            cp.addTuple(costTuples, new int[] {1,3,10});
            cp.addTuple(costTuples, new int[] {1,4,10});
            cp.addTuple(costTuples, new int[] {1,5,10});
            cp.addTuple(costTuples, new int[] {1,6,15});
            cp.addTuple(costTuples, new int[] {2,0,0});
            cp.addTuple(costTuples, new int[] {2,1,0});
            cp.addTuple(costTuples, new int[] {2,2,0});
            cp.addTuple(costTuples, new int[] {2,3,10});
            cp.addTuple(costTuples, new int[] {2,4,10});
            cp.addTuple(costTuples, new int[] {2,5,10});
            cp.addTuple(costTuples, new int[] {2,6,15});
            cp.addTuple(costTuples, new int[] {3,0,3});
            cp.addTuple(costTuples, new int[] {3,1,3});
            cp.addTuple(costTuples, new int[] {3,2,3});
            cp.addTuple(costTuples, new int[] {3,3,0});
            cp.addTuple(costTuples, new int[] {3,4,10});
            cp.addTuple(costTuples, new int[] {3,5,10});
            cp.addTuple(costTuples, new int[] {3,6,15});
            cp.addTuple(costTuples, new int[] {4,0,3});
            cp.addTuple(costTuples, new int[] {4,1,3});
            cp.addTuple(costTuples, new int[] {4,2,3});
            cp.addTuple(costTuples, new int[] {4,3,10});
            cp.addTuple(costTuples, new int[] {4,4,0});
            cp.addTuple(costTuples, new int[] {4,5,10});
            cp.addTuple(costTuples, new int[] {4,6,15});
            cp.addTuple(costTuples, new int[] {5,0,3});
            cp.addTuple(costTuples, new int[] {5,1,3});
            cp.addTuple(costTuples, new int[] {5,2,3});
            cp.addTuple(costTuples, new int[] {5,3,10});
            cp.addTuple(costTuples, new int[] {5,4,10});
            cp.addTuple(costTuples, new int[] {5,5,0});
            cp.addTuple(costTuples, new int[] {5,6,15});
            cp.addTuple(costTuples, new int[] {6,0,3});
            cp.addTuple(costTuples, new int[] {6,1,3});
            cp.addTuple(costTuples, new int[] {6,2,3});
            cp.addTuple(costTuples, new int[] {6,3,10});
            cp.addTuple(costTuples, new int[] {6,4,10});
            cp.addTuple(costTuples, new int[] {6,5,10});
            cp.addTuple(costTuples, new int[] {6,6,0});
            
            IloIntVar[] transitionCost = cp.intVarArray(nbTrucks-1, 0, 1000);
            for (int i = 1; i < nbTrucks; i++) {
                IloIntVar[] auxVars = new IloIntVar[3];
                auxVars[0]= truckConfigs[i-1];
                auxVars[1]= truckConfigs[i];
                auxVars[2]= transitionCost[i-1];
                cp.add(cp.allowedAssignments(auxVars, costTuples));
            }
            
            // Constrain the volume of the orders in each truck 
            cp.add(cp.pack(load, where, volumes, numUsed));
            for (int i = 0; i < nbTrucks; i++) {
                cp.add(cp.le(load[i], cp.element(maxTruckConfigLoad, truckConfigs[i])));
            }
            
            // Compatibility between the colors of an order and the configuration of its truck 
            int[][] allowedContainerConfigs = new int[3][];
            allowedContainerConfigs[0] = new int[] {0, 3, 4, 6};
            allowedContainerConfigs[1] = new int[] {1, 3, 5, 6};
            allowedContainerConfigs[2] = new int[] {2, 4, 5, 6};
            for (int j = 0; j < nbOrders; j++) {
                IloIntVar configOfContainer = cp.intVar(allowedContainerConfigs[colors[j]]);
                cp.add(cp.eq(configOfContainer, cp.element(truckConfigs, where[j])));
            }
            
            // Only one customer per truck 
            for (int j = 0; j < nbOrders; j++) {
                cp.add(cp.eq(cp.element(customerOfTruck, where[j]), customerOfOrder[j]));
            }
            
            // Non-used trucks are at the end
            for (int j = 1; j < nbTrucks; j++) {
                cp.add(cp.or(cp.gt(load[j-1], 0) , cp.eq(load[j], 0)));
            }
            
            // Dominance: the non used trucks keep the last used configuration
            cp.add(cp.gt(load[0], 0));
            for (int i = 1; i < nbTrucks; i++) {
                cp.add(cp.or(cp.gt(load[i], 0), cp.eq(truckConfigs[i], truckConfigs[i-1])));
            }
            
            // Dominance:  regroup deliveries with same configuration
            for (int i = nbTrucks-2; i >0; i--) {
                IloConstraint ct = cp.trueConstraint();
                for (int p = i+1; p < nbTrucks; p++) 
                    ct = cp.and(cp.neq(truckConfigs[p], truckConfigs[i-1]) , ct);
                cp.add(cp.or(cp.eq(truckConfigs[i], truckConfigs[i-1]), ct));
            }
            
            // Objective: first criterion for minimizing the cost for configuring and loading trucks 
            //            second criterion for minimizing the number of trucks
            
            IloIntExpr obj1 = cp.constant(0);
            for (int i = 0; i < nbTrucks; i++) {
                obj1 = cp.sum(obj1, cp.prod(cp.element(truckCost, truckConfigs[i]), cp.neq(load[i], 0)));
            }
            obj1 = cp.sum(obj1, cp.sum(transitionCost));
            
            IloIntExpr obj2 = numUsed;
            
            // Multicriteria lexicographic optimization
            cp.add(cp.minimize(cp.staticLex(obj1, obj2)));
            cp.setParameter(IloCP.DoubleParam.TimeLimit, 20); 
            cp.setParameter(IloCP.IntParam.LogPeriod, 50000); 
            cp.solve();
            double[] obj = cp.getObjValues();
            System.out.println("Configuration cost: " + (int) obj[0] +
                               " Number of Trucks: " + (int) obj[1]);
            for (int i = 0; i < nbTrucks; i++) {  
                if (cp.getValue(load[i]) > 0) {
                    System.out.print(
                      "Truck " +  i + ": Config=" +
                      (int) cp.getValue(truckConfigs[i]) + " Items= "
                    );
                    for (int j = 0; j < nbOrders; j++) {
                        if (cp.getValue(where[j]) == i) {
                            System.out.print("<" + j + "," + colors[j] + "," + volumes[j] + "> ");
                        }
                    }
                    System.out.println();
                }
            }
        } catch (IloException e) {
            System.out.println("Error : " + e);
            e.printStackTrace();
        } 
    }
}
