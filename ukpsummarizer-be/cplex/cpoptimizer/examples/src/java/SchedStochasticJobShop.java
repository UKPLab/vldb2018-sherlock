// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/SchedStochasticJobShop.java
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

The Stochastic Job-Shop Scheduling problem is a variant of the classical 
deterministic Job-Shop Scheduling problem (see SchedJobShop.java) where 
the duration of operations is uncertain. 

Scenarios
---------

This example illustrates how to solve a Stochastic Job-Shop Scheduling 
problem using a scenario-based approach. A set of n scenarios is created, 
each scenario represents a particular realization of the durations of 
the operations. 

The instance is a small 6x6 Job-Shop Scheduling problem with 20 scenarios.
In the example we suppose the scenarios are given as input. In practical 
problems, these scenarios may be given by a selection of representative 
past execution of the system or they may be computed by sampling the 
probability distributions of operations duration.

For example the different scenarios give the following durations 
for the 6 operations of the first job:

JOB #1
                 Machine:  M5 -> M1 -> M4 -> M3 -> M0 -> M2
Duration in scenario #00: 218  284  321  201  101  199
Duration in scenario #01: 259  313  311  191   93  211
... 
Duration in scenario #19: 501  309  301  203   95  213


The objective is to find a robust sequencing of operations on machines so
as to minimize the expected makespan across all scenarios.

The problem can be seen as a particular case of Two-Stage Stochastic 
Programming problem where first stage decision variables are the sequences 
of operations on machines and second stage decision variables are the 
actual start/end time of operations that will depend on the actual duration
of operations at execution time.
 
The model proposed here generalizes to any type of stochastic scheduling 
problem where first stage decision variables involve creating robust 
sequences of operations on a machine.

Model
-----

Each scenario is modeled as a particular deterministic Job-Shop Scheduling 
problem.

Let makespan[k] denote the makespan of scenario k and sequence[k][j] denote 
the sequence variable representing the sequencing of operations on machine 
j in scenario k.

A set of 'sameSequence' constraints are posted across all scenarios k to 
state that for a machine j, the sequence of operations should be the same 
for all scenarios. The sequence variable of the first scenario (sequence[0][j]) 
is used as reference:
forall j, forall 0<k: sameSequence(sequence[0][j],sequence[k][j])

The global objective function is the average makespan over the different 
scenarios:
objective: (sum(k) makespan[k]) / nbScenarios

Solution quality
----------------

Solution with expected makespan 4648.4 is optimal. As the sample is small,
this solution can be proved to be optimal by exploring the complete search 
tree in depth first search as follows:
  cp.setParameter(IloCP.IntParam.SearchType, IloCP.ParameterValues.DepthFirst);  
  cp.solve(cp.searchPhase(refSequences));

Note that the solution built by using the optimal solution of the 
deterministic Job-Shop Scheduling problem using average operation duration 
yields an expected makespan of 4749.6 which is clearly suboptimal.

Nevertheless, in practical stochastic scheduling problems, a solution 
to a deterministic version of the problem (like the one using an average behavior) 
may provide an interesting starting point for the scenario-based stochastic model 
(See CP Optimizer Starting Point concept).

------------------------------------------------------------ */

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.ArrayList;
import java.util.List;

public class SchedStochasticJobShop {

  static class DataReader {

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

  static class IntervalVarList extends ArrayList<IloIntervalVar> {
    public IloIntervalVar[] toArray() {
      return (IloIntervalVar[]) this.toArray(new IloIntervalVar[this.size()]);
    }
  }
  
