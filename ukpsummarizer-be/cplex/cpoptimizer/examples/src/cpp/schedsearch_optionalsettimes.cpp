// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/schedsearch_optionalsettimes.cpp
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

The MMRCPSP (Multi-Mode Resource-Constrained Project Scheduling
Problem) is a generalization of the Resource-Constrained Project
Scheduling problem (see sched_rcpsp.cpp). In the MMRCPSP, each
activity can be performed in one of several modes. Each mode 
of an activity represents an alternative way of combining different 
levels of resource requirements with a related duration. Renewable 
and nonrenewable resources are distinguished. While renewable 
resources have a limited instantaneous availability such as manpower
and machines, nonrenewable resources are limited for the entire 
project, allowing to model, e.g., a budget for the project. The 
objective is to find a mode and a start time for each activity 
such that the schedule is makespan minimal and feasible with 
regard to the precedence and resource constraints.

------------------------------------------------------------ */

/* ------------------------------------------------------------

Preliminary remarks
-------------------

This sample is based upon a standard academic benchmark in Resource 
Scheduling. It is a reduction of real applications. Some instances 
of MMRCPSP are still open. The goal of this sample is not to
be more efficient than the automatic search procedure of
CP Optimizer. The goal of this sample is to give the user principles of
search procedure heuristics, design implementation hints, and source
code of building blocks.

Please read the example schedsearch_settimes.cpp first.

------------------------------------------------------------ */


/* ------------------------------------------------------------

Rationale
---------

Fixing an interval variable requires fixing its presence value.
The main difference between solving RCPSMM as compared to RCPSP are the
decisions regarding the presence value of the selected interval. In other
words, scheduling an interval consists of setting it present and setting 
the value of the endpoints or fixing it absent. The sample uses a similar 
search algorithm than for sched_settimes.cpp (chronological setting time + 
postponing cut dominance rules), but it is modified to handle the presence 
value. The decision scheme of the search tree traversal for a present 
interval in the model can be summarized by:

until all intervals are fixed
  select a candidate interval and the boundary date
  apply postpone dominance rule relatively to the boundary date
     Try
            Schedule the interval to start at boundary date
         Or Postpone the interval from boundary date

After the selection of an optional interval variable, the presence status 
must be fixed. For a schedulable intervals at the boundary of the partial 
schedule, the decision branching scheme becomes:

until all intervals are fixed
  select a candidate interval and the boundary date
  apply the postpone dominance rule relatively to the boundary date
  if the interval is present
     Try
            Schedule the interval to be present and to start at boundary date
         Or Postpone the interval from boundary date
  else
     Try
            Schedule the interval to be present and to start at boundary date
         Or Postpone the interval from boundary date
         Or Fix the interval to be absent

The selection application requires an accurate knowledge of the lower bound of 
the start date for the selection among schedulable activities. If a schedulable optional 
interval were actually present, it could influence the domain of other
intervals and, by the action of the propagation algorithm, have its
own start lower bound delayed. In other word, the engine propagates with
candidate intervals being optional and the decision is accurate only
for present intervals. That means the selection procedure can be invalid.

To bypass this effect, before instantiating the interval, a preventive
check if the interval is not present is required. If the upper bound of the 
start increases, then update it to its new value and go back to the
selection procedure. This is achieved by using a shaving of the start of the
interval thanks to an internal solve that sets the presence of the interval.
The decision branching scheme becomes:

until all intervals are fixed
  select a candidate interval and the boundary date
  apply the postpone dominance rule relatively to the boundary date
  if the interval is present
     Try
            Schedule the interval to be present and to start at boundary date
         Or Postpone the interval from boundary date
  else
   if (Try to set the interval present) 
      if (start lower bound > boundary date) // already postponed
        continue
      Try
            Schedule the interval to be present and to start at boundary date
         Or Postpone the interval from boundary date
         Or Fix the interval to be absent


------------------------------------------------------------ */

