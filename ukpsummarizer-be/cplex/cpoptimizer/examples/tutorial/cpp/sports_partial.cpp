// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/sports_partial.cpp
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

IloInt Game(IloInt h, IloInt a, IloInt n) {
  assert(h != a);
  return h * (n - 1) + a - (a > h);
}

typedef IloArray<IloIntVarArray> IloIntVarArray2;

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    IloModel model(env);

    IloInt n = 10;
    if (argc > 1)
      n = atoi(argv[1]);
    if ((n % 2) == 1)
      n++;
    env.out() << "Finding schedule for " << n << " teams" << std::endl;

    //Calculate the data
    //Declare the game, home team and away team variables
    //Calculate the allowed tuples
    //Add the constraint on allowed combinations

    //Add the alldiff constraint
    //Add the inverse constraint
    //Add the week of game constraint

    //Create the different halves constraint
    //Create the distance constraint
    //Add max break length constraints
    //Add the constraint on first and last weeks

    //Add the objective

    //Add surrogate constraints
    //Add more surrogate constraints
    //Add constraints to fix first week
    //Add the slot order constraint

    //Create an instance of IloCP
    //Add the time limit
    //Create the search selectors
    //Create the tuning object
    //Search for a solution

      cp.out() << "Solution at " << cp.getValue(breaks) << std::endl;
    }
    cp.endSearch();
    cp.end();
  } catch (IloException & ex) {
    env.out() << "Caught " << ex << std::endl;
  }
  env.end();
  return 0;
}
