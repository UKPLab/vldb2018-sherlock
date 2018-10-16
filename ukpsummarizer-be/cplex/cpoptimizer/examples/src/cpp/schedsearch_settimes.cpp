// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/schedsearch_settimes.cpp
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

The RCPSP (Resource-Constrained Project Scheduling Problem) is a
generalization of the production-specific Job-Shop (see
sched_jobshop.cpp), Flow-Shop (see sched_flowshop.cpp) and Open-Shop
(see sched_openshop.cpp) scheduling problems. Given:

- a set of q resources with given capacities,
- a network of precedence constraints between the activities, and
- for each activity and each resource, the amount of the resource
  required by the activity over its execution,

The goal of the RCPSP is to find a schedule meeting all the
constraints whose makespan (i.e., the time at which all activities
are finished) is minimal.

------------------------------------------------------------ */


/* ------------------------------------------------------------

Preliminary remarks
------------------

This sample is based upon a standard academic benchmark in Resource 
Scheduling. It is a reduction of real applications and some instances 
of RCPSP are still open. The goal of this sample is not to try to be
more efficient than the automatic search procedures of CP Optimizer,
but to introduce the user to principles of search procedure
heuristics, design implementation hints, and source code buliding
blocks.

------------------------------------------------------------ */

  
/* ------------------------------------------------------------

Rationale
---------

In Resource Scheduling, the number of time points is often quite large,
generally much larger than the number of interval variables.
Any part of the problem manipulation, from
modeling to solving and displaying should avoid enumerating the 
time dimension. IBM ILOG Concert Technology and CP Optimizer
allow a user to formally describe a problem without time enumeration  
by providing classes of interval variables and the resource description 
framework as sequences of functions defined on consecutive integer 
intervals. For solving, the same practice must also hold. Other than 
some very specific cases like proving optimality, the predefined 
algorithms of CP Optimizer, such as propagation, variable selection, 
decision making, search tree traversal or cost improvement methods, 
enumerate only the intervals.

The setTimes algorithm aims to build a chronological schedule.
At a given schedule building state, the minimum of the lower bound 
of the start points of any not fixed intervals defines the date 
boundary of the
partial schedule. Any intervals that can start at
this date are said to be schedulable. Among schedulable intervals, a
tie break is applied to select an interval and the date for setting
the start time of the interval. The decision is:
  The selected interval starts at the date.
  Or the start date of the interval is delayed.

In practice, heuristic selection must be adapted to the real 
cases according to the structural properties and the objective
function of the problem. Examples given here:
  The schedulable interval criteria are more selective than 
  the lower bound of the start.
  The start time of the selected interval is not set at the exact
  boundary date.
  The length of the interval is variable and the end date of the
  interval has to be fixed. That is, we do not deal with a set of 
  schedulable intervals but a set of schedulable end-points of
  intervals.

The delaying of the start date in case of backtrack of the search 
tree is a policy that must be implemented depending upon the problem.
We present two different way of achieving this.

Caution: This kind of tree traversal heuristics is not complete 
unless very specific conditions are verified for each case 
of problems. That means, such heuristics cannot be used to prove 
optimality in a optimimization process. Nevertheless, they are knwon
to be quite fast and robust, because the selection argument for an
interval and its start time is usually locally quite pertinent since
the past intervals are fixed, and it takes full advantadge of the
propagation engine.

------------------------------------------------------------ */


/* ------------------------------------------------------------


setTimes algorithm with guilty resource occupation
--------------------------------------------------

A first idea for when backtracks happens, that is, when the start
time fixing of an intervals fails, is to find a sensible argument 
that allows delaying the start time of the interval.

In this sample, given the very simple structure of the temporal
network, we assume that the failure is due to the concurrency between
intervals that shares the same cumul functions at the boundary of the
schedule. At this point, the lower bound of the end of any not fixed
concurrent intervals is a sensible lower bound for the start time of
the failing interval. As you may notice, the interval is advanced of
a fraction of the typical length of the schedulable intervals.

The number of decisions to build a schedule is a function of the
number of intervals, the selection and propagation depends only
on the number of intervals (except in pathologic cases). Similarly, the
delay is typically a size of the lengths of the intervals in the model.
The search algorithm follows the no complete time enumeration practice.



------------------------------------------------------------ */