/* ------------------------------------------------------------

An Optimization
---------------

The "Try present ..." decision updates the lower bound of the interval 
if necessary. So, whenever the  "Try present ..." decision fails, the decision 
procedure is fine. If the  "Try present ..." decision succeeds, which 
should be the most common case, the setPresent intsruction on the interval 
happens twice, one in the "Try present ...", one in the "Schedule ...": 
that is, the exact same propagation happens twice.

To avoid this replication, we store the information coming from the 
"Schedule .. " decision when the presence setting is applied, and, if the 
selection is not valid, fail (abandon the current branch). Then the
"Postpone" decision:
   use the stored information to update the interval
   if the selection is valid, branch between postponing or fixing
   as absent.

The part of the decision scheme related to the not present intervals
 becomes

...
   else
     allocate memory to store the upper bound ub
     Try
         Schedule2(interval, boundary date, ub)
      Or Postpone2(interval, boundary date, ub)


Where Schedule2 branch is:

Schedule2(interval, boundary date, ub)
   ub = -inf
   set interval present
   ub = lower bound of start of interval
   if (ub > boundary date)
      fail
   else
      Fix start of interval to boundary date

      
And Postpone2 branch is :

Postpone2(interval, boundary date, ub)
   set upper bound of start of interval to ub
   if (upper bound of interval <= ub)
      Try
           Postpone the interval from boundary date
        Or Fix the interval to be absent
   

In practice, this kind of technique works with any other
decision that is associated with the scheduling of an interval.

------------------------------------------------------------ */

  
/* ------------------------------------------------------------

Summary
-------

In this sample we show how to extend a chronological schedule search
heuristics to optional intervals in the model. The method applies 
regardless of the chronological schedule heuristics measure used in the
search. If the measure changes when the interval becomes present, 
the measure was not valid and the selection of the interval must be
delayed.

We see how to shave the end-points of an interval for
non present interval scheduling in order to be sure the selection
heuristics was applied under valid assumptions. This practice can be
generalized for any co-decision that is to be taken for
scheduling an interval 

This sample should be used for search design and as building blocks
of source code to write a custom search.


------------------------------------------------------------ */


#include <ilcp/cpext.h>

class ScheduleParam {
public:
  enum Policy {
    Automatic, // CP Optimizer automatic search
    Passive,
    LocalCut,  // On the fly selection validation
    Shaving    // First shaving the start time by presence fixing 
  };
};

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : Class Declarations and goals
//
///////////////////////////////////////////////////////////////////////////////


class SetTimesSearch {
private:
  IlcIntervalVarArray _intervals;
  IlcRevInt* _postponedDates;
  IlcIntVar _makespan;
  ScheduleParam::Policy _policy;
public:
  SetTimesSearch(IloCPEngine cp,
                 IloIntervalVarArray vars,
                 IloIntVar makespan,
                 ScheduleParam::Policy policy);
  IloCPEngine getCPEngine() const { return _intervals.getCPEngine();}
  IlcIntervalVar getInterval(IloInt index) const {
    return _intervals[index];
  }
  IloBool isPostponed(IlcInt i) const {
    return _postponedDates[i] >= _intervals[i].getStartMin();
  }
  IloBool select(IloInt& index, IloInt& date) const; 
  IloBool tryPresent(IlcIntervalVar var, IloInt date);
  static void schedule(IlcIntervalVar var, IloInt date);
  void postpone(IlcInt index, IloInt date);
  void testPostponed(IloInt date = IloIntervalMax);
  static IloBool computeLocalCut(IloInt* localcut, IlcIntervalVar var, IloInt date);
  static IloBool applyLocalCut(IloInt* localcut, IlcIntervalVar var, IloInt date);
  IlcIntVar getMakespan() const { return _makespan;}
  ScheduleParam::Policy getPolicy() const {
    return _policy;}
  IlcGoal makeGoal(IloCPEngine cp);
};

SetTimesSearch::SetTimesSearch(IloCPEngine cp,
                               IloIntervalVarArray vars,
                               IloIntVar makespan,
                               ScheduleParam::Policy policy)
  :_intervals(IlcIntervalVarArray(cp, vars.getSize()))
  ,_postponedDates(0)
  ,_makespan(cp.getIntVar(makespan))
  ,_policy(policy)
{
  IloInt size = vars.getSize();
  _postponedDates = new (cp.getHeap()) IlcRevInt[size];
  for(IloInt i = 0; i < size; ++i) {
    _intervals[i] = cp.getInterval(vars[i]);
    _postponedDates[i].setValue(cp, IloIntMin);
  }
}

