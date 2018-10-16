// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedGoalProg.java
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

This example solves a multi-machine assignment and scheduling problem
([1]), where jobs, with release dates and deadlines, have to be
processed on parallel unrelated machines where processing times depend
on machine assignment. Given a job/machine assignment cost matrix, the
objective is to minimize the total cost while keeping all machine
schedules feasible.

In the instances used in [1], two types of machines can be
distinguished: expensive machines (whose cost for a job equals 1000)
and regular machines (whose cost is much lower, typically less than
50). This example implements a two-steps approach:

- In a first step, the problem is solved by taking all the constraints
into account but neglecting the cost of regular machines and focusing
on minimizing the cost of allocating jobs to expensive machines. 

- A solution to the above problem is also a solution to the main
problem. In a second step, this solution is used as starting point
for solving the main problem.

This two-steps approach permits to focus in the first phase on the
most important part of the cost (the allocation of job to expensive
machines) and, once a good (or optimal) solution has been found, to
use this solution as a starting point for minimizing the other part of
the cost. This approach can be used in most problems where the
components of the objective function are lexically ordered.

By default, the program executes the two-steps approach. If the second
argument (useGoalProgramming) is set to 0, the program executes a
single-step approach. With the same global limit, results with the
two-steps approach are better than the ones of a single-step approach.

[1] R. Sadykov and L. Wolsey. Integer programming and constraint
programming in solving a multi-machine assignment scheduling problem
with deadlines and release dates.  INFORMS Journal on Computing. 2006.

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;

public class SchedGoalProg {
  
  private int nbMachines, nbJobs;
  private IloIntervalVar[][] machines;
  private int[][] costs;
  private IloCP cp;  
  
  public SchedGoalProg(String fileName) throws IOException, IloException {
    cp = new IloCP();
    createModel(fileName);
  }
  
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
  
  private void createModel(String dataFile) throws IOException, IloException {
    
    DataReader data = new DataReader(dataFile);
    nbJobs = data.next();
    nbMachines = data.next();
      
    int[] rel = new int[nbJobs];
    int[] due = new int[nbJobs];
      
    costs = new int[nbMachines][nbJobs];
    int[][] dur = new int[nbMachines][nbJobs];
      
    for (int i = 0; i < nbJobs; i++)
      rel[i] = data.next();

    for (int i = 0; i < nbJobs; i++)
      due[i] = data.next();
      
    for (int j = 0; j < nbMachines; j++) 
      for (int i = 0; i < nbJobs; i++)
        costs[j][i] = data.next();
      
    for (int j = 0; j < nbMachines; j++)
      for (int i = 0; i < nbJobs; i++)
        dur[j][i] = data.next();
      
    machines = new IloIntervalVar[nbMachines][nbJobs];
    for(int i = 0; i < nbJobs; i++) {
      IloIntervalVar job = cp.intervalVar();
      job.setStartMin(rel[i]);
      job.setEndMax(due[i]);
      job.setName("Op" + i);
      IloIntervalVar[] jobm = new IloIntervalVar[nbMachines];
      for (int j = 0; j < nbMachines; j++) {
        jobm[j] = cp.intervalVar(dur[j][i]);
        jobm[j].setName("Alt" + i + "_" + j + "_C" + costs[j][i]);
        jobm[j].setOptional();
        machines[j][i] = jobm[j];
      }
      cp.add(cp.alternative(job, jobm));
    }
    for(int j = 0; j < nbMachines; j++)
      cp.add(cp.noOverlap(machines[j]));
  }
  
