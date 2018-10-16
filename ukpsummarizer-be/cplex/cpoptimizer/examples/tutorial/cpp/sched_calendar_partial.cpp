// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/sched_calendar_partial.cpp
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

IloInt NbTasks = 10;

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

char* TaskNames [] = {
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

IloInt TaskDurations [] = {
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

//Create the MakeHouse function
  IloEnv env = model.getEnv();

  //Create the interval variables
  //Add the precendence constraints
  //Add the tasks to workers
  //Add the cost expression
}

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {

    //Declare the objects needed for MakeHouse
    //Create the houses
    //Add the no overlap constraints
    //Add the intensity step functions
    //Add the weekends and holidays
    //Add the forbidden start and end constraints
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
