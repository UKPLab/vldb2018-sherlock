// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/ppp.cpp
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

 For a description of the problem and resolution methods:

    The Progressive Party Problem: Integer Linear Programming
    and Constraint Programming Compared

    Proceedings of the First International Conference on Principles
    and Practice of Constraint Programming table of contents

    Lecture Notes In Computer Science; Vol. 976, pages 36-52, 1995
    ISBN:3-540-60299-2

------------------------------------------------------------ */

#include <ilcp/cp.h>

//
// Matrix operations
//

IloArray<IloIntVarArray> Transpose(IloArray<IloIntVarArray> x) {
  IloEnv env = x.getEnv();
  IloInt n = x[0].getSize();
  IloInt m = x.getSize();
  IloArray<IloIntVarArray> y(env, n);
  for (IloInt i = 0; i < n; i++) {
    y[i] = IloIntVarArray(env, m);
    for (IloInt j = 0; j < m; j++)
      y[i][j] = x[j][i];
  }
  return y;
}

IloIntVarArray Flatten(IloArray<IloIntVarArray> x) {
  IloEnv env = x.getEnv();
  IloIntVarArray y(env);
  for (IloInt i = 0; i < x.getSize(); i++)
    y.add(x[i]);
  return y;
}

int main(int argc, const char * argv[]) {
  IloEnv env;

  try {
    //
    // Data
    //
    const IloInt numBoats = 42;
    IloIntArray boatSize(env, numBoats,
      7, 8, 12, 12, 12, 12, 12, 10, 10, 10,
      10, 10, 8, 8, 8, 12, 8, 8, 8, 8,
      8, 8, 7, 7, 7, 7, 7, 7, 6, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 9, 2,
      3, 4
    );
    IloIntArray crewSize(env, numBoats,
      2, 2, 2, 2, 4, 4, 4, 1, 2, 2,
      2, 3, 4, 2, 3, 6, 2, 2, 4, 2,
      4, 5, 4, 4, 2, 2, 4, 5, 2, 4,
      2, 2, 2, 2, 2, 2, 4, 5, 7, 2,
      3, 4
    );
    IloInt numPeriods = 6;
    if (argc > 1)
      numPeriods = atoi(argv[1]);

    //
    // Variables
    //

    // Host boat choice
    IloBoolVarArray host(env, numBoats);
    for (IloInt j = 0; j < numBoats; j++) {
      char name[20];
      sprintf(name, "H%ld", (long)j);
      host[j].setName(name);
    }

    // Who is where each time period (time- and boat-based views)
    IloArray<IloIntVarArray> timePeriod(env, numPeriods);
    for (IloInt i = 0; i < numPeriods; i++) {
      timePeriod[i] = IloIntVarArray(env, numBoats, 0, numBoats - 1);
      char name[20];
      for (IloInt j = 0; j < numBoats; j++) {
        sprintf(name, "T%ld,%ld", (long)i, (long)j);
        timePeriod[i][j].setName(name);
      }
    }
    IloArray<IloIntVarArray> visits = Transpose(timePeriod);

    //
    // Objective
    //
    IloModel mdl(env);
    IloIntVar numHosts(env, numPeriods, numBoats);
    mdl.add(numHosts == IloSum(host));
    mdl.add(IloMinimize(env, numHosts));

    //
    // Constraints
    //

    // Stay in my boat (host) or only visit other boats (guest)
    for (IloInt i = 0; i < numBoats; i++)
      mdl.add(IloCount(visits[i], i) == host[i] * numPeriods);

    // Capacity constraints: only hosts have capacity
    for (IloInt p = 0; p < numPeriods; p++) {
      IloIntVarArray load(env);
      for (IloInt j = 0; j < numBoats; j++) {
        char name[20];
        sprintf(name, "L%ld,%ld", (long)p, (long)j);
        load.add(IloIntVar(env, 0, boatSize[j], name));
        mdl.add(load[j] <= host[j] * boatSize[j]);
      }
      mdl.add(IloPack(env, load, timePeriod[p], crewSize, numHosts));
    }

    // No two crews meet more than once
    for (IloInt i = 0; i < numBoats; i++) {
      for (IloInt j = i + 1; j < numBoats; j++) {
        IloIntExpr timesMet(env, 0);
        for (IloInt p = 0; p < numPeriods; p++)
          timesMet += (visits[i][p] == visits[j][p]);
        mdl.add(timesMet <= 1);
      }
    }

    // Host and guest boat constraints: given in problem spec
    mdl.add(host[0] == IloTrue);
    mdl.add(host[1] == IloTrue);
    mdl.add(host[2] == IloTrue);
    mdl.add(host[39] == IloFalse);
    mdl.add(host[40] == IloFalse);
    mdl.add(host[41] == IloFalse);

    //
    // Solving
    //
    IloCP cp(mdl);
    if (cp.solve()) {
      cp.out() << "Solution at cost = " << cp.getValue(numHosts) << std::endl;
      cp.out() << "Hosts: ";
      for (IloInt i = 0; i < numBoats; i++)
        cp.out() << cp.getValue(host[i]);
      cp.out() << std::endl;

      for (IloInt i = 0; i < numBoats; i++) {
        cp.out() << "Boat " << i << " (size = " << crewSize[i] << "):\t";
        for (IloInt j = 0; j < numPeriods; j++)
          cp.out() << cp.getValue(visits[i][j]) << "\t";
        cp.out() << std::endl;
      }
      for (IloInt p = 0; p < numPeriods; p++) {
        cp.out() << "Period " << p << std::endl;
        for (IloInt h = 0; h < numBoats; h++) {
          if (cp.getValue(host[h])) {
            cp.out() << "\tHost " << h << " : ";
            IloInt load = 0;
            for (IloInt i = 0; i < numBoats; i++) {
              if (cp.getValue(visits[i][p]) == h) {
                load += crewSize[i];
                cp.out() << i << " (" << crewSize[i] << ") ";
              }
            }
            cp.out() << " --- " << load << " / " << boatSize[h] << std::endl;
          }
        }
      }
      cp.out() << std::endl;
    }
  } catch (IloException & ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
