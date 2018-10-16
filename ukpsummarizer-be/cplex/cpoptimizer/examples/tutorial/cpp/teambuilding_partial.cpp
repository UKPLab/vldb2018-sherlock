// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/tutorial/cpp/teambuilding_partial.cpp
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

const IloInt nbPersons = 60;
const IloInt nbTeams = 10;
const IloInt teamSize = 6;
const IloInt nbServices = 6;

IloInt coaching[nbPersons];

/*
MakeTeamTuples return a IloIntTupleSet containing all the possible configurations of a team.
The team members in a tuple are ordered to break symmetry.
*/
IloIntTupleSet MakeTeamTuples(IloEnv globalEnv) {
  IloEnv env;
  IloModel model(env);
  IloInt i;

  IloIntArray newEmployee(env, nbPersons);
  IloIntArray service(env, nbPersons);
  for (i = 0; i < nbPersons; i++) {
    newEmployee[i] = ((i % 2) == 0);
    if      (i < 20) service[i] = 0;
    else if (i < 40) service[i] = 1;
    else if (i < 45) service[i] = 2;
    else if (i < 50) service[i] = 3;
    else if (i < 55) service[i] = 4;
    else             service[i] = 5;
  }

  //Add the tupleset
  //Add the team members variable array
  //Add the constraint on the number of new employees
  //Add the constraint on coaching pairs
  //Add the service unit variables
  //Add the constraint on cardinality of service unit on team
  //Add the constraint on disjoint service units
  //Add the symmetry reducing constraint

  //Start the search
    //Search for all solutions
  //End the env for the subproblem
}

int main(int argc, const char* argv[]){
  IloInt i;
  for (i = 0; i < nbPersons ; i++)
    coaching[i]= -1;
  // the 12 first of Service A are couples of coached/coach
  for (i = 0; i < 12 ; i=i+2) {
    coaching[i]=i+1;
    coaching[i+1]=i;
  }
  // the 12 first of Service B are couples of coached/coach
  for (i = 20; i < 32 ; i=i+2) {
    coaching[i]=i+1;
    coaching[i+1]=i;
  }
  // the 4 first of Services C,D,E,F are couples of coached/coach
  for (i = 40; i < nbPersons; i+=5) {
    coaching[i]=i+1;
    coaching[i+1]=i;
    coaching[i+2]=i+3;
    coaching[i+3]=i+2;
  }

  IloEnv env;
  try {
    IloIntTupleSet tupleSet = MakeTeamTuples(env);
    IloModel model(env);
    //Add the group variables and allowed assignments
    //Add the all diff constraint
    //Add the team variables
    //Add the preference constraints
    //Add the symmetry constraint

    //Create an instance of IloCP
    //Modify the search
    //Search for a solution
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
