// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/facility_partial.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2013. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

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

    const char* filename = "../../../examples/data/facility.dat";
    if (argc > 1)
      filename = argv[1];
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file>" << std::endl;
      throw FileError();
    }

    //Model the data
    //Input the data

    nbLocations = capacity.getSize();
    nbStores = cost.getSize();
    IloBool consistentData = (fixedCost.getSize() == nbLocations);
    for (i = 0; consistentData && (i < nbStores); i++)
      consistentData = (cost[i].getSize() == nbLocations);
    if (!consistentData){
      env.out() << "ERROR: data file '"
                << filename << "' contains inconsistent data" << std::endl;
    }

    //Declare the supplier decision variables
    //Declare the warehouse open decision variables
    //Add the constraint on open warehouses
    //Add the capacity constraints

    //Build the objective expression
    //Add the objective to the model

    //Create an instance of IloCP
    //Search for a solution

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
