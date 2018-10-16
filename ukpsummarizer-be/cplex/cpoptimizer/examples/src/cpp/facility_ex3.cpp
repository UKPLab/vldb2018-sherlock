// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/facility_ex3.cpp
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

In addition, stores 2 and 7 may not be served by the same warehouse.

------------------------------------------------------------ */

#include <ilcp/cp.h>

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try{
    IloModel model(env);
    IloInt i, j;

    const char* filename = "../../../examples/data/facility.data";
    if (argc > 1)
      filename = argv[1];
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file>" << std::endl;
      throw FileError();
    }

    IloIntArray capacity(env), fixedCost(env);
    IloArray<IloIntArray> cost(env);
    IloInt nbLocations;
    IloInt nbStores;

    file >> nbLocations;
    file >> nbStores;
    capacity = IloIntArray(env, nbLocations);
    for(i = 0; i < nbLocations; i++){
      file >> capacity[i];
    }
    fixedCost = IloIntArray(env, nbLocations);
    for(i = 0; i < nbLocations; i++){
      file >> fixedCost[i];
    }
    for(j = 0; j < nbStores; j++){
      cost.add(IloIntArray(env, nbLocations));
      for(i = 0; i < nbLocations; i++){
        file >> cost[j][i];
      }
    }

    IloBool consistentData = (fixedCost.getSize() == nbLocations);
    consistentData = consistentData && nbStores <= IloSum(capacity);
    for (i = 0; consistentData && (i < nbStores); i++)
      consistentData = (cost[i].getSize() == nbLocations);
    if (!consistentData){
      env.out() << "ERROR: data file '"
                << filename << "' contains inconsistent data" << std::endl;
    }

    IloIntVarArray supplier(env, nbStores, 0, nbLocations - 1);
    for (j = 0; j < nbLocations; j++)
      model.add(IloCount(supplier, j)  <= capacity[j]);
    model.add(supplier[2] != supplier[7]);
    IloIntVarArray open(env, nbLocations, 0, 1);
    for (i = 0; i < nbStores; i++)
      model.add(open[supplier[i]] == 1);

    IloIntExpr obj = IloScalProd(open, fixedCost);
    for (i = 0; i < nbStores; i++)
      obj += cost[i][supplier[i]];
    model.add(IloMinimize(env, obj));
    IloCP cp(model);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.solve();

    cp.out() << std::endl << "Optimal value: " << cp.getValue(obj) << std::endl;
    for (j = 0; j < nbLocations; j++){
      if (cp.getValue(open[j]) == 1){
        cp.out() << "Facility " << j << " is open, it serves stores ";
        for (i = 0; i < nbStores; i++){
          if (cp.getValue(supplier[i]) == j)
            cp.out() << i << " ";
        }
        cp.out() << std::endl;
      }
    }
  }
  catch(IloException& e){
    env.out() << " ERROR: " << e.getMessage() << std::endl;
  }
  env.end();
  return 0;
}

/*
Optimal value: 1390
Facility 0 is open, it serves stores 0 5 7
Facility 1 is open, it serves stores 3
Facility 3 is open, it serves stores 1 2 4 6
*/
