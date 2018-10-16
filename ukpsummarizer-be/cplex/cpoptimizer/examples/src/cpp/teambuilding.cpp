// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/teambuilding.cpp
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

Problem Description:
-------------------
The HR department of a company organizes an integration day to welcome new employees.
The problem is to configure 10 teams of 6 people that respect the following rules:
There are 30 new employees and 30 existing employees. They work in 6 different services lettered A to F.
A team must have 3 existing employees and 3 new employees, and at most 4 people from the same service.
Some new employees are coached by an existing employee, and an existing employee can coach only one new employee.
A new employee who is coached must be in the team of his coach.
Furthermore, employees of services A and B cannot be in the same team; employees of services E and F cannot be in the same team.

Each person is represented by a number in 0-59; new employees are the even numbers, existing employees are the odd numbers.

Service       Range
  A          0-19
  B          20-39
  C          40-44
  D          45-49
  E          50-54
  F          55-59

In Service A: the couples coach/coached new employee are 0-1, 2-3, 4-5, 6-7, 8-9, 10-11
In Service B: the couples coach/coached new employee are 20-21, 22-23, 24-25, 26-27, 28-29, 30-31
In Services C,D,E,F, the couples coach/coached new employee are 40-41, 42-43, 45-46, 47-48, 50-51, 52-53, 55-56, 57-58


Additional constraints:

Person number 5 must be with either person 41 or person 51.
Person number 15 must be with either 40 or person 51.
Person number 25 must be with either 40 or person 50.
Furthermore, person 20 is with person 24, or person 22 is with person 50.

Solving
-------
1 Generate all possible sets of people for a single team and store
  these sets in an IloIntTupleSet
  A team is represented by an ordered group of people.
  In this example, a possible solution of a team means taking into account
  all the constraints except the additional constraints (AC).

2 Find a compatible configuration for the 10 teams.
  An allowed assignment constraint forces each team to be assigned
  one of the possible solutions generated in step 1.
  Then, we need to take into account the additional constraints, as well
  as the fact that a person can be only in one team.

------------------------------------------------------------ */

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

  IloIntTupleSet ts(globalEnv, teamSize);

  IloIntVarArray teamMembers(env, teamSize, 0, nbPersons-1);

  //number of new employees among the teamMembers = number of teamMembers / 2
  IloIntExpr nbNewEmployees(env);
  for (i = 0; i < teamSize; i++)
    nbNewEmployees += newEmployee[teamMembers[i]];
  model.add(nbNewEmployees == teamSize / 2);

  //a new employee and his coach must be in the same team
  for (i = 0; i < 60; i += 2) {
    if (coaching[i] >= 0)
      model.add(IloCount(teamMembers, i) == IloCount(teamMembers, coaching[i]));
  }

  IloIntVarArray serviceVar(env, teamSize, 0, nbServices - 1);
  for (i = 0; i < teamSize; i++)
    model.add(serviceVar[i] == service[teamMembers[i]]);

  // at most 4 people of the same service
  for (i = 0; i < nbServices; i++)
    model.add(IloCount(serviceVar, i) <= 4);

  // Persons of Services A and B cannot be in the same team
  // Persons of Services E and F cannot be in the same team
  model.add(IloCount(serviceVar, 0) == 0 || IloCount(serviceVar, 1) == 0);
  model.add(IloCount(serviceVar, 4) == 0 || IloCount(serviceVar, 5) == 0);

  // order the teamMembers to break symmetry
  for (i = 0; i < teamSize-1; i++)
    model.add(teamMembers[i] < teamMembers[i+1]);

  IloIntArray tuple(globalEnv, teamSize);
  IloCP cp(model);
  cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
  cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
  cp.setParameter(IloCP::Workers, 1);
  cp.startNewSearch();
  while (cp.next()) {
    for (i = 0; i < teamSize; i++)
      tuple[i] = cp.getValue(teamMembers[i]);
    ts.add(tuple);
  }
  cp.end();
  env.end();
  return ts;
}

int main(int, const char* []){
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
    //compute the possible solutions of a team
    IloIntTupleSet tupleSet = MakeTeamTuples(env);
    IloModel model(env);

    // groups[i] represents the ordered set of people in the team i
    IloArray<IloIntVarArray> groups(env, nbTeams);
    for (i = 0; i < nbTeams; i++) {
      groups[i] = IloIntVarArray(env, teamSize, 0, nbPersons-1);
      model.add(IloAllowedAssignments(env,groups[i], tupleSet));
    }

    IloIntVarArray allVars(env, nbPersons);
    IloInt s = 0, w, p;
    for (w = 0; w < nbTeams; ++w) {
      for (p = 0; p < teamSize; ++p) {
        allVars[s] = groups[w][p];
        ++s;
      }
    }
    model.add(IloAllDiff(env, allVars));

    // team[i] represents the number of the team of people number i
    IloIntVarArray team(env, nbPersons, 0, nbTeams);
    for (w = 0; w < nbTeams; ++w) {
      for (p = 0; p < teamSize; ++p) {
        model.add(team[groups[w][p]]==w);
      }
    }

    // Additional constraints
    // to improve efficiency we could force the following
    // first three constraints directly in MakeTeamTuples but the fourth
    // constraint cannot be expressed as a restriction of
    // the tuple set, since it is not local to a tuple
    model.add(team[5]== team[41] || team[5]==team[51]);
    model.add(team[15]== team[40] || team[15]==team[51]);
    model.add(team[25]== team[40] || team[25]==team[50]);
    model.add(team[20]== team[24] || team[22]==team[50]);

  // break symmetry: the teams are ordered according to the smallest
  // in each team
    for (i=0; i<nbTeams-1; i++)
      model.add(groups[i][0] < groups[i+1][0]);

    IloCP cp(model);
    cp.setParameter(IloCP::AllDiffInferenceLevel, IloCP::Extended);

    if (cp.solve()) {
      cp.out() << std::endl << "SOLUTION" << std::endl;
      for (p=0; p < nbTeams; ++p) {
        cp.out() << "team " << p << " : ";
        for (w=0; w < teamSize; ++w) {
          cp.out() << cp.getValue(groups[p][w]) << " ";
        }
        cp.out() << std::endl;
      }
    }
    else
      cp.out() << "**** NO SOLUTION ****" << std::endl;
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
/*
SOLUTION
team 0 : 0 1 2 3 55 56
team 1 : 4 5 15 18 50 51
team 2 : 6 7 16 19 45 46
team 3 : 8 9 12 14 49 59
team 4 : 10 11 13 17 44 54
team 5 : 20 21 24 25 40 41
team 6 : 22 23 32 33 57 58
team 7 : 26 27 38 39 52 53
team 8 : 28 29 34 35 42 43
team 9 : 30 31 36 37 47 48
*/