/* ------------------------------------------------------------

setTimes algorithm with dominance rule
--------------------------------------

A more powerful idea is to find a rule on the behavior of the search
algorithm from a global perpective to a local perspective. This is the
aim of dominance rules. An elementary case of dominance rule is
symetry breaking: Among the solutions, you select the ones that
belong to one equivalence class of the symmetry (global perspective) 
by adding constraints on subsets of variables (local perspective)
that reduce the problem to a solution in one equivalence class.

In our case, we build a chronological schedule; before the boundary
date t, all interval variables are fixed. In other words, we have a
partial schedule PS and the setting of the start time of selected 
interval v as soon as possible fail. Consequently, there exists a schedule 
solution that begins by PS, for which a decision delays the start time
of v (global perpective): the interval v is in fact not schedulable. We  
mark it "postponed from t". In further schedule building, we 
unmark v when setting times of some intervals leads the propagation 
to delay the start time of v after t.

We assume the propagation after each chronological decision is able to
accurately update the start time of any unscheduled intervals. So if
the current boundary date exceeds the lower bound of the end or the
lower bound of the start, the interval v should have been advanced.
The current partial schedule PS+v is necessarely dominated by
another one and must be abandoned (local perpective). The cut is
applied at the computation of the date boundary of PS setting absent 
any interval that violates the rule.

Such a technique supposes particuliar properties of the scheduling 
problems and may lose solutions when these properties do not hold. 
Otherwise, this technique is known to be very robust and quite fast.
Moreover, in case of a not perfect scheduling problem, considering
the structure of the problem locally to the schedulable intervals 
makes it possible to adapt the heuristcs. This kind of technique is 
extensively used in the automatic search engine of CP Optimizer. 

The number of decisions needed to build a schedule is a function of the 
number of intervals, the selection, the checking of postponing status 
and propagation also only depends on the number of intervals.

------------------------------------------------------------ */
  
/* ------------------------------------------------------------

Summary
-------

In this sample we learn how to build a chronological schedule search
heuristic either by using techniques like list schedulingdominance rules 
or ones that take advantadge of the structure of the problem,
such as concentrating on the dominant resources in the schedule.

The sample should be also used for search design and as source code for 
building blocks to write a custom search.

We introduce the usage of reversible data as instances of the class
IlcRevInt which stores information only valid on the remaining sub tree
search. 


------------------------------------------------------------ */

#include <ilcp/cpext.h>

ILOSTLBEGIN

///////////////////////////////////////////////////////////////////////////////
//
// MODEL INPUT DATA
//
///////////////////////////////////////////////////////////////////////////////

IloInt ResCapacities [] = { 9, 8, 10};

IloInt Activities [] = { 0, 0, 0, 0, 3, 2, 3, 4,
                         2, 3, 5, 2, 1, 5,
                         4, 5, 4, 3, 2, 5, 8,
                         1, 5, 2, 2, 2, 6, 7,
                         1, 4, 1, 4, 2, 9, 11,
                         2, 5, 5, 4, 1, 12,
                         4, 3, 5, 2, 1, 9,
                         2, 2, 4, 4, 1, 10,
                         4, 3, 2, 2, 1, 13,
                         7, 3, 2, 4, 2, 13, 14,
                         3, 3, 3, 2, 3, 13, 15, 16,
                         2, 4, 1, 4, 1, 13,
                         2, 1, 4, 4, 1, 18,
                         4, 2, 2, 2, 2, 17, 20,
                         2, 5, 5, 4, 1, 17,
                         5, 1, 5, 4, 1, 19,
                         3, 4, 5, 4, 2, 21, 22,
                         1, 3, 2, 3, 2, 21, 24,
                         5, 5, 3, 3, 1, 23,
                         6, 2, 4, 6, 1, 21,
                         1, 1, 6, 2, 1, 25,
                         3, 3, 2, 1, 1, 26,
                         2, 1, 0, 4, 1, 25,
                         7, 2, 2, 1, 1, 25,
                         5, 0, 1, 3, 1, 27,
                         5, 2, 2, 2, 1, 27,
                         0, 0, 0, 0, 0 };


