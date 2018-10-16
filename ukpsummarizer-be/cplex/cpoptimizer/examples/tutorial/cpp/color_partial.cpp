// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/color_partial.cpp
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

const char* Names[] = {"blue", "white", "yellow", "green"};

int main(int argc, const char * argv[]){
  //Create the environment
  try {
    //Create the model
    //Declare the decision variables
    //Add the constraints
    //Create an instance of IloCP
    //Search for a solution
      {
      cp.out() << std::endl << cp.getStatus() << " Solution" << std::endl;
      cp.out() << "Belgium:     " << Names[cp.getValue(Belgium)] << std::endl;
      cp.out() << "Denmark:     " << Names[cp.getValue(Denmark)] << std::endl;
      cp.out() << "France:      " << Names[cp.getValue(France)] << std::endl;
      cp.out() << "Germany:     " << Names[cp.getValue(Germany)] << std::endl;
      cp.out() << "Luxembourg:  " << Names[cp.getValue(Luxembourg)] << std::endl;
      cp.out() << "Netherlands: " << Names[cp.getValue(Netherlands)] << std::endl;
     }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
