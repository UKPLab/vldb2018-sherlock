// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/smooth.cpp
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

Problem Description
-------------------

An hospital has to assign nurses to shifts. There are 12 nurses, 4 shifts
and 14 days to schedule.  For each day, a minimum number of nurses have
to be assigned to each shifts.  The problem is to have a fair spread
of the the number of nurses allocated to all the shifts. We don't know
precisely in advance how many nurses must be assigned to each, we only
have a minimum.  For instance, if for one day a minimum of 3 nurses are
required for the first shift, 2 for the second, 4 for the third and 0 for
the fourth, then 12-(3+2+4)=3 nurses can be assigned this day to any of
the shift. The goal is to have a fair spread of these 3 assignments among
the shifts. For instance, we could have one nurse more in the first shift,
one more in the second and one more in the third. On the other hand
if 3 more nurses are added to the first shift then the solution is not
well balanced.

------------------------------------------------------------ */

#include <ilcp/cp.h>

void smoothingProblem(){
  IloEnv env;
  try {
    IloModel model(env);

    IloInt numDays = 14;
    IloInt numShifts = 4;
    IloInt numNurses = 12;

    // demandMin contains the minimum number of nurses for each shift of each day
    IloIntArray demandMin(env,numShifts*numDays,
                          3, 3, 4, 3, 3, 2, 2, 3, 3, 4, 3, 3, 2, 2,
                          3, 3, 4, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3,
                          2, 2, 2, 2, 4, 4, 3, 2, 2, 2, 2, 4, 4, 3,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    IloIntVarArray work(env, numNurses * numDays, 0, numShifts - 1);
    // slacks variables are defined, there is one slack variable
    // for each day and for each shift. A slack variable is equal to
    // the number of nurses assigned to the shift for the day minus
    // the minimum demand associated with this shift
    IloIntVarArray slacks(env,numShifts*numDays,0,numNurses);

    IloInt sumDemandMin = 0;
    for (IloInt d = 0; d < numDays; d++) {
      // for each day, we define an array containing the nurses of this day
      IloIntVarArray nurses(env, numNurses);
      for (IloInt n = 0; n < numNurses; n++)
        nurses[n] = work[n + d * numNurses];

      for (IloInt s = 0; s < numShifts; s++) {
        IloInt i = s * numDays + d;
        sumDemandMin += demandMin[i];
        model.add(slacks[i] == IloCount(nurses, s) - demandMin[i]);
      }
    }

    // In order to have a well balanced solution, we define
    // the standard deviation of the slack variables
    model.add(IloSum(slacks) == (numNurses * numDays - sumDemandMin));
    IloNumExpr sd = IloStandardDeviation(slacks);

    // Since we know the number of nurses and the minimum demand of the shifts,
    // we can compute the perfect value of the standard variation, but using
    // the mathematical definition of the standard deviation. In our case,
    // the mean of the slack variables is equal to the number of nurses
    // minus the sum of the minimum demand divided by the total number of
    // shifts. Then we obtain the perfect value of the standard deviation
    // easily because only integer values can be used, so the perfect
    // assignment is to spread one value of slacks among all the possible shifts
    IloInt sumSlacks = numNurses * numDays - sumDemandMin;
    IloNum mean = sumSlacks / ((IloNum)(numShifts * numDays));
    IloNum sdLB = sqrt(sumSlacks * (1.0 - mean) / ((IloNum)(numShifts * numDays)));

    // Compensate for any FPU rounding error in lower bound calculation
    IloNum epsilon = 1e-9;

    // Apply lower bound with rounding correction
    model.add(sd >= sdLB - epsilon);

    // we ask for a solution minimizing the standard deviation
    model.add(IloMinimize(env,sd));

    model.add(IloSum(slacks) == numDays * numNurses - sumDemandMin);

    IloCP cp(model);

    cp.startNewSearch();
    while (cp.next()) {
      cp.out() << "NURSES WORK: " << std::endl;
      for (IloInt n = 0; n < numNurses; n++) {
        for (IloInt d = 0; d < numDays; d++) {
          cp.out() << cp.getValue(work[n + d * numNurses]) << " ";
        }
        cp.out() << std::endl;
      }
      cp.out() << "CARDINALITY VARS:" << std::endl;
      for (IloInt s = 0; s < numShifts; s++) {
        for (IloInt d = 0; d < numDays; d++) {
          cp.out() << demandMin[s * numDays + d]
                    + cp.getValue(slacks[s * numDays + d]) << " ";
        }
        cp.out() << std::endl;
      }
      cp.out() << "SLACK VARS: " << std::endl;
      for (IloInt s = 0; s < numShifts; s++) {
        for (IloInt d = 0; d < numDays; d++) {
          cp.out() << cp.getValue(slacks[s * numDays + d]) << " ";
        }
        cp.out() << std::endl;
      }
      cp.out() << "Perfect Standard Deviation : " << sdLB << std::endl;
      cp.out() << "Current standard deviation: " << cp.getValue(sd) << std::endl;
    }
    cp.endSearch();
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}

int main(int, const char * [])
{
  smoothingProblem();
  return 0;
}

/* Output:

NURSES WORK:
[3] [2] [2] [0] [2] [2] [1] [0] [0] [0] [0] [0] [2] [0]
[2] [2] [0] [2] [1] [2] [3] [1] [1] [0] [1] [0] [0] [1]
[0] [1] [0] [0] [1] [1] [2] [3] [3] [1] [1] [2] [3] [2]
[2] [2] [3] [1] [1] [2] [2] [1] [2] [3] [2] [1] [2] [2]
[0] [0] [1] [1] [3] [3] [0] [1] [2] [0] [1] [2] [0] [0]
[2] [0] [1] [1] [2] [2] [2] [2] [0] [0] [1] [1] [1] [1]
[1] [0] [1] [3] [0] [0] [0] [2] [1] [2] [2] [2] [1] [1]
[0] [1] [2] [0] [2] [0] [1] [1] [1] [1] [0] [2] [1] [3]
[0] [0] [0] [1] [0] [2] [1] [0] [2] [2] [3] [2] [2] [2]
[1] [1] [2] [0] [2] [0] [0] [0] [0] [1] [0] [0] [1] [1]
[1] [3] [1] [2] [1] [1] [2] [0] [0] [1] [0] [1] [2] [0]
[1] [1] [0] [2] [0] [1] [1] [2] [1] [0] [2] [1] [2] [2]
CARDINALITY VARS:
[4] [4] [4] [4] [3] [3] [3] [4] [4] [5] [4] [3] [2] [3]
[4] [4] [4] [4] [4] [3] [4] [4] [4] [4] [4] [4] [4] [4]
[3] [3] [3] [3] [4] [5] [4] [3] [3] [2] [3] [5] [5] [4]
[1] [1] [1] [1] [1] [1] [1] [1] [1] [1] [1] [0] [1] [1]
SLACK VARS:
1 1 0 1 0 1 1 1 1 1 1 0 0 1
1 1 0 1 1 0 1 1 1 0 1 1 1 1
1 1 1 1 0 1 1 1 1 0 1 1 1 1
1 1 1 1 1 1 1 1 1 1 1 0 1 1
Perfect Standard Deviation : 0.382993
Current standard deviation: [0.382993..0.382994]
 ! Search terminated, replaying optimal solution
 *         3.63     68527                              _139  =    1   0.382993
Number of branches      : 68527
Number of fails         : 26198
Number of choice points : 41966
Number of variables     : 358 (266 model + 92 additional)
Number of constraints   : 151
Total memory usage      : 843.3 Kb (764.9 Kb CP + 78.4 Kb Concert)
Time in last solve      : 3.63 (3.63 engine + 0.00 extraction)
Total time spent in CP  : 3.63
 ! ----------------------------------------------------------------------------
 ! Solution status        : Terminated normally, optimal solution found
 ! Number of branches     : 68527
 ! Number of fails        : 26198
 ! Total memory usage     : 843.3 Kb (764.9 Kb CP + 78.4 Kb Concert)
 ! Time spent in solve    : 3.63 (3.63 engine + 0.00 extraction)
 ! Search speed (br. / s) : 18904
 ! ----------------------------------------------------------------------------

*/