  public double solveGoalProgramming(int branchLimit) throws IloException {
    
    // Objective 1
    System.out.println();
    System.out.println(" ! ----------------------------------------------------------------------------");
    System.out.println(" ! STEP 1: Minimizing usage of expensive resources");
    System.out.println(" ! ----------------------------------------------------------------------------");
    IloIntExpr costExpr = cp.intExpr();
    for (int i = 0; i < nbJobs; i++) {
      for (int j = 0; j < nbMachines; j++) {
        if (costs[j][i] == 1000) {
          costExpr = cp.sum(costExpr, cp.presenceOf(machines[j][i]));
        }
      }
    }
    IloObjective obj1 = cp.minimize(costExpr);
    cp.add(obj1);
    
    cp.setParameter(IloCP.IntParam.LogPeriod, 10000000);
    cp.setParameter(IloCP.IntParam.BranchLimit, branchLimit / 2);
    cp.setParameter(IloCP.IntParam.NoOverlapInferenceLevel, IloCP.ParameterValues.Extended);
    cp.solve();
    int cost1 = (int)cp.getObjValue();
    int branchesLeft = branchLimit - cp.getInfo(IloCP.IntInfo.NumberOfBranches);
    
    IloSolution startSol = cp.solution();
    for (int i = 0; i < nbJobs; i++) {
      for (int j = 0; j < nbMachines; j++) {
        if (cp.isPresent(machines[j][i])) {
          startSol.setPresent(machines[j][i]);
          startSol.setStart(machines[j][i], cp.getStart(machines[j][i]));
        }
      }
    }
    
    // Objective 2
    System.out.println();
    System.out.println(" ! ----------------------------------------------------------------------------");
    System.out.println(" ! STEP 2: Minimizing total cost");
    System.out.println(" ! ----------------------------------------------------------------------------");
    cp.remove(obj1);
    cp.add(cp.le(costExpr, cost1));
    IloIntExpr costExpr2 = cp.intExpr();
    for (int i = 0; i < nbJobs; i++) {
      for (int j = 0; j < nbMachines; j++) {
        if (costs[j][i] < 1000)
          costExpr2 = cp.sum(costExpr2, cp.prod(costs[j][i], cp.presenceOf(machines[j][i])));
      }
    }
    IloObjective obj2 = cp.minimize(costExpr2);
    cp.add(obj2);
    cp.setParameter(IloCP.IntParam.BranchLimit, branchesLeft);
    cp.setStartingPoint(startSol);
    cp.solve();
    double cost2 = cp.getObjValue();
    return 1000 * cost1 + cost2;
  }  
  
  public double solveBasic(int branchLimit) throws IloException {

    // Objective
    System.out.println();
    System.out.println(" ! ----------------------------------------------------------------------------");
    System.out.println(" ! Minimizing total cost");
    System.out.println(" ! ----------------------------------------------------------------------------");
    IloIntExpr costExpr = cp.intExpr();
    int nbMachines = machines.length;
    int nbJobs = machines[0].length;
    for (int i = 0; i < nbJobs; i++) {
      for (int j = 0; j < nbMachines; j++) {
        costExpr = cp.sum(costExpr, cp.prod(costs[j][i], cp.presenceOf(machines[j][i])));
      }
    }
    IloObjective obj = cp.minimize(costExpr);
    cp.add(obj);
    
    cp.setParameter(IloCP.IntParam.LogPeriod, 10000000);
    cp.setParameter(IloCP.IntParam.BranchLimit, branchLimit);
    cp.setParameter(IloCP.IntParam.NoOverlapInferenceLevel, IloCP.ParameterValues.Extended);
    cp.solve();
    double cost = cp.getObjValue();
    return cost;    
  }
  
  public static void main(String args[]) throws IOException {
    
    String fileName = "../../../examples/data/goalprog_8_40_0.6_3.data";
    boolean useGoalProgramming = true;
    int branchLimit = 300000;
    
    if (args.length > 0)
      fileName = args[0];
    if (args.length > 1)
      useGoalProgramming = Boolean.getBoolean(args[1]);
    if (args.length > 3)
      branchLimit = Integer.parseInt(args[2]);
    
    try {
      double cost = 0;
      System.out.println("Data file: " + fileName);
      SchedGoalProg gp = new SchedGoalProg(fileName);
      if (useGoalProgramming) {
        System.out.println("Solving in two steps using goal programming ...");
        cost = gp.solveGoalProgramming(branchLimit);
      }
      else {
        System.out.println("Solving in a single step  ...");
        cost = gp.solveBasic(branchLimit);
      }
    
      System.out.println();
      System.out.println(" ! ----------------------------------------------------------------------------");
      System.out.println(" ! Cost= " + cost );
      System.out.println(" ! ----------------------------------------------------------------------------");
    } 
    catch (IloException e) {
      System.out.println("ERROR:" + e);
    }
  }
}