///////////////////////////////////////////////////////////////////////////////
//
// MODEL DECLARATION
//
///////////////////////////////////////////////////////////////////////////////

class AllResources {
  // tasks per resource association
private:
  IloInt _size;
  IloIntArray* _intervalsPerResource;
public:
  AllResources(IloEnv env, IloInt size);
  IloInt getSize() const { return _size;}
  void addInterval(IloInt r, IloInt var) {
    _intervalsPerResource[r].add(var); }
  IloIntArray getIntervals(IloInt r) const {
    return _intervalsPerResource[r];
  }
};

AllResources::AllResources(IloEnv env, IloInt size)
  :_size(size),
   _intervalsPerResource(new (env) IloIntArray[size])
{
  for (IloInt i = 0; i < size; i++)
    _intervalsPerResource[i] = IloIntArray(env);
}

class AllJobs {
  // resources per task association
private:
  IloIntervalVarArray _intervals;
  IloIntArray* _resourcesPerInterval;
public:
  AllJobs(IloEnv env, IloInt size);
  IloIntervalVarArray getIntervals() const { return _intervals;}
  void add(IloIntervalVar var) {
    _intervals.add(var);
  }
  IloIntervalVar get(IloInt var) const {
    return _intervals[var];
  }
  void addResource(IloInt var, IloInt r) {
    _resourcesPerInterval[var].add(r);
  }
  IloIntArray getResources(IloInt var) const {
    return _resourcesPerInterval[var];
  }
};

AllJobs::AllJobs(IloEnv env, IloInt size) 
  :_intervals(IloIntervalVarArray(env))
   ,_resourcesPerInterval(new (env) IloIntArray[size])
{
  for (IloInt i = 0; i < size; i++)
  _resourcesPerInterval[i] = IloIntArray(env);
}

IloModel
DefineModel(IloEnv env,
            IloInt numberOfJobs,
            IloInt numberOfResources,
            IloInt* resourceCaps,
            IloInt* resourceUsage,
            IloInt* durations,
            IloInt** precedences,
            AllJobs* jobs,
            AllResources* resources,
            IloIntExpr &makespan)
{
  char buffer[128];
  /* CREATE THE SCHEDULE. */
  IloModel m(env);
  IloInt horizon = 0;
  IloInt k;
  for (k = 0; k < numberOfJobs; k++)
    horizon += durations[k];
  
  /* CREATE THE RESOURCES. */
  IloInt j;
  IloCumulFunctionExprArray cumuls(env);
  for (j = 0; j < numberOfResources; j++) {
    IloCumulFunctionExpr cumul(env);
    cumuls.add(cumul);
    sprintf(buffer, "R%ld", (long)j);
    cumul.setName(buffer);
  }

  /* CREATE THE ACTIVITIES and the requirements. */
  for (j = 0; j < numberOfJobs; j++) {
    IloIntervalVar var(env, durations[j]);
    var.setEndMax(horizon);
    jobs->add(var);
    sprintf(buffer, "J%ld", (long)j);
    var.setName(buffer);
    for (k = 0; k < numberOfResources; k++) {
      IloInt pos = k + j*numberOfResources;
      if (resourceUsage[pos] > 0) {
        cumuls[k] += IloPulse(var, resourceUsage[pos]);
        resources->addInterval(k, j);
        jobs->addResource(j, k);
      }
    }
  }
  
  /* create the cumul constraints */
  for (k = 0; k < numberOfResources; k++) 
    m.add(cumuls[k] <= resourceCaps[k]);
  IloIntervalVarArray intervals = jobs->getIntervals();
  /* CREATE THE PRECEDENCE CONSTRAINTS and makespan */
  IloIntExprArray ends(env);
  for (j = 0; j < numberOfJobs; j++) {
    if (precedences[j][0] == -1)
      ends.add(IloEndOf(intervals[j]));
    else {
      for (IloInt p = 0; precedences[j][p] != -1; ++p)
        m.add(IloEndBeforeStart(env,
                                intervals[j],
                                intervals[precedences[j][p]-1]));
    }
  }
  makespan = IloMax(ends);
  return m;
}

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : WRAPPER MODEL ENGINE CLASS DECLARATION 
//
///////////////////////////////////////////////////////////////////////////////