IloBool SetTimesSearch::select(IloInt& index, IloInt& date) const {
  IloInt minsmin = IloIntervalMax + 1;
  IloInt minsmax = IloIntervalMax + 1;
  IloInt size = _intervals.getSize();
  for (IloInt i = 0; i < size; i++) {
    IlcIntervalVar var = _intervals[i];
    if (var.isFixed() || isPostponed(i))
      continue;
    IloInt smin = var.getStartMin();
    IloInt smax = var.getStartMax();
    if ((smin < minsmin) || ((smin == minsmin) && (smax < minsmax))) {
      minsmin = smin;
      minsmax = smax;
      index = i;
    }
  }
  date = minsmin;
  return (minsmin <= IloIntervalMax);
}

void SetTimesSearch::testPostponed(IloInt date) {
  IloInt size = _intervals.getSize();
  for(IloInt i = 0; i < size; ++i) {
    IlcIntervalVar var = _intervals[i];
    if (!var.isAbsent() && isPostponed(i) && 
      ((var.getEndMin() <= date) || (var.getStartMax() <= date)))
      var.setAbsent();
  }
}

void SetTimesSearch::schedule(IlcIntervalVar var, IloInt date) {
  if (var.isAbsent()) 
    return;
  else if (var.getStartMin() > date) {
    var.setAbsent();
  } else {
    var.setPresent();
    var.setStart(date);
  }
}

void SetTimesSearch::postpone(IlcInt index, IloInt date) {
  IlcIntervalVar var = _intervals[index];
  if ((!var.isAbsent() || (var.getStartMin() > date))) {
    _postponedDates[index].setValue(getCPEngine(), date);
  }
}

ILCGOAL3(ScheduleSetTimesGoal,
         SetTimesSearch*, s,
         IlcIntervalVar, var,
         IlcInt, date) {
  s->schedule(var, date);
  return 0;
}

ILCGOAL3(PostponeSetTimesGoal,
         SetTimesSearch*, s,
         IlcInt, index,
         IlcInt, date) {
  s->postpone(index, date);
  return 0;
}

ILCGOAL1(SetAbsentGoal, IlcIntervalVar, var) {
  var.setAbsent();
  return 0;
}

// Shaving by using Internal solve

ILCGOAL2(TryPresentGoal, IlcIntervalVar, var, IloInt&, mn) {
  var.setPresent();
  mn = var.getStartMin();
  return 0;
}

IloBool SetTimesSearch::tryPresent(IlcIntervalVar var, IloInt date) {
  IloCPEngine cp = getCPEngine();
  if (var.isAbsent())
    return IloFalse;
  else if (!var.isPresent()) {
    IloInt smin;
    if (!cp.solve(TryPresentGoal(cp, var, smin), IloTrue)) {
      var.setAbsent();
      return IloFalse;
    }
    var.setStartMin(smin);
  }
  assert(!var.isAbsent());
  return (var.getStartMin() <= date);
}

//// on the fly shaving of start time

IloBool SetTimesSearch::computeLocalCut(IloInt* localcut, 
                                        IlcIntervalVar var,
                                        IloInt date) {
  localcut[0] = IloIntervalMin - 1;
  if (var.isAbsent()) 
    return IlcTrue;
  localcut[0] = IloIntervalMax + 1;
  var.setPresent();
  IloInt min = var.getStartMin();
  localcut[0] = min;
  return (min > date);
}

IloBool SetTimesSearch::applyLocalCut(IloInt* localcut,
                                      IlcIntervalVar var,
                                      IloInt date) {
  IloInt nsmin = localcut[0];
  if (nsmin > date) {
    var.setStartMin(nsmin);
    return IloTrue;
  }
  return IloFalse;
}

ILCGOAL4(ScheduleSetTimesGoal2,
         SetTimesSearch*, s,
         IloInt*, localcut,
         IlcIntervalVar, var,
         IloInt, date) {
  assert(ScheduleParam::LocalCut == s->getPolicy());
  if (var.isAbsent())
    return 0;
  else if (!var.isPresent()) {
    if (s->computeLocalCut(localcut, var, date))
      return 0;
  }
  s->schedule(var, date);
  return 0;
}

