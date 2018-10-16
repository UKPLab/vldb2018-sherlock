// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/plantlocation.cpp
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

A ship-building company has a certain number of customers. Each customer is supplied
by exactly one plant. In turn, a plant can supply several customers. The problem is
to decide where to set up the plants in order to supply every customer while minimizing
the cost of building each plant and the transportation cost of supplying the customers.

For each possible plant location there is a fixed cost and a production capacity.
Both take into account the country and the geographical conditions.

For every customer, there is a demand and a transportation cost with respect to
each plant location.

While a first solution of this problem can be found easily by CP Optimizer, it can take
quite some time to improve it to a very good one. We illustrate the warm start capabilities
of CP Optimizer by giving a good starting point solution that CP Optimizer will try to improve.
This solution could be one from an expert or the result of another optimization engine
applied to the problem.

In the solution we only give a value to the variables that determine which plant delivers
a customer. This is sufficient to define a complete solution on all model variables.
CP Optimizer first extends the solution to all variables and then starts to improve it.

------------------------------------------------------------ */

#include <ilcp/cp.h>

ILOSTLBEGIN

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char * argv[]) {
  IloEnv env;
  try{
    const char* filename = "../../../examples/data/plant_location.data";
    if (argc > 1)
      filename = argv[1];
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file>" << std::endl;
      throw FileError();
    }

    IloInt nbCustomer;
    file >> nbCustomer;
    IloInt nbLocation;
    file >> nbLocation;

    IloInt c, l;
    IloArray<IloIntArray> cost(env);
    for(c = 0; c < nbCustomer; c++){
      cost.add(IloIntArray(env, nbLocation));
      for(l = 0; l < nbLocation; l++){
        file >> cost[c][l];
      }
    }
    IloIntArray demand(env, nbCustomer);
    for(c = 0; c < nbCustomer; c++) file >> demand[c];
    IloIntArray fixedCost(env, nbLocation);
    for(l = 0; l < nbLocation; l++) file >> fixedCost[l];
    IloIntArray capacity(env, nbLocation);
    for(l = 0; l < nbLocation; l++) file >> capacity[l];

    IloIntVarArray cust(env, nbCustomer, 0, nbLocation - 1);
    IloIntVarArray open(env, nbLocation, 0, 1);
    IloIntVarArray load(env);
    for(l = 0; l < nbLocation; l++){
      load.add(IloIntVar(env, 0, capacity[l]));
    }

    IloModel model(env);
    for(l = 0; l < nbLocation; l++){
      model.add(open[l] == (load[l] > 0));
    }
    model.add(IloPack(env, load, cust, demand));
    IloExpr obj(env);
    obj += IloScalProd(fixedCost, open);
    for(c = 0; c < nbCustomer; c++){
      obj += cost[c][cust[c]];
    }
    model.add(IloMinimize(env, obj));

    IloIntArray custValues(env, nbCustomer,
       19, 0, 11, 8, 29, 9, 29, 28, 17, 15, 7, 9, 18, 15, 1, 17, 25, 18, 17, 27,
       22, 1, 26, 3, 22, 2, 20, 27, 2, 16, 1, 16, 12, 28, 19, 2, 20, 14, 13, 27,
       3, 9, 18, 0, 13, 19, 27, 14, 12, 1, 15, 14, 17, 0, 7, 12, 11, 0, 25, 16,
       22, 13, 16, 8, 18, 27, 19, 23, 26, 13, 11, 11, 19, 22, 28, 26, 23, 3, 18, 23,
       26, 14, 29, 18, 9, 7, 12, 27, 8, 20);

    IloSolution sol(env);
    for(c = 0; c < nbCustomer; c++){
      sol.setValue(cust[c], custValues[c]);
    }

    IloCP cp(model);
    cp.setStartingPoint(sol);
    cp.setParameter(IloCP::TimeLimit, 10);
    cp.setParameter(IloCP::LogPeriod, 10000);
    cp.solve();
  }
  catch(IloException& e){
    env.out() << " ERROR: " << e.getMessage() << std::endl;
  }
  env.end();
  return 0;
}