class RCPSPScheduler {
  AllJobs* _jobs;
  AllResources* _resources;
  IloInt _heuristics; //0 setTimes, 1 setTime2
public:
  RCPSPScheduler(AllJobs* jobs, AllResources* resources, IloInt heuristics)
    :_jobs(jobs),_resources(resources),_heuristics(heuristics) {}
  AllJobs* getJobs() const { return _jobs;}
  AllResources* getResources() const { return _resources;}
  IlcGoal initSearch(IloCPEngine cp);
};

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : SET TIMES WITH DOMINANCE RULE
//
///////////////////////////////////////////////////////////////////////////////

class SetTimesDominance {
private:
  IlcIntervalVarArray _intervals;
  IlcRevInt* _postponeDates;
public:
  SetTimesDominance(IloCPEngine cp, RCPSPScheduler* scheduler);
  IloCPEngine getCPEngine() const { return _intervals.getCPEngine();}
  IloBool select(IloInt &p, IloInt& date);
  IloBool isPostponed(IloInt i) const {
    return _postponeDates[i] >= _intervals[i].getStartMin();
  }
  void postpone(IloInt p, IloInt date);
  void schedule(IloInt p, IloInt date) {
    _intervals[p].setStart(date);
  }
  void testPostponed(IloInt date = IloIntervalMax);
  IlcGoal makeGoal(IloCPEngine cp);
};

SetTimesDominance::SetTimesDominance(IloCPEngine cp, RCPSPScheduler* s) 
  :_intervals(IlcIntervalVarArray
              (cp, s->getJobs()->getIntervals().getSize()))
   ,_postponeDates(0)
{
  IloInt i;
  IloInt nbOfIntervals = s->getJobs()->getIntervals().getSize();
  _postponeDates = new (cp.getHeap()) IlcRevInt[nbOfIntervals];
  IloIntervalVarArray intervals(s->getJobs()->getIntervals());
  for(i = 0; i < nbOfIntervals; ++i) {
    _intervals[i] = cp.getInterval(intervals[i]);
    _postponeDates[i].setValue(cp, IloIntMin);
  }
}

IloBool SetTimesDominance::select(IloInt &interval, IloInt& date) {
  // min start mim, min start max selection
  // That is a very common selection criteria
  IloInt minsmin = IloIntervalMax + 1;
  IloInt minsmax = IloIntervalMax + 1;
  IloInt nbOfIntervals = _intervals.getSize();
  for (IloInt i = 0; i < nbOfIntervals; i++) {
    IlcIntervalVar curr = _intervals[i];
    if (curr.isFixed() || isPostponed(i))
      continue;
    IloInt smin = curr.getStartMin();
    IloInt smax = curr.getStartMax();
    if ((smin < minsmin) || ((smin == minsmin) && (smax < minsmax))) {
      minsmin = smin;
      minsmax = smax;
      interval = i;
    }
  }
  date = minsmin;
  return (minsmin <= IloIntervalMax);
}

void SetTimesDominance::postpone(IloInt i, IloInt date) {
  if (!(_intervals[i].isAbsent() ||
        (_intervals[i].getStartMin() > date)))
    _postponeDates[i].setValue(getCPEngine(), date);
}

void SetTimesDominance::testPostponed(IloInt date) {
  IloInt nbOfIntervals = _intervals.getSize();
  for(IloInt i = 0; i < nbOfIntervals; ++i) {
    if (isPostponed(i) && !_intervals[i].isAbsent() && 
        ((_intervals[i].getEndMin() <= date) ||
         (_intervals[i].getStartMax() <= date)))
      _intervals[i].setAbsent();
  }
}

ILCGOAL3(ScheduleSetTimesDominanceGoal,
         SetTimesDominance*, s, IloInt, p, IloInt, date) {
  s->schedule(p, date);
  return 0;
}

ILCGOAL3(PostponeSetTimesDominanceGoal, SetTimesDominance*, s, IloInt, p, IloInt, date) {
  s->postpone(p, date);
  return 0;
}