  static IloIntExpr MakeScenarioSubmodel(IloCP                    cp,
                                         int                      nbJobs,
                                         int                      nbMachines,
                                         int[][]                  machines,
                                         int[][]                  durations,
                                         IloIntervalSequenceVar[] sequences) throws IloException {
    IntervalVarList[] machinesOps = new IntervalVarList[nbMachines];
    String name;
    int i,j;
    for (j = 0; j < nbMachines; j++)
      machinesOps[j] = new IntervalVarList();
    IloIntExpr[] ends = new IloIntExpr[nbJobs];
    for (i = 0; i < nbJobs; i++) {
      IloIntervalVar prec = cp.intervalVar();
      for (j = 0; j < nbMachines; j++) {
        name = "J" + i + "_O" + j; 
        IloIntervalVar ti = cp.intervalVar(durations[i][j], name);
        machinesOps[machines[i][j]].add(ti);
        if (j > 0)
          cp.add(cp.endBeforeStart(prec, ti));
        prec = ti;
      }
      ends[i] = cp.endOf(prec);
    }
    for (j = 0; j < nbMachines; j++) {
      name = "M" + j;
      sequences[j] = cp.intervalSequenceVar(machinesOps[j].toArray(), name);
      cp.add(cp.noOverlap(sequences[j]));
    }
    return cp.max(ends);
  }
  
  static IloIntExpr[] arrayFromList(List<IloIntExpr> list) {
      return (IloIntExpr[]) list.toArray(new IloIntExpr[list.size()]);
  }

  public static void main(String[] args) throws IOException {

    String filename = "../../../examples/data/stochastic_jobshop_default.data";
    int failLimit = 250000;

    if (args.length > 0)
        filename = args[0];
    if (args.length > 1)
        failLimit = Integer.parseInt(args[1]);

    DataReader data = new DataReader(filename);
    try {
      // Data reading
      int nbJobs, nbMachines, nbScenarios;
      nbJobs      = data.next();
      nbMachines  = data.next();
      nbScenarios = data.next();
      int i,j,k;
      // machines[i][j]: machine used by jth operation of job i
      int[][] machines = new int[nbJobs][]; 
      for (i = 0; i < nbJobs; i++) {
        machines[i] = new int[nbMachines];
        for (j = 0; j < nbMachines; j++) {
          machines[i][j] = data.next();
        }
      }
      // durations[k][i][j]: duration of jth operation of job i in scenario k
      int[][][] durations = new int[nbScenarios][][]; 
      for (k = 0; k < nbScenarios; k++) {
        durations[k] = new int[nbJobs][];
        for (i = 0; i < nbJobs; i++) {
          durations[k][i] = new int[nbMachines];
          for (j = 0; j < nbMachines; j++) {
            durations[k][i][j] = data.next();
          }
        }
      }
 
      IloCP cp = new IloCP();
      IloIntervalSequenceVar[] refSequences = new IloIntervalSequenceVar[nbMachines];
      IloIntExpr sumMakespan = cp.intExpr();    
      for (k = 0; k < nbScenarios; k++) {
        IloIntervalSequenceVar[] scenarioSequences = new IloIntervalSequenceVar[nbMachines];
        IloIntExpr scenarioMakespan = 
          MakeScenarioSubmodel(cp, nbJobs, nbMachines,
                               machines, durations[k],
                               scenarioSequences);
        // Objective function is aggregated
        sumMakespan = cp.sum(sumMakespan, scenarioMakespan);
        // For each machine, a sameSequence constraint is posted across all scenarios
        if (0==k) {
          refSequences = scenarioSequences;
        } else {
          for (j = 0; j < nbMachines; j++) {
            cp.add(cp.sameSequence(refSequences[j], scenarioSequences[j]));
          }
        }
      }
      // Objective function is expected makespan
      IloNumExpr expectedMakespan = cp.quot(sumMakespan, nbScenarios);
      IloObjective objective = cp.minimize(expectedMakespan);
      cp.add(objective);
      cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
      cp.setParameter(IloCP.IntParam.LogPeriod, 1000000);
      System.out.println("Instance \t: " + filename);
      if (cp.solve()) {
        System.out.println("Expected makespan \t: " + cp.getObjValue());
        for (j=0; j<nbMachines; ++j) {
          IloIntervalSequenceVar s = refSequences[j];
          System.out.print(s.getName() + ":\t");
          IloIntervalVar op = cp.getFirst(s);
          for (; !op.equals(cp.getLast(s)); op = cp.getNext(s, op))
            System.out.print(op.getName()+ "\t");
          System.out.println(op.getName()+ "\t");
        }
      } else {
        System.out.println("No solution found.");
      }
    } catch (IloException e) {
      System.err.println("Error: " + e);
    }
  }
}
