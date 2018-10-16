// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/Teambuilding.java
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2017. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
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

import ilog.cp.*;
import ilog.concert.*;

public class Teambuilding {
    static int nbPersons=60;
    static int nbTeams=10;
    static int teamSize=6;
    static int nbServices = 6;
    
    static int coaching[];
    
    /* 
       MakeTeamTuples return a IloIntTupleSet containing all the 
       possible configurations of a team. 
       The team members in a tuple are ordered to break symmetry.
    */
    public static IloIntTupleSet MakeTeamTuples(IloCP mainCP) {
        try {
            IloCP cp = new IloCP();
            int i;
            int[] newEmployee = new int[nbPersons]; 
            int[] service  = new int[nbPersons]; 
            for (i = 0; i < nbPersons; i++) {
                if ((i % 2) == 0)
                    newEmployee[i] = 1;
                else 
                    newEmployee[i] = 0;
                if      (i < 20) service[i] = 0;
                else if (i < 40) service[i] = 1;
                else if (i < 45) service[i] = 2;
                else if (i < 50) service[i] = 3;
                else if (i < 55) service[i] = 4; 
                else             service[i] = 5;
            }
            
            IloIntTupleSet ts = mainCP.intTable(teamSize);
            
            IloIntVar[] teamMembers = cp.intVarArray(teamSize, 0, nbPersons-1);
            
            //number of new employees among the teamMembers = number 
            // of teamMembers / 2
            IloIntExpr nbNewEmployees =  cp.constant(0);
            for (i = 0; i < teamSize; i++)
                nbNewEmployees = cp.sum(nbNewEmployees, 
                                        cp.element(newEmployee, teamMembers[i]));
            cp.add(cp.eq(nbNewEmployees, teamSize / 2));
            
            //a new employee and his coach must be in the same team
            for (i = 0; i < 60; i += 2) {
                if (coaching[i] >= 0) {
                    cp.add(cp.eq( cp.count(teamMembers, i), cp.count(teamMembers, coaching[i])));
                }           
            }
            
            IloIntVar[] serviceVar = cp.intVarArray(teamSize, 0, nbServices - 1); 
            for (i = 0; i < teamSize; i++)
                cp.add(cp.eq(serviceVar[i], cp.element(service, teamMembers[i])));
            
            // at most 4 people of the same service
            for (i = 0; i < nbServices; i++) {
                cp.add(cp.le(cp.count(serviceVar, i),4));
                
            }
            
            // Persons of Services A and B cannot be in the same team
            // Persons of Services E and F cannot be in the same team
            cp.add( cp.or(cp.eq(cp.count(serviceVar, 0), 0), cp.eq(cp.count(serviceVar, 1), 0))); 
            cp.add( cp.or(cp.eq(cp.count(serviceVar, 4), 0), cp.eq(cp.count(serviceVar, 5), 0))); 
            
            // order the teamMembers to break symmetry
            for (i = 0; i < teamSize-1; i++) {
                cp.add(cp.lt(teamMembers[i],teamMembers[i+1]));
            }
            
            int[] tuple = new int[teamSize];

            cp.setParameter(IloCP.IntParam.LogVerbosity, 
                            IloCP.ParameterValues.Quiet);           
            cp.setParameter(IloCP.IntParam.SearchType, 
                            IloCP.ParameterValues.DepthFirst);      
            cp.startNewSearch();
            while (cp.next()) {     
                for (i = 0; i < teamSize; i++)
                    tuple[i]= (int) cp.getValue(teamMembers[i]);
                cp.addTuple(ts, tuple);
            }
            return ts;
        } catch (IloException e) {
            System.out.println("Error: " + e);
            e.printStackTrace();
        }
        return null;
    }

    public static void main( String[] args ) {  
        try {
            IloCP cp = new IloCP();
            coaching =  new int[nbPersons];
            int i;
            for (i = 0; i < nbPersons ; i++)
                coaching[i]= -1;
            for (i = 0; i < 12 ; i=i+2) { // the 12 first of Service A are couples of coached/coach
                coaching[i]=i+1;
                coaching[i+1]=i;
            }
            for (i = 20; i < 32 ; i=i+2) {// the 12 first of Service B are couples of coached/coach
                coaching[i]=i+1;
                coaching[i+1]=i;
            }
            for (i = 40; i < nbPersons; i+=5) { // the 4 first of Services C,D,E,F are couples of coached/coach
                coaching[i]=i+1;
                coaching[i+1]=i;
                coaching[i+2]=i+3;
                coaching[i+3]=i+2;
            }
            // compute the possible solutions of a team
            IloIntTupleSet tupleSet = MakeTeamTuples(cp);

            // groups[i] represents the ordered set of people in the team i 
            IloIntVar[][] groups = new IloIntVar[nbTeams][];
            for (i = 0; i < nbTeams; i++) {
                groups[i] = cp.intVarArray(teamSize, 0, nbPersons-1);
                cp.add(cp.allowedAssignments(groups[i], tupleSet));
            }

            IloIntVar[] allVars = cp.intVarArray(nbPersons);
            int s = 0;
            int w;
            int p;
            for (w = 0; w < nbTeams; ++w) {
                for (p = 0; p < teamSize; ++p) {
                    allVars[s] = groups[w][p];
                    ++s;
                }
            }
            cp.add(cp.allDiff(allVars));
            
            // team[i] represents the number of the team of people number i
            IloIntVar[] team= cp.intVarArray(nbPersons, 0, nbTeams);
            for (w = 0; w < nbTeams; ++w) {
                for (p = 0; p < teamSize; ++p) {
                    cp.add(cp.eq(cp.element(team, groups[w][p]),w));
                }
            }
            
            // Additional constraints
            // to improve efficiency we could force the following 
            // first three constraints directly in MakeTeamTuples but the fourth 
            // constraint cannot be expressed as a restriction of 
            // the tuple set, since it is not local to a tuple
            cp.add(cp.or(cp.eq(team[5], team[41]), cp.eq(team[5],team[51])));
            cp.add(cp.or(cp.eq(team[15], team[40]), cp.eq(team[15],team[51])));
            cp.add(cp.or(cp.eq(team[25], team[40]), cp.eq(team[25],team[50])));
            cp.add(cp.or(cp.eq(team[20], team[24]), cp.eq(team[22], team[50])));

            // break symmetry: the teams are ordered according to the smallest 
            // in each team 
            for (i=0; i<nbTeams-1; i++)
                cp.add(cp.lt(groups[i][0], groups[i+1][0])); 
            
            
            cp.setParameter(IloCP.IntParam.AllDiffInferenceLevel, 
                            IloCP.ParameterValues.Extended);
            
            if (cp.solve()) {
                System.out.println();
                System.out.println( "SOLUTION");
                for (p=0; p < nbTeams; ++p) {
                    System.out.print( "team " + p +" : ");
                    for (w=0; w < teamSize; ++w) {
                        System.out.print((int) cp.getValue(groups[p][w])+" ");
                    }
                    System.out.println();
                }
            }
            else
                System.out.println(  "**** NO SOLUTION ****");
        } catch (IloException e) {
            System.out.println("Error : " + e);
            e.printStackTrace();
        }
    }
}
            