ILCGOAL4(PostponeSetTimesGoal2,
         SetTimesSearch*, s,
         IloInt*, localcut,
         IloInt, index,
         IloInt, date) {
  assert(ScheduleParam::LocalCut == s->getPolicy());
  IlcIntervalVar var = s->getInterval(index);
  if (var.isAbsent())
    return 0;
  if (s->applyLocalCut(localcut, var, date))
    return 0;
  IloCPEngine cp = getCPEngine();
  return IlcOr(PostponeSetTimesGoal(cp, s, index, date),
               SetAbsentGoal(cp, var));
}

ILCGOAL1(SetTimesSearchGoal, SetTimesSearch*, s) {
  IlcInt index;
  IloInt date;
  IloCPEngine cp = getCPEngine();
  if (s->select(index, date)) {
    IlcIntervalVar var = s->getInterval(index);
    s->testPostponed(date);
    if (var.isPresent()) 
      return IlcAnd(IlcOr(ScheduleSetTimesGoal(cp, s, var, date),
                          PostponeSetTimesGoal(cp, s, index, date)),
                    this);
    if (s->getPolicy() == ScheduleParam::Shaving) {
      // Rough shaving the start with presence constraint
      // return false if selected date is not achievable
      if (!s->tryPresent(var, date))
        return this;
    }
    if (s->getPolicy() == ScheduleParam::LocalCut) {
      // on the fly start time shaving with presence fixing is
      // done in the schedule decision.
      // the result is memorized and used by the postpone decision
      IloInt* localcut = new (cp.getHeap()) IloInt[1L];
      return IlcAnd(IlcOr(ScheduleSetTimesGoal2(cp, s, localcut, var, date),
                          PostponeSetTimesGoal2(cp, s, localcut, index, date)),
                    this);
    } else
      return IlcAnd(IlcOr(ScheduleSetTimesGoal(cp, s, var, date),
                          PostponeSetTimesGoal(cp, s, index, date),
                          SetAbsentGoal(cp, var)),
                    this);
     
  }
  assert(s->getMakespan().isFixed());
  s->testPostponed(s->getMakespan().getMin());
  return 0;
}

IlcGoal SetTimesSearch::makeGoal(IloCPEngine cp) {
  return SetTimesSearchGoal(cp, this);
}

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : WRAPPER MODEL ENGINE  
//
///////////////////////////////////////////////////////////////////////////////

class RCPSPMMScheduler {
private:
  IloIntervalVarArray   _intervals;
  IloIntVar             _makespan;
  ScheduleParam::Policy _policy;
public:
  RCPSPMMScheduler(IloIntervalVarArray intervals,
                   IloIntVar makespan,
                   ScheduleParam::Policy policy)
    :_intervals(intervals), _makespan(makespan), _policy(policy)
  { assert(policy != ScheduleParam::Automatic);}
  IloIntervalVarArray getIntervals() const { return _intervals;}
  IloIntVar getMakespan() const { return _makespan;}
  ScheduleParam::Policy getPolicy() const { return _policy;}
  IlcGoal initSearch(IloCPEngine cp);
};

IlcGoal RCPSPMMScheduler::initSearch(IloCPEngine cp) {
  SetTimesSearch* r = new (cp.getHeap())
    SetTimesSearch(cp, getIntervals(), getMakespan(), getPolicy());
  return r->makeGoal(cp);
}

ILOCPGOALWRAPPER1(RCPSPMMSchedulerGoal, cp, RCPSPMMScheduler*, s) {
  return s->initSearch(cp);
}

