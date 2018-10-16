// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/sched_time_partial.cpp
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

//Add the earliness function
//Add the tardiness function

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    IloModel model(env);

    //Declare the interval variables
    //Add the precedence constraints

    //Add the objective
    //Create an instance of IloCP
    //Search for a solution
      cp.out() << "Optimal Value: " << cp.getObjValue() << std::endl;
      cp.out() << cp.domain(masonry)   << std::endl;
      cp.out() << cp.domain(carpentry) << std::endl;
      cp.out() << cp.domain(plumbing)  << std::endl;
      cp.out() << cp.domain(ceiling)   << std::endl;
      cp.out() << cp.domain(roofing)   << std::endl;
      cp.out() << cp.domain(painting)  << std::endl;
      cp.out() << cp.domain(windows)   << std::endl;
      cp.out() << cp.domain(facade)    << std::endl;
      cp.out() << cp.domain(garden)    << std::endl;
      cp.out() << cp.domain(moving)    << std::endl;
    } else {
      cp.out() << "No solution found. " << std::endl;
    }
    cp.printInformation();
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
