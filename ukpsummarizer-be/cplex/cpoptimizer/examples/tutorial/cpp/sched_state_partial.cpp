// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/sched_state_partial.cpp
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

const IloInt NbHouses  = 5;
const IloInt NbTasks   = 10;
const IloInt NbWorkers = 2;

enum Tasks {
  masonry   = 0,
  carpentry = 1,
  plumbing  = 2,
  ceiling   = 3,
  roofing   = 4,
  painting  = 5,
  windows   = 6,
  facade    = 7,
  garden    = 8,
  moving    = 9
};

const char* TaskNames [] = {
  "masonry  ",
  "carpentry",
  "plumbing ",
  "ceiling  ",
  "roofing  ",
  "painting ",
  "windows  ",
  "facade   ",
  "garden   ",
  "moving   "
};

const IloInt TaskDurations [] = {
  35,
  15,
  40,
  15,
  05,
  10,
  05,
  10,
  05,
  05,
};

// Possible house state
const IloInt Clean = 0;
const IloInt Dirty = 1;

//Create the MakeHouse function
  IloEnv env = model.getEnv();
  //Create the interval variables
  //Add the temporal constraints
  //Add the state constraints

  //Add the cost expression
}

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {

  //Declare the objects needed for MakeHouse
  //Create the transition times
  //Create the houses

  //Add the cumulative constraints
  //Add the objective

  //Create an instance of IloCP
  //Search for a solution
      cp.out() << "Solution with objective " << cp.getObjValue() << ":" << std::endl;
      for (i=0; i<allTasks.getSize(); ++i) {
        cp.out() << cp.domain(allTasks[i]) << std::endl;
      }
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