///////////////////////////////////////////////////////////////////////////////
//
// MODEL : Data source, Parameters And Declaration
//
///////////////////////////////////////////////////////////////////////////////

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/rcpspmm_searchex.data";
    IloInt failLimit = 5000;
    ScheduleParam::Policy policy
      = ScheduleParam::LocalCut;
    if (argc > 1) {
      IloInt val = atoi(argv[1]);
      switch(val) {
      case 1:
        policy = ScheduleParam::Passive;
        break;
      case 2:
        policy = ScheduleParam::LocalCut;
        break;
      default:
        if (val > 2)
          policy = ScheduleParam::Shaving;
        else
          policy = ScheduleParam::Automatic;
      }
    }
    if (argc > 2)
      failLimit = atoi(argv[2]);
    if (argc > 3)
      filename = argv[1];
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <policy> <failLimit> <file>" << std::endl;
      throw FileError();
    }
    
    IloInt i, j, k;
    IloModel model(env);
    IloInt nbTasks, nbRenewable, nbNonRenewable;
    file >> nbTasks >> nbRenewable >> nbNonRenewable;
    IloCumulFunctionExprArray renewables(env, nbRenewable);
    IloIntExprArray nonRenewables(env, nbNonRenewable);
    IloIntArray capRenewables   (env, nbRenewable);
    IloIntArray capNonRenewables(env, nbNonRenewable);
    for (j=0; j<nbRenewable; j++) {
      renewables[j] = IloCumulFunctionExpr(env);
      file >> capRenewables[j];
    }
    for (j=0; j<nbNonRenewable; j++){
      nonRenewables[j] = IloIntExpr(env);
      file >> capNonRenewables[j];
    }

    IloIntervalVarArray  tasks(env, nbTasks);
    IloIntArray modes(env, nbTasks);
    IloIntervalVarArray intervals(env);
    char buffer[128];
    for (i=0; i<nbTasks; i++) {
      tasks[i] = IloIntervalVar(env);
      sprintf(buffer, "T%ld", (long)i);
      tasks[i].setName(buffer);
    }
    IloIntExprArray ends(env);
    for (i=0; i<nbTasks; i++) {
      IloIntervalVar task = tasks[i];
      IloInt d, nbModes, nbSucc;
      file >> d >> nbModes >> nbSucc;
      modes[i] = nbModes;
      if (nbModes == 1)
        intervals.add(task);
      else {
        IloIntervalVarArray altModes(env);
        for (k=0; k<nbModes; ++k) {
          IloIntervalVar alt(env);
          sprintf(buffer, "T%ldM%ld", (long)i, (long)k);
          alt.setName(buffer);
          alt.setOptional();
          altModes.add(alt);
          intervals.add(alt);
        }
        model.add(IloAlternative(env, task, altModes));
      }
      if (nbSucc == 0) {
        ends.add(IloEndOf(task));
      } else {
        for (IloInt s=0; s<nbSucc; ++s) {
          IloInt succ;
          file >> succ;
          model.add(IloEndBeforeStart(env, task, tasks[succ]));
        }
      }
    }
    IloInt k0 = 0;
    for (i=0; i<nbTasks; i++) {
      IloInt k1 = k0 + modes[i];
      for (k=k0; k < k1; ++k) {
        IloInt taskId, modeId, d;
        file >> taskId >> modeId >> d;
        intervals[k].setSizeMin(d);
        intervals[k].setSizeMax(d);
        IloInt q;
        for (j = 0; j < nbRenewable; j++) {
          file >> q;
          if (0 < q) {
            renewables[j] += IloPulse(intervals[k], q);
          }
        }
        for (j = 0; j < nbNonRenewable; j++) {
          file >> q;
          if (0 < q) {
            nonRenewables[j] += q * IloPresenceOf(env, intervals[k]);
         }
       }
      }
      k0 = k1;
    }
    
    for (j = 0; j < nbRenewable; j++) {
      model.add(renewables[j] <= capRenewables[j]);
    }

    for (j = 0; j < nbNonRenewable; j++) {
      model.add(nonRenewables[j] <= capNonRenewables[j]);
    }
    IloIntVar obj(env, 0, 10000);
    model.add(obj == IloMax(ends));
    IloObjective objective = IloMinimize(env, obj);
    model.add(objective);
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    std::cout << "Instance \t: " << filename << std::endl;
    IloBool result;
    if (ScheduleParam::Automatic == policy) {
      std::cout << "+++++ Automatic Search +++++" << std::endl;
      cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
      result = cp.solve();
    } else {
      RCPSPMMScheduler* rcpsp =
        new (env) RCPSPMMScheduler(intervals, obj, policy);
      switch(policy) {
      case ScheduleParam::Passive:
        std::cout << "+++++ Regular Set Times +++++" << std::endl;
        break;
      case ScheduleParam::LocalCut:
        std::cout << "+++++ Set Times With Presence Local Cut +++++" << std::endl;
        break;
      default:
        std::cout << "+++++ Set Times With Presence Shaving +++++" << std::endl;
      }
      IloGoal g = RCPSPMMSchedulerGoal(env, rcpsp);
      result = cp.solve(g);
    }
    if (result) {
      cp.out() << "Makespan \t: " << cp.getObjValue() << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}
