// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/java/SchedConflict.java
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

/* --------------------------------------------------------------------------

Problem Description
-------------------

This model illustrates the use of the CP Optimizer conflict refiner
on an infeasible scheduling problem. 
The problem is an instance of RCPSP (Resource-Constrained Project Scheduling 
Problem) with time windows. Given:

- a set of q resources with given capacities,
- a network of precedence constraints between the activities,
- a set of activities to be executed within a given time window and
- for each activity and each resource the amount of the resource
  required by the activity over its execution,

the goal of the problem is to find a schedule satisfying all the
constraints whose makespan (i.e., the time at which all activities are
finished) is minimal.

The instance is infeasible. The example illustrates 5 scenarios using the 
conflict refiner:

- Scenario 1: Identify a minimal conflict (no matter which one).
- Scenario 2: Identify a minimal conflict preferably using resource capacity 
              constraints.
- Scenario 3: Identify a minimal conflict preferably using precedence 
              constraints.
- Scenario 4: Find a minimal conflict partition that is, a set of disjoint 
              minimal conflicts S1,...,Sn such that when all constraints in 
              S1 U S2 U... U Sn are removed from the model, the model becomes 
              feasible.
- Scenario 5: Identify all minimal conflicts of the problem.

----------------------------------------------------------------------------- */

import ilog.concert.*;
import ilog.cp.*;
import java.io.*;
import java.util.*;

public class SchedConflict {

  public static IloCP cp;
  public static IloConstraint[] capacityCts;
  public static IloConstraint[] precedenceCts;
                   
