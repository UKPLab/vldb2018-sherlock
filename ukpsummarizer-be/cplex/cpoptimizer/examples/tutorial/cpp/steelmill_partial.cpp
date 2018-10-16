// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/steelmill_partial.cpp
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

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt m, o, c, q;

    //Create the data
    //Declare the decision variables
    //Add the pack constraint
    // Color constraints
    //Add the color constraints

  // Objective function
  //Add the objective
  //Add the constraints to reduce symmetry
  //Create an instance of IloCP
  //Search for a solution

  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