ILCGOAL1(SetTimesDominanceGoal, SetTimesDominance*, s) {
  IloCPEngine cp = getCPEngine();
  IloInt i, date;
  if (s->select(i, date)) {
    s->testPostponed(date);
    return IlcAnd(IlcOr(ScheduleSetTimesDominanceGoal(cp, s, i, date),
                        PostponeSetTimesDominanceGoal(cp, s, i, date)),
                  this);
  }
  s->testPostponed();
  return 0;
}

IlcGoal SetTimesDominance::makeGoal(IloCPEngine cp) {
  return SetTimesDominanceGoal(cp, this);
}

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : SET TIMES WITH GUILTY RESOURCE OCCUPATION
//
///////////////////////////////////////////////////////////////////////////////

class setTimesResources {
  // variables to fixed
  // association resources intervals 
private:
  IlcIntervalVarArray _intervals;
  IloInt _nbOfResources;
  IloInt* _nbOfResourcesPerInterval;
  IloInt** _resourcesPerInterval;
  IloInt* _nbOfIntervalsPerResource;
  IloInt** _intervalsPerResource;
public:
  setTimesResources(IloCPEngine cp, RCPSPScheduler* scheduler);
  IloCPEngine getCPEngine() const { return _intervals.getCPEngine();}
  IloBool select(IloInt &p, IloInt& date);
  void postpone(IloInt p, IloInt date);
  void schedule(IloInt p, IloInt date) {
    _intervals[p].setStart(date);
  }
  IlcGoal makeGoal(IloCPEngine cp);
};

setTimesResources::setTimesResources(IloCPEngine cp, RCPSPScheduler* s) 
  :_intervals(IlcIntervalVarArray
              (cp, s->getJobs()->getIntervals().getSize()))
  ,_nbOfResources(s->getResources()->getSize())
  ,_nbOfResourcesPerInterval(0)
  ,_resourcesPerInterval(0)
  ,_nbOfIntervalsPerResource(0)
  ,_intervalsPerResource(0)
{
  IloInt size = _intervals.getSize();
  _nbOfResourcesPerInterval = new (cp.getHeap()) IloInt[size];
  _resourcesPerInterval = new (cp.getHeap()) IloInt*[size];
  IloInt i, r;
  IloInt nbOfIntervals = s->getJobs()->getIntervals().getSize();
  IloIntervalVarArray intervals = s->getJobs()->getIntervals();
  for(i = 0; i < nbOfIntervals; ++i) {
    IlcIntervalVar var = cp.getInterval(intervals[i]);
    _intervals[i] = var;
    IloIntArray resources = s->getJobs()->getResources(i);
    IloInt nbOfResources = resources.getSize();
    _nbOfResourcesPerInterval[i] = nbOfResources;
    if (nbOfResources > 0) {
      _resourcesPerInterval[i] =  new (cp.getHeap()) IloInt[nbOfResources];
      for(r = 0; r < nbOfResources; ++r)
        _resourcesPerInterval[i][r] = resources[r];
    } else
      _resourcesPerInterval[i] = 0;
  }
  _nbOfIntervalsPerResource = new (cp.getHeap()) IloInt[_nbOfResources];
  _intervalsPerResource = new (cp.getHeap()) IloInt*[_nbOfResources];
  for(r = 0; r < _nbOfResources; ++r) {
    IloIntArray resIntervals = s->getResources()->getIntervals(r);
    IloInt resNbOfIntervals = resIntervals.getSize();
    _nbOfIntervalsPerResource[r] = resNbOfIntervals;
    if (resNbOfIntervals > 0) {
      _intervalsPerResource[r] = new (cp.getHeap()) IloInt[resNbOfIntervals];
      for(i = 0; i < resNbOfIntervals; ++i)
        _intervalsPerResource[r][i] = resIntervals[i];
    } else
      _intervalsPerResource[r] = 0;
  }
}