  public SchedConflict(String fileName) throws IOException, IloException {
    cp = new IloCP();
    this.buildModel(fileName);
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

  static IloIntExpr[] arrayFromExprList(List<IloIntExpr> list) {
    return (IloIntExpr[]) list.toArray(new IloIntExpr[list.size()]);
  }
  
  static IloConstraint[] arrayFromConstraintList(List<IloConstraint> list) {
    return (IloConstraint[]) list.toArray(new IloConstraint[list.size()]);
  }
    
  //----- RCPSP Model creation ------------------------------------------------

  private void buildModel(String fileName) throws IOException {
    int nbTasks, nbResources;
    DataReader data = new DataReader(fileName);
    try {
      nbTasks = data.next();
      nbResources = data.next();
      List<IloIntExpr>    ends             = new ArrayList<IloIntExpr>();
      List<IloConstraint> capacityCtList   = new ArrayList<IloConstraint>();
      List<IloConstraint> precedenceCtList = new ArrayList<IloConstraint>();
      IloCumulFunctionExpr[] resources = new IloCumulFunctionExpr[nbResources];
      int[] capacities = new int[nbResources];
      for(int j = 0; j < nbResources; j++) {
        capacities[j] = data.next();
        resources[j] = cp.cumulFunctionExpr();
      }
      IloIntervalVar[] tasks = new IloIntervalVar[nbTasks];
      for (int i = 0; i < nbTasks; i++) {
        tasks[i] = cp.intervalVar();
        tasks[i].setName("ACT" + i);
      }
      for (int i = 0; i < nbTasks; i++) {
        IloIntervalVar task = tasks[i];
        int d, smin, emax, nbSucc;
        d    = data.next();
        smin = data.next();
        emax = data.next();
        task.setSizeMin(d);
        task.setSizeMax(d);
        task.setStartMin(smin);
        task.setEndMax(emax);
        ends.add(cp.endOf(task));
        for (int j = 0; j < nbResources; j++ ) {
          int q = data.next();
          if (q > 0) {
            resources[j] = cp.sum(resources[j], cp.pulse(task, q));
          }
        }
        nbSucc = data.next();
        for (int s = 0; s < nbSucc; s++ ) {
          int succ = data.next();
          IloConstraint pct = cp.endBeforeStart(task, tasks[succ]);
          cp.add(pct);
          precedenceCtList.add(pct);
        }
      }
      for (int j = 0; j < nbResources; j++) {
        IloConstraint cct = cp.le(resources[j], capacities[j]);
        cp.add(cct);
        capacityCtList.add(cct);
      }
      precedenceCts = arrayFromConstraintList(precedenceCtList);
      capacityCts   = arrayFromConstraintList(capacityCtList);
      IloObjective objective = cp.minimize(cp.max(arrayFromExprList(ends)));
      cp.add(objective);
    } catch (IloException e) {
        System.err.println("Error: " + e);
    }
  }
    
  //----- Basic run of conflict refiner  --------------------------------------

  private static void runBasicConflictRefiner() throws IloException {
    if (cp.refineConflict()) {
      cp.writeConflict();
    } 
  }
    
  //----- Run conflict refiner with a subset of preferred constraints  --------

  private static void runConflictRefinerWithPreferences(IloCP cp, 
                                                        IloConstraint[] preferredCts, 
                                                        IloConstraint[] otherCts) throws IloException {
    int nbPreferred = preferredCts.length;
    int nbOther     = otherCts.length;
    IloConstraint[] cts = new IloConstraint[nbPreferred+nbOther];  
    double[]      prefs = new double[nbPreferred+nbOther];
    for (int i=0; i<nbOther; ++i) {
      cts  [i] = otherCts[i];
      prefs[i] = 1.0; // Normal preference
    }
    for (int i=0; i<nbPreferred; ++i) {
      cts  [nbOther+i]= preferredCts[i];
      prefs[nbOther+i]= 2.0; // Higher preference
    }
    if (cp.refineConflict(cts, prefs)) {
      cp.writeConflict();
    } 
  }

  //----- Use conflict refiner to compute a minimal conflict partition --------

  private static void runConflictRefinerPartition(IloCP cp, IloConstraint[] cts) throws IloException {     
    int n = cts.length;
    double[] prefs = new double[n];   
    for (int i=0; i<n; ++i) {
      prefs[i]=1.0; // Normal preference
    }
    while (cp.refineConflict(cts, prefs)) {
      cp.writeConflict();
      for (int i=0; i<n; ++i) {
        if (cp.getConflict(cts[i]) == IloCP.ConflictStatus.ConflictMember) {
          prefs[i]=-1.0; // Next run will ignore constraints of the current conflict
        }
      }
    }
  }

  //----- Use conflict refiner to compute all minimal conflicts  --------------
  //----- Note: the simple search below is complete but not systematic: -------
  //----- it may find the same conflict several times.                  -------
  
  private static void runConflictRefinerAllConflicts(IloCP cp, IloConstraint[] cts) throws IloException {     
    int n = cts.length;
    double[] prefs = new double[n];   
    int i;  
    ArrayList<ArrayList<Integer>> forbiddenCts = new ArrayList<ArrayList<Integer>>();
    // Initial iteration: no forbidden constraints
    forbiddenCts.add(new ArrayList<Integer>());
    while (0<forbiddenCts.size()) {
      ArrayList<Integer> forbidden = (ArrayList<Integer>)forbiddenCts.remove(0);
      for (i=0; i<n; ++i) {
        prefs[i]=1.0; // Initialize to normal preference
      }
      for (i=0; i<forbidden.size(); ++i) {
        prefs[(Integer)forbidden.get(i)]=-1.0; // Remove constraint
      }
      if (cp.refineConflict(cts, prefs)) {
        cp.writeConflict();
        // Next iterations
        for (i=0; i<n; ++i) {
          if (cp.getConflict(cts[i]) == IloCP.ConflictStatus.ConflictMember) {
            ArrayList<Integer> newforbidden = new ArrayList<Integer>();
            newforbidden.add(i);
            newforbidden.addAll(forbidden);           
            forbiddenCts.add(newforbidden);
          }
        }
      }
    }
  }
                                       
                                       
  public static void main(String[] args) throws IOException {
    String fileName = "../../../examples/data/sched_conflict.data";
    int failLimit = 10000;
    if (args.length > 1)
      fileName = args[1];
    if (args.length > 2)
      failLimit = Integer.parseInt(args[2]);
    try {
      SchedConflict problem = new SchedConflict(fileName);
      int nbCapacityCts   = capacityCts.length;
      int nbPrecedenceCts = precedenceCts.length;
      IloConstraint[] allCts = new IloConstraint[nbCapacityCts+nbPrecedenceCts];  
      for (int i=0; i<nbCapacityCts; ++i) {
        allCts[i] = capacityCts[i];
      }
      for (int i=0; i<nbPrecedenceCts; ++i) {
        allCts[nbCapacityCts+i] = precedenceCts[i];
      }
      cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
      cp.setParameter(IloCP.IntParam.CumulFunctionInferenceLevel, IloCP.ParameterValues.Extended);
      cp.setParameter(IloCP.IntParam.ConflictRefinerOnVariables, IloCP.ParameterValues.On);
      System.out.println("Instance \t: " + fileName);
      if (cp.solve()) {
        // A solution was found
        System.out.println("Solution found with makespan : " + cp.getObjValue());
      } else {
        int status = cp.getInfo(IloCP.IntInfo.FailStatus);
        if (status != IloCP.ParameterValues.SearchHasFailedNormally.getValue()) {
          // No solution found but problem was not proved infeasible
          System.out.println("No solution found but problem was not proved infeasible.");
        } else {
          // Run conflict refiner only if problem was proved infeasible
          System.out.println("Infeasible problem, running conflict refiner ...");
          System.out.println("SCENARIO 1: Basic conflict refiner:");
          runBasicConflictRefiner();    
          cp.setParameter(IloCP.IntParam.LogVerbosity, IloCP.ParameterValues.Quiet);
          System.out.println("SCENARIO 2: Conflict refiner with preference on resource capacity constraints:");
          runConflictRefinerWithPreferences(cp, capacityCts, precedenceCts);
          System.out.println("SCENARIO 3: Conflict refiner with preference on precedence constraints:");
          runConflictRefinerWithPreferences(cp, precedenceCts, capacityCts);
          System.out.println("SCENARIO 4: Conflict partition:"); 
          runConflictRefinerPartition(cp, allCts);
          System.out.println("SCENARIO 5: All conflicts:");
          runConflictRefinerAllConflicts(cp, allCts);
        }
      }
    } catch (IloException e) {
      System.err.println("Error: " + e);
    }
  }
}
