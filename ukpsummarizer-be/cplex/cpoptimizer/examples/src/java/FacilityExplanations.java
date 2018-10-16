// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/FacilityExplanations.java
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

 A company has 10 stores.  Each store must be supplied by one warehouse. The 
 company has five possible locations where it has property and can build a 
 supplier warehouse: Bonn, Bordeaux, London, Paris, and Rome. The warehouse 
 locations have different capacities. A warehouse built in Bordeaux or Rome 
 could supply only one store. A warehouse built in London could supply two 
 stores; a warehouse built in Bonn could supply three stores; and a warehouse 
 built in Paris could supply four stores. 

 The supply costs vary for each store, depending on which warehouse is the 
 supplier. For example, a store that is located in Paris would have low supply 
 costs if it were supplied by a warehouse also in Paris.  That same store would 
 have much higher supply costs if it were supplied by the other warehouses.

 The cost of building a warehouse varies depending on warehouse location.

 The problem is to find the most cost-effective solution to this problem, while
 making sure that each store is supplied by a warehouse.

 ------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

import java.io.*;

public class FacilityExplanations {

  public static String[] Names = { "blue", "white", "yellow", "green" };

  private static class DataReader {

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

  public static void main(String[] args) throws IOException {
    String filename;
    if (args.length > 0)
      filename = args[0];
    else
      filename = "../../../examples/data/facility.data";
    try {
      IloCP cp = new IloCP();
      int i, j;

      DataReader data = new DataReader(filename);

      int nbLocations = data.next();
      int nbStores = data.next();
      int[] capacity = new int[nbLocations];
      int[] fixedCost = new int[nbLocations];
      int[][] cost = new int[nbStores][];

      for (i = 0; i < nbStores; i++)
        cost[i] = new int[nbLocations];

      for (j = 0; j < nbLocations; j++)
        capacity[j] = data.next();

      for (j = 0; j < nbLocations; j++)
        fixedCost[j] = data.next();

      for (i = 0; i < nbStores; i++)
        for (j = 0; j < nbLocations; j++)
          cost[i][j] = data.next();

      IloIntVar[] supplier = cp.intVarArray(nbStores, 0, nbLocations - 1);
      IloIntVar[] open = cp.intVarArray(nbLocations, 0, 1);
      
      for (i = 0; i < nbStores; i++)
        cp.add(cp.eq(cp.element(open, supplier[i]), 1));
      
      for (j = 0; j < nbLocations; j++)
        cp.add(cp.le(cp.count(supplier, j), capacity[j]));

      IloIntExpr obj = cp.scalProd(open, fixedCost);
      for (i = 0; i < nbStores; i++)
        obj = cp.sum(obj, cp.element(cost[i], supplier[i]));

      cp.add(cp.minimize(obj));
      
      cp.setParameter(IloCP.IntParam.Workers, 1);
      cp.setParameter(IloCP.IntParam.SearchType, IloCP.ParameterValues.DepthFirst);
      cp.setParameter(IloCP.IntParam.LogPeriod, 1);
      cp.setParameter(IloCP.IntParam.LogSearchTags, IloCP.ParameterValues.On);
      cp.solve();

      cp.clearExplanations();
      cp.explainFailure(15);
      cp.explainFailure(20);
      int failureArray[] = {3, 10, 11, 12};
      cp.explainFailure(failureArray);
      cp.solve();

      cp.clearExplanations();
      cp.explainFailure(1);
      cp.solve();

      System.out.println();
      System.out.println("Optimal value: " + (int) cp.getValue(obj));
      for (j = 0; j < nbLocations; j++) {
        if (cp.getValue(open[j]) == 1) {
          System.out.print("Facility " + j
              + " is open, it serves stores ");
          for (i = 0; i < nbStores; i++) {
            if (cp.getValue(supplier[i]) == j)
              System.out.print(i + " ");
          }
          System.out.println();
        }
      }
    } catch (IloException e) {
      System.err.println("Error " + e);
    } 
  }
}
