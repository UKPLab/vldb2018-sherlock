// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sched_stochastic_jobshop.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2014. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------

The Stochastic Job-Shop Scheduling problem is a variant of the classical 
deterministic Job-Shop Scheduling problem (see sched_jobshop.cpp) where 
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
  cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);  
  cp.setSearchPhases(IloSearchPhase(env, refSequences));
  cp.solve();

Note that the solution built by using the optimal solution of the 
deterministic Job-Shop Scheduling problem using average operation duration 
yields an expected makespan of 4749.6 which is clearly suboptimal.

Nevertheless, in practical stochastic scheduling problems, a solution 
to a deterministic version of the problem (like the one using an average behavior) 
may provide an interesting starting point for the scenario-based stochastic model 
(See CP Optimizer Starting Point concept).

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

IloModel MakeScenarioSubmodel(IloEnv                       env,
                              IloInt                       nbJobs,
                              IloInt                       nbMachines,
                              IloIntArray2                 machines,
                              IloIntArray2                 durations,
                              IloIntervalSequenceVarArray& sequences,
                              IloIntExpr&                  makespan) {
  IloModel model(env);
  IloIntervalVarArray2 machineOps(env, nbMachines);
  char name[32];
  IloInt i,j;
  for (j = 0; j < nbMachines; j++)
    machineOps[j] = IloIntervalVarArray(env);
  IloIntExprArray ends(env);
  for (i = 0; i < nbJobs; i++) {
    IloIntervalVar prec;
    for (j = 0; j < nbMachines; j++) {
      sprintf(name, "J%ld_O%ld", (long)i, (long)j);
      IloIntervalVar ti(env, durations[i][j], name);
      machineOps[machines[i][j]].add(ti);
      if (0 != prec.getImpl())
        model.add(IloEndBeforeStart(env, prec, ti));
      prec = ti;
    }
    ends.add(IloEndOf(prec));
  }
  sequences = IloIntervalSequenceVarArray(env, nbMachines);
  for (j = 0; j < nbMachines; j++) {
    sprintf(name, "M%ld", (long)j);
    sequences[j] = IloIntervalSequenceVar(env, machineOps[j], name);
    model.add(IloNoOverlap(env, sequences[j]));
  }
  makespan = IloMax(ends);
  return model;  
}

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/stochastic_jobshop_default.data";
    IloInt failLimit = 250000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }
    
    // Data reading
    IloInt nbJobs, nbMachines, nbScenarios;
    file >> nbJobs;
    file >> nbMachines;
    file >> nbScenarios;
    IloInt i,j,k;
    // machines[i][j]: machine used by jth operation of job i
    IloIntArray2 machines(env, nbJobs); 
    for (i = 0; i < nbJobs; i++) {
      machines[i] = IloIntArray(env, nbMachines);
      for (j = 0; j < nbMachines; j++) {
        file >> machines[i][j];
      }
    }
    // durations[k][i][j]: duration of jth operation of job i in scenario k
    IloIntArray3 durations(env, nbScenarios); 
    for (k = 0; k < nbScenarios; k++) {
      durations[k] = IloIntArray2(env, nbJobs);
      for (i = 0; i < nbJobs; i++) {
        durations[k][i] = IloIntArray(env, nbMachines);
        for (j = 0; j < nbMachines; j++) {
          file >> durations[k][i][j];
        }
      }
    }
    
    IloModel model(env);
    IloIntervalSequenceVarArray refSequences(env, nbMachines);
    IloIntExpr sumMakespan(env);    
    for (k = 0; k < nbScenarios; k++) {
      IloIntExpr scenarioMakespan;
      IloIntervalSequenceVarArray scenarioSequences;
      IloModel scenario = MakeScenarioSubmodel(env, nbJobs, nbMachines,
                                               machines, durations[k],
                                               scenarioSequences,
                                               scenarioMakespan);
      // Scenario is added as sub-model
      model.add(scenario);
      // Objective function is aggregated
      sumMakespan += scenarioMakespan;
      // For each machine, a sameSequence constraint is posted across all scenarios
      if (0==k) {
        refSequences = scenarioSequences;
      } else {
        for (j = 0; j < nbMachines; j++) {
          model.add(IloSameSequence(env, refSequences[j], scenarioSequences[j]));
        }
      }
    }  
    // Objective function is expected makespan
    IloNumExpr expectedMakespan = sumMakespan / IloNum(nbScenarios);
    IloObjective objective = IloMinimize(env, expectedMakespan);
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.setParameter(IloCP::LogPeriod, 1000000);   
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
      cp.out() << "Expected makespan \t: " << cp.getObjValue() << std::endl;
      for (j=0; j<nbMachines; ++j) {
        IloIntervalSequenceVar s = refSequences[j];
        cp.out() << s.getName() << ":\t";
        for (IloIntervalVar op = cp.getFirst(s); op.getImpl() != 0; op = cp.getNext(s, op))
          cp.out() << op.getName() << "\t";
        cp.out() << std::endl;
      }
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}