IloBool setTimesResources::select(IloInt &interval, IloInt& date) {
  // min start mim, min start max selection
  // That is a very common selection criteria
  IloInt minsmin = IloIntervalMax + 1;
  IloInt minsmax = IloIntervalMax + 1;
  IloInt nbOfIntervals = _intervals.getSize();
  for (IloInt i = 0; i < nbOfIntervals; i++) {
    IlcIntervalVar curr = _intervals[i];
    if (curr.isFixed())
      continue;
    IloInt smin = curr.getStartMin();
    IloInt smax = curr.getStartMax();
    if ((smin < minsmin) || ((smin == minsmin) && (smax < minsmax))) {
      minsmin = smin;
      minsmax = smax;
      interval = i;
    }
  }
  date = minsmin;
  return (minsmin <= IloIntervalMax);
}

void setTimesResources::postpone(IloInt interval, IloInt date) {
  IlcIntervalVar var = _intervals[interval];
  if (var.isAbsent() || (var.getStartMin() > date))
    return;
  if (var.getStartMax() <= date) {
    var.setAbsent();
    return;
  }
  IloInt start = var.getStartMin();
  IloInt end = var.getEndMin();
  IloInt nsmin = var.getStartMax() + 1;
  IloInt nbOfResources = _nbOfResourcesPerInterval[interval];
  IloInt* resources = _resourcesPerInterval[interval];
  for(IloInt k = 0; k < nbOfResources; ++k) {
    IloInt r = resources[k];
    IloInt nbOfIntervals = _nbOfIntervalsPerResource[r];
    for(IloInt i = 0; i < nbOfIntervals; ++i) {
      if (i == interval)
        continue;
      IloInt j = _intervalsPerResource[r][i];
      IlcIntervalVar curr = _intervals[j];
      if (!curr.isFixed()) {
        IloInt emin  = curr.getEndMin();
        IloInt smin = curr.getStartMin();
        IloBool overlap = IlcMax(start, smin) < IlcMin(end, emin);
        if (overlap && (emin < nsmin))
          nsmin = emin;
      }
    }
  }
  var.setStartMin(nsmin);
}

ILCGOAL3(ScheduleSetTimesResourcesGoal,
         setTimesResources*, s, IloInt, p, IloInt, date) {
  s->schedule(p, date);
  return 0;
}

ILCGOAL3(PostponeSetTimesResourcesGoal, setTimesResources*, s, IloInt, p, IloInt, date) {
  s->postpone(p, date);
  return 0;
}

ILCGOAL1(SetTimesResourcesGoal, setTimesResources*, s) {
  IloCPEngine cp = getCPEngine();
  IloInt i, date;
  if (s->select(i, date))
    return IlcAnd(IlcOr(ScheduleSetTimesResourcesGoal(cp, s, i, date),
                        PostponeSetTimesResourcesGoal(cp, s, i, date)),
                  this);
  return 0;
}

IlcGoal setTimesResources::makeGoal(IloCPEngine cp) {
  return SetTimesResourcesGoal(cp, this);
}

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING  WRAPPER MODEL ENGINE GOALS
//
///////////////////////////////////////////////////////////////////////////////

ILCGOAL1(SetTimesResourcesSearch, RCPSPScheduler*, scheduler) {
  IloCPEngine cp = getCPEngine();
  setTimesResources* s = new (cp.getHeap()) setTimesResources(cp, scheduler);
  return s->makeGoal(cp);
}

ILCGOAL1(SetTimesDominanceSearch, RCPSPScheduler*, scheduler) {
  IloCPEngine cp = getCPEngine();
  SetTimesDominance* s = new (cp.getHeap()) SetTimesDominance(cp, scheduler);
  return s->makeGoal(cp);
}

IlcGoal RCPSPScheduler::initSearch(IloCPEngine cp) {
  switch(_heuristics) {
  case 1:
    return SetTimesResourcesSearch(cp, this);
  default:
    return SetTimesDominanceSearch(cp, this);
  }
  return 0;
}

ILOCPGOALWRAPPER1(RCPSPSchedulerGoal, cp, RCPSPScheduler*, s) {
  return s->initSearch(cp);
}

///////////////////////////////////////////////////////////////////////////////
//
// MAIN
//
///////////////////////////////////////////////////////////////////////////////

