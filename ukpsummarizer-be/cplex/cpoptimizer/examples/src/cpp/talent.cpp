// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/talent.cpp
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

This example is inspired from the talent hold cost scheduling problem
described in:

T.C.E Cheng, J. Diamond, B.M.T. Lin.  Optimal scheduling in film
production to minimize talent holding cost.  Journal of Optimization
Theory and Applications, 79:197-206, 1993.

of which the 'Rehearsal problem' is a specific case:

Barbara M. Smith.  Constraint Programming In Practice: Scheduling
                   a Rehearsal.  Report APES-67-2003, September 2003.

See: http://www.csplib.org/Problems/prob039/


------------------------------------------------------------ */

#include <ilcp/cp.h>

// Read talent scheduling problem data from a file
IloBool ReadData(const char * filename,
                 IloIntArray actorPay,
                 IloIntArray sceneDuration,
                 IloArray<IloIntSet> actorInScene) {
  IloEnv env = actorPay.getEnv();
  std::ifstream in(filename);
  if (!in.good())
    return IloFalse;

  IloInt numActors, numScenes, a, s;

  in >> numActors;
  for (a = 0; a < numActors; a++) {
    IloInt pay;
    in >> pay;
    actorPay.add(pay);
  }

  in >> numScenes;
  for (s = 0; s < numScenes; s++) {
    IloInt duration;
    in >> duration;
    sceneDuration.add(duration);
  }

  for (a = 0; a < numActors; a++) {
    actorInScene.add(IloIntSet(env));
    for (s = 0; s < numScenes; s++) {
      IloBool inScene;
      in >> inScene;
      if (inScene)
        actorInScene[a].add(s);
    }
  }
  if (!in.good())
    return IloFalse;

  in.close();
  return IloTrue;
}

// Build the talent scheduling model
IloModel BuildModel(IloIntVarArray scene,
                    IloIntExpr& idleCost,
                    IloIntArray actorCost,
                    IloIntArray sceneDuration,
                    IloArray<IloIntSet> actorInScene) {
  IloEnv env = scene.getEnv();
  IloInt numScenes = scene.getSize();
  IloInt numActors = actorCost.getSize();
  IloModel model(env);

  // Make the slot-based secondary model
  IloIntVarArray slot(env, numScenes, 0, numScenes - 1);
  model.add(IloInverse(env, scene, slot));

  // Expression representing the global cost
  idleCost = IloIntExpr(env, 0);

  // Loop over all actors, building cost
  for (IloInt a = 0; a < numActors; a++) {
    // Expression for the waiting time for this actor
    IloIntExpr actorWait(env);

    // Calculate the first and last slots where this actor plays
    IloIntVarArray position(env);
    for (IloIntSet::Iterator it(actorInScene[a]); it.ok(); ++it)
      position.add(slot[*it]);
    IloIntExpr firstSlot = IloMin(position);
    IloIntExpr lastSlot = IloMax(position);

    // If an actor is not in a scene,
    // he waits if he is on set when the scene is filmed
    for (IloInt s = 0; s < numScenes; s++) {
      if (!actorInScene[a].contains(s)) { // not in scene
        IloIntExpr wait = (firstSlot <= slot[s] && slot[s] <= lastSlot);
        actorWait += sceneDuration[s] * wait;
      }
    }

    // Accumulate the cost of waiting time for this actor
    idleCost += actorCost[a] * actorWait;
  }
  model.add(IloMinimize(env, idleCost));
  return model;
}

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    const char * inputFile = "../../../examples/data/rehearsal.data";
    IloNum tlim = 10.0;
    if (argc > 1)
      inputFile = argv[1];
    if (argc > 2)
      tlim = atof(argv[2]);

    IloIntArray actorPay(env);
    IloIntArray sceneDuration(env);
    IloArray<IloIntSet> actorInScene(env);
    IloBool ok = ReadData(inputFile, actorPay, sceneDuration, actorInScene);
    if (!ok) {
      env.out() << "Error reading " << inputFile << std::endl;
    }
    else {
      // Create the decision variables, cost, and the model
      IloInt numScenes = sceneDuration.getSize();
      IloInt numActors = actorPay.getSize();
      IloIntVarArray scene(env, numScenes, 0, numScenes - 1);
      IloIntExpr idleCost;
      IloModel model = BuildModel(
        scene, idleCost, actorPay, sceneDuration, actorInScene
      );

      // Create the CP solver
      IloCP cp(model);
      cp.setParameter(IloCP::TimeLimit, tlim);
      cp.solve();

      cp.out() << "Solution of idle cost " << cp.getValue(idleCost) << std::endl;
      cp.out() << "Order:";
      for (IloInt s = 0; s < numScenes; s++)
        cp.out() << " " << 1 + cp.getValue(scene[s]);
      cp.out() << std::endl;

      // Give more detailed information on the schedule
      for (IloInt a = 0; a < numActors; a++) {
        cp.out() << "|";
        for (IloInt s = 0; s < numScenes; s++) {
          IloInt sc = cp.getValue(scene[s]);
          for (IloInt d = 0; d < sceneDuration[sc]; d++) {
            if (actorInScene[a].contains(sc))
              cp.out() << "X";
            else
              cp.out() << ".";
          }
          cp.out() << "|";
        }
        cp.out() << "  (Rate = " << actorPay[a] << ")" << std::endl;
      }
      cp.end();
    }
  } catch (IloException & ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