void
InitParameters(IloInt numberOfJobs,
               IloInt numberOfResources,
               IloInt*& resourceUsage,
               IloInt*& durations,
               IloInt**& precedences)
{
  IloInt usageIndex = 0;
  resourceUsage = new IloInt[numberOfJobs * numberOfResources];
  
  IloInt durIndex = 0;
  durations = new IloInt[numberOfJobs];

  IloInt precIndex = 0;
  precedences = new IloInt*[numberOfJobs];

  IloInt i = 0;
  IloInt k;
  for(k = 0; k < numberOfJobs; k++) {

    durations[durIndex++] = Activities[i++];

    IloInt j;
    for (j = 0; j < numberOfResources; j++)
      resourceUsage[usageIndex++] = Activities[i++];

    IloInt numNextJobs = Activities[i++];
    precedences[precIndex] = new IloInt[numNextJobs + 1];
    for(j = 0; j < numNextJobs; ++j)
      precedences[precIndex][j] = Activities[i++];
    precedences[precIndex][numNextJobs] = -1;
    precIndex++;
  }
}

IloBool SolveProblem(IloCP cp, IloInt heuristics,
                     AllJobs* jobs, AllResources* resources) {
  IloEnv env = cp.getEnv();
  cp.setParameter(IloCP::FailLimit, 20000);
  cp.setParameter(IloCP::CumulFunctionInferenceLevel, IloCP::Extended);
  if (heuristics == 0) {
    std::cout << "\t+++++ Automatic Search +++++" << std::endl;
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    return cp.solve();
  }
  switch(heuristics) {
  case 1:
    std::cout << "\t+++++ SetTimes Resources Search +++++" << std::endl;
    break;
  default:
    std::cout << "\t+++++ SetTimes Dominance Search +++++" << std::endl;
  }
  RCPSPScheduler* s = new (env) RCPSPScheduler(jobs, resources, heuristics);
  IloGoal g = RCPSPSchedulerGoal(env, s);
  return cp.solve(g);
}

void RcpspProblemSample(IloInt numberOfJobs,
                        IloInt numberOfResources,
                        IloInt* resourceCaps,
                        IloInt* resourceUsage,
                        IloInt* durations,
                        IloInt** precedences,
                        IloInt heuristics) {
  IloEnv env;
  try {
    AllResources* resources = new (env) AllResources(env, numberOfResources);
    AllJobs* jobs = new (env) AllJobs(env, numberOfJobs);
    IloIntExpr makespan;
    IloModel model = DefineModel(env,
                                 numberOfJobs,
                                 numberOfResources,
                                 resourceCaps,
                                 resourceUsage,
                                 durations,
                                 precedences,
                                 jobs,
                                 resources,
                                 makespan);
    IloCP cp(model);
    IloObjective objective = IloMinimize(env, makespan);
    model.add(objective);
    if (SolveProblem(cp, heuristics, jobs, resources)) {
      std::cout << "Makespan \t: " << cp.getObjValue() << std::endl;
      IloIntervalVarArray intervals = jobs->getIntervals();
      for (IloInt i = 0; i < intervals.getSize(); i++)
        std::cout << cp.domain(intervals[i]) << std::endl;
    } else {
      std::cout << "No solution found."  << std::endl;
    }
  } catch (IloException& exc) {
    std::cout << exc << std::endl;
  }
  env.end();
}

int main(int, const char*[]) {
  IloInt numberOfJobs = 27;
  IloInt numberOfResources = 3;
  IloInt* resourceCaps = ResCapacities;
  IloInt* resourceUsage = 0;
  IloInt* durations = 0;
  IloInt** precedences = 0;
  InitParameters(numberOfJobs,
                 numberOfResources,
                 resourceUsage,
                 durations,
                 precedences);
  RcpspProblemSample(numberOfJobs, numberOfResources, resourceCaps,
                     resourceUsage, durations, precedences,
                     0);
  RcpspProblemSample(numberOfJobs, numberOfResources, resourceCaps,
                     resourceUsage, durations, precedences,
                     1);
  RcpspProblemSample(numberOfJobs, numberOfResources, resourceCaps,
                     resourceUsage, durations, precedences,
                     2);
  for( IloInt i=0; i < numberOfJobs; ++i )
    delete [] precedences[i];
  delete [] precedences;
  delete [] durations;
  delete [] resourceUsage;
  return 0;
}
