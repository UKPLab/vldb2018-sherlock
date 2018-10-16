// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/schedsearch_greedyandrelax.cpp
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

The problem aims to dispatch workers and schedule a set of
orders. An order is a set of tasks linked by precedence
constraints. Each way to perform an order is a possible recipe for the
order.

Moreover, the problem aims to quickly build a schedule that may
leave some ordereds unperformed, partially performed some others, or delay their
execution date. 

------------------------------------------------------------ */


/* ------------------------------------------------------------

Rationale
---------


The schedule building is a greedy algorithm, that, when it fails,
returns a guilty order. The guilty order is relaxed and the greedy
algorithm restarted until all orders are scheduled, eventually by 
unperforming some. The greedy scheduler uses a chronological 
dynamic list scheduling had-oc heuristics.

To achieve this in CP Optimizer, a model is created
according to the most possible relaxation level. Then the search
manager will iterate until it has relaxed the jobs enough to
find a solution with the greedy scheduler. Before running the 
scheduler, a user constraint constrains the variables in the 
CP engine in a function of the current level of relaxation of 
the orders.


------------------------------------------------------------ */

/* ------------------------------------------------------------

A design Aspect
---------------

When writing your own goal, it is important to have a clear separation
between the data coming from the model and some global parameters (the
model declaration data) and the data manipulated by the search
(the model solving data: instances of IlcIntervalVar class, reversible data,
parameters of the search procedure, ...).

The memory used by the first one is usually allocated with the
allocator of the instance of IloEnv (model variables and constraints, ...) or
your own allocator (usually the C++ allocator). The memory used by the latter
one must absolutely be allocated using the heap of the instance of
IloCP for this memory must be recovered when the instance of IloCP ends, may be
reversible and must not change outside of the search.

To achieve this, consider the data sets:
  The first one stores the model declaration data and is allocated using 
  the instance of IloEnv. On this environment, an instance of IloGoal
  wrapper that is used as argument of the solve method of the instance
  of IloCP can be created. This object in the model context.
  The second class is allocated using the instance of IloCP. It locally 
  stores any piece of information (decision variables, search parameters, 
  reversible data, search control invariants,...). It creates an instance 
  of IlcGoal that is actually used by the search. This class is the 
  engine context. Instances of these classes must 
  not point at any data coming from the model declaration.

  The wrapper goal of the model context, an instance of IloGoal,
  creates, using the heap of the instance, the instance of scheduler
  search procedure, and returns, using it, the instance of IlcGoal,
  the top level of the search procedure.

We emphasize such a design for it clearly separates the data from
the model declaration and the data structure of the solving procedure.

------------------------------------------------------------ */

/* ------------------------------------------------------------

Summary
-------

This example illustrates how to build a search procedure for CP
Optimizer. In particular, it shows:
   A safe design to transfer model and parameter components
   to the search manager in the instance of IloCP.
   How to write a search that builds a chronological schedule; here, a very simple
   search as there is no branch and bound tree traversal.
   How to write a dynamic selector for chronological list scheduling. In particular,
   only considering variables that are the actual decision variables.
   How to write a very simple constraint in the engine.
   How to use the internal solve method of the instance of IloCP to iterate
   on schedule building procedures. This is a key point as
   writing iterative improvment methods follows the same principle.

------------------------------------------------------------ */

#include <ilcp/cpext.h>

ILOSTLBEGIN


const IloInt NumberOfJobsA = 20;
const IloInt NumberOfJobsB = 12;

IloInt DeadlinesA []  = {  5, 15,  7, 24,  9, 10, 25, 17,  8, 15,
                          29,  2, 15, 17, 24,  7, 10, 28, 17, 10};
IloInt DeadlinesB []  = {  7, 18,  9, 27,  7, 10, 
                          21,  8, 18, 16, 26, 17};

const IloInt DeadlineExtension = 5;



/////////////////////////////////////////
//
// PROBLEM DEFINITION
//
/////////////////////////////////////////


class Job {
  /*
    This class is intended to store all interval variables from the
    model and the relaxation policy for each possible recipe for an order,
   */
private:
  IloInt _type; // drive the relaxation rules
  IloBool _representant;
  IloInt _deadline;
  IloIntervalVar _top;
  IloIntervalVar _span;
  IloIntervalVarArray _opers;
public:
  Job(IloInt type,
      IloInt deadline,
      IloBool rep,
      IloIntervalVar top,
      IloIntervalVar span,
      IloIntervalVarArray opers);
  IloInt getType() const { return _type;}
  IloInt getNumberOfIntervals() const { return _opers.getSize();}
  IloIntervalVar getSpan() const { return _span;}
  IloIntervalVar getTop() const { return _top;}
  IloIntervalVarArray getOperations() const { return _opers;}
  IloIntervalVar getOperation(IloInt i) const { return _opers[i];}
  IloInt getDeadline() const { return _deadline;};
  static IloInt getNumberOfWorkers(IloInt) { return 2;}
  IloInt getWorkers(IloInt i) { return (_type == 0) ? i : i + 2;}
  IloInt getNumberOfOpers() const { return (_type == 0) ? 4 : 2;}
  IloInt getExtendedDeadline() const { return _deadline + DeadlineExtension;}
  IloInt getAllowedAbsent() const { return (_type == 0) ? 2 : IloIntMin;}
  IloBool allowsAbsence() const { return _type == 0;}
  IloInt getMaxRelaxationLevel() const { return (_type == 0) ? 2 : 1;}
  IloBool isRepresentant() const { return _representant;}
};

Job::Job(IloInt type,
         IloInt deadline,
         IloBool representant,
         IloIntervalVar top,
         IloIntervalVar span,
         IloIntervalVarArray opers)
  :_type(type)
  ,_representant(representant)
  ,_deadline(deadline)
  ,_top(top)
  ,_span(span)
  ,_opers(opers)
{}


void MakeJob(IloModel model,
             IloCumulFunctionExprArray workers,
             const IloInt oindex,
             const IloInt type,
             const IloInt deadline,
             IloAnyArray jobs) {
  /* This function builds all the alternative recipes for an order.
     In this case, a recipe consists in:
        - Creating the intevals that correspond to the recipe and
        declare the span/alternative work breakdown structure.
        - Adding the precedences in a recipe. Note that if a task execution 
        can be relaxed, the precedences must hold between executed tasks
        - Choosing a worker for executing the tasks.
   */
  char buffer[128];
  IloEnv env = model.getEnv();
  IloInt numberOfWorkers = Job::getNumberOfWorkers(type);
  IloIntervalVar top(env);
  top.setOptional();
  sprintf(buffer, "Job%ld<Type%ld>", (long)oindex, (long)type);
  top.setName(buffer);
  IloInt i, w, j;
  IloIntervalVarArray spans(env);
  for (i = 0 ; i < numberOfWorkers; i++) {
    IloIntervalVar span(env);
    span.setOptional();
    spans.add(span);
    IloIntervalVarArray opers(env);
    Job* job = new (env) Job(type, deadline, (i == 0), top, span, opers);
    w = job->getWorkers(i);    
    sprintf(buffer, "Job%ld<Type%ldWorker%ld>", (long)oindex, (long)type, (long)w);
    span.setName(buffer);
    jobs.add((IloAny) job);
    IloInt a = job->getAllowedAbsent();
    IloInt numberOfOpers = job->getNumberOfOpers();
    for(j = 0; j < numberOfOpers; j++) {
      IloIntervalVar var(env, 1);
      var.setOptional();
      opers.add(var);
      sprintf(buffer, "Oper%ld<Job%ldType%ldWorker%ld>",
              (long)j, (long)oindex, (long)type, (long)w);
      var.setName(buffer);
      if (j >= 1)
        model.add(IloEndBeforeStart(env, opers[j - 1], var));
      if (j == a)
        model.add(IloPresenceOf(env, var) <= IloPresenceOf(env, span));
      else {
        model.add(IloPresenceOf(env, var) == IloPresenceOf(env, span));
        if ((j - 1 == a) && (j >= 2)) // due to relaxation policy
          model.add(IloEndBeforeStart(env, opers[j - 2], var));
      }
      workers[w] += IloPulse(var, 1);
    }
    model.add(IloSpan(env, span, opers));
  }
  model.add(IloAlternative(env, top, spans));
}

IloCumulFunctionExprArray DefineWorkers(IloEnv env) {
  /*
    Workers are cumul functions with some initial usage
   */
  IloCumulFunctionExprArray workers(env);
  for (IloInt i = 0; i < 4; ++i) 
    workers.add(IloCumulFunctionExpr(env));
  
  workers[0] += IloPulse(env, 3, 6, 1);
  workers[1] += IloPulse(env, 22, 23, 1);
  workers[2] += IloPulse(env, 3, 10, 1);
  workers[2] += IloPulse(env, 12, 22, 1);
  workers[3] += IloPulse(env, 6, 11, 1);
  workers[3] += IloPulse(env, 17, 25, 1);
  return workers;
}

IloModel DefineModel(IloEnv env, 
                     IloInt numberOfJobsA,
                     IloInt numberOfJobsB,
                     IloInt* deadlinesA,
                     IloInt* deadlinesB,
                     IloAnyArray jobs) {
  IloModel model(env);
  IloCumulFunctionExprArray workers = DefineWorkers(env);
  IloInt j;
  for(j = 0; j < numberOfJobsA; j++)
    MakeJob(model, workers, j, 0, deadlinesA[j], jobs);
  for(j = 0; j < numberOfJobsB; j++)
    MakeJob(model, workers, j + numberOfJobsA, 1, deadlinesB[j], jobs);
  for (IloInt i = 0; i < 4; ++i) 
    model.add(workers[i] <= 1);
  return model;
}


/////////////////////////////////////////////////////////
//
// SOLVING : RELAXATION AND LIST SCHEDULING
//
/////////////////////////////////////////////////////////


class JobRelaxation {
  /*
    This class is used in the engine to manage the relaxation policy
    for a recipe.
    An instance of this class must be allocated in the heap of the
    instance of IloCP and must not point out on any data coming from the
    IBM ILOG Concert Technology model.
    The interval variables are asked from the IloCP instance of the search.
   */
private:
  Job* _job;
  IloInt _relaxedLevel;
  IlcIntervalVarArray _opers;
  IlcIntervalVar _span;
public:
  JobRelaxation(IloCPEngine cp, Job* job);
  IloInt getNumberOfOpers() const { return _opers.getSize();}
  IlcIntervalVar getSpan() const { return _span;}
  IlcIntervalVar getInterval(IloInt i) const { return _opers[i];} 
  IloInt getRelaxedLevel() const { return _relaxedLevel;}
  void relax();
  void constrain();
  IloInt getSlack() const;
  IloBool isAbsent() const { return _span.isAbsent();}
};

JobRelaxation::JobRelaxation(IloCPEngine cp, Job* job) 
  :_job(job)
  ,_relaxedLevel(0)
  ,_opers(IlcIntervalVarArray(cp, job->getNumberOfIntervals()))
  ,_span(cp.getInterval(job->getSpan()))
{
  IloInt size = job->getNumberOfIntervals();
  IloIntervalVarArray opers = job->getOperations();
  for (IloInt i = 0; i < size; i++) 
    _opers[i] = cp.getInterval(opers[i]);
}

void JobRelaxation::relax() {
  if (_job->getMaxRelaxationLevel() < _relaxedLevel)
    throw IloWrongUsage("relaxation level overflow");
  _relaxedLevel++;
}

IloInt JobRelaxation::getSlack() const {
  if (_span.isAbsent())
    return IloIntMax;
  return _span.getEndMax() - _span.getEndMin();
}

void JobRelaxation::constrain() {
  /* This member function propagate to the interval of a
     job, the status of relaxation
  */
  if (_job->getMaxRelaxationLevel() < _relaxedLevel) {
    _span.setAbsent();
  } else if (_relaxedLevel == 0)
    _span.setEndMax(_job->getDeadline());
  else if (_relaxedLevel == 1)
    _span.setEndMax(_job->getExtendedDeadline());
  else if (_job->allowsAbsence()) {
    _opers[_job->getAllowedAbsent()].setAbsent();
    _span.setEndMax(_job->getExtendedDeadline());
  } else
    _span.setAbsent();
}


class RelaxGreedySearch {
  /*
    This class is the search manager of the engine. It must be
    allocated in the heap of the instance of IloCP and must copy
    any data from the model. 
    An instance of this class points out the jobs description in an
    array of instances of JobRelaxation.
    It also stores the current guilty job if any.
    The member function select implements the policy of greedy list scheduling
    priority.
    The member relax relaxes the current guilty jobs.
    The member makeGoal build an instance of IlcGoal that is the top
    level of the search iterations of the greedy scheduler.
    Recall this class must not contain any reference that are using memory
    outside of the heap allocator of the instance of IloCP. 
  */
private:
  IloInt _nbOfJobs;
  JobRelaxation** _jobs;
public:
  RelaxGreedySearch(IloCPEngine cp, IloAnyArray jobs);
  void constrain();
  JobRelaxation* getJob(IloInt j) const { return _jobs[j];}
  void relax(IloInt j);
  IloBool select(IloInt& job, IloInt& interval, IloInt& date) const;
  IlcGoal makeGoal(IloCPEngine cp);
};

class JobConstraint : public IlcConstraintI {
  /*
    This constraint class is used to constrain the jobs in function of
    their relaxation status.
    It is posted by the top level goal before starting the greedy scheduler
   */
private:
  RelaxGreedySearch* _s;
public:
  JobConstraint(IloCPEngine cp, RelaxGreedySearch* s)
    :IlcConstraintI(cp)
    ,_s(s)
  {}
  virtual void post() {};
  virtual void propagate() { _s->constrain();}
};

ILCGOAL2(GreedyListSchedulingGoal,
         RelaxGreedySearch*, s,
         IloInt&, guilty) {
  // greedy scheduling
  guilty = IloIntMin;
  IloInt j, i, d;
  while(s->select(j, i, d)) {
    guilty = j; 
    IlcIntervalVar var = s->getJob(j)->getInterval(i);
    // do not forget presence decision
    var.setPresent();
    var.setStart(d);
  }
  guilty = IloIntMin;
  return 0;
}

ILCGOAL1(RelaxGreedySearchGoal,
         RelaxGreedySearch*, s) {
  /*
    Top level goal: it used internal solve function of the CP instance
    for an iteration
  */
  IloCPEngine cp = getCPEngine();
  JobConstraint* jobCt =
    new (cp.getHeap()) JobConstraint(cp, s);
  IloInt guilty;
  IlcGoal g = IlcAnd(IlcGoal(jobCt), GreedyListSchedulingGoal(cp, s, guilty));
  IloInt iter = 0;
  while(IloTrue) {
    std::cout << "iteration " << ++iter << std::endl;
    if (cp.solve(g)) {
      std::cout << "\tSuccesfull" << std::endl;
      break;
    }
    std::cout << "\tRelaxing "
              << s->getJob(guilty)->getSpan().getName()
              << " To Level "
              << s->getJob(guilty)->getRelaxedLevel() + 1
              << std::endl;
    
    s->relax(guilty);
  }
  
  return 0;
}

RelaxGreedySearch::RelaxGreedySearch(IloCPEngine cp, IloAnyArray jobs)
  :_nbOfJobs(jobs.getSize()),
   _jobs(0)
{
  IloInt size = jobs.getSize();
  _jobs = new (cp.getHeap()) JobRelaxation*[size];
  for(IloInt j = 0; j < size; ++j) {
    JobRelaxation* job =
      new (cp.getHeap()) JobRelaxation(cp, (Job*) jobs[j]);
    _jobs[j] = job;
  }
}
 
IloBool
RelaxGreedySearch::select(IloInt& job, IloInt& interval, IloInt& date) const {
  /*
    The selection considers the intervals that are not span or
    alternative masters (for these ones are automatically fixed when the
    interval modeling tasks are all fixed). It is important to take
    into account the Work Breakdown Structure of the span/alternative
    graph for efficient scheduling decisions.
    Among non absent tasks, the first selection criterium is the start min, 
    we choose the slack of the span as tie break. 
  */
  IloInt minsmin = IloIntMax;
  IloInt minslack = IloIntMax;
  for(IloInt k = 0; k < _nbOfJobs; ++k) {
    JobRelaxation* ljob = _jobs[k];
    if (ljob->isAbsent())
      continue;
    IloInt nbOfOpers = ljob->getNumberOfOpers();
    IloInt slack = ljob->getSlack();
    assert(slack < IloIntMax);
    for(IloInt i = 0; i < nbOfOpers; ++i) {
      IlcIntervalVar var = ljob->getInterval(i);
      if (var.isFixed())
        continue;
      IloInt smin = var.getStartMin();
      if ((smin < minsmin) || ((smin == minsmin) && (slack < minslack))) {
        minsmin = smin;
        minslack = slack;
        job = k;
        interval = i;
        date = smin;
      }
    }
  }
  return (minsmin < IloIntMax);
}

void RelaxGreedySearch::constrain() {
  for(IloInt i = 0; i < _nbOfJobs; i++)
    _jobs[i]->constrain();
}

void RelaxGreedySearch::relax(IloInt guilty) {
  if ((guilty < 0) || (guilty >= _nbOfJobs))
    throw IloWrongUsage("index of relaxation interval out of range");
  _jobs[guilty]->relax();
}


IlcGoal RelaxGreedySearch::makeGoal(IloCPEngine cp) {
  return RelaxGreedySearchGoal(cp, this);
}

 
/////////////////////////////////////////
//
//MODEL SOLVING :  WRAPPING MODEL ENGINE
//
/////////////////////////////////////////


class RelaxGreedyScheduler {
  /*
    This class bounds model and parameters data of interest and the
    actual search managing class
  */
private:
  IloAnyArray _jobs;
public:
  RelaxGreedyScheduler(IloAnyArray jobs) :_jobs(jobs) {}
  IlcGoal initSearch(IloCPEngine cp);
  IloAnyArray getJobs() {return _jobs;}
};

ILCGOAL1(RelaxGreedyInitSearchGoal, RelaxGreedyScheduler*, s) {
  /*
    At this time the model is surely extracted.
   */
  IloCPEngine cp = getCPEngine();
  RelaxGreedySearch* rg =
    new (cp.getHeap()) RelaxGreedySearch(cp, s->getJobs());
  return rg->makeGoal(cp);
} 
 
ILOCPGOALWRAPPER1(RelaxGreedSchedulerGoal, cp, RelaxGreedyScheduler*, s) {
  /*
    Wrapper goal: it creates the search manager.
   */
  return s->initSearch(cp);
}

IlcGoal RelaxGreedyScheduler::initSearch(IloCPEngine cp) {
  return RelaxGreedyInitSearchGoal(cp, this);
}

/////////////////////////////////////////
//
// MAIN FUNCTION
//
/////////////////////////////////////////

void PrintSolution(IloCP cp, IloAnyArray jobs) {
  /* reading of the solution from the instance of IloCP */
  std::cout  << "Solution: " << std::endl;
  for(IloInt j = 0; j < jobs.getSize(); ++j) {
    Job* job = (Job*) jobs[j];
    if (cp.isPresent(job->getTop())) {
      if (cp.isPresent(job->getSpan())) {
        std::cout << job->getSpan().getName();
        if (cp.getEnd(job->getSpan()) > job->getDeadline())
          std::cout << " is Late";
        else
          std::cout << " is On Time";
        std::cout << std::endl;
        for (IloInt i = 0; i < job->getNumberOfIntervals(); i++) {
          std::cout << "\t";
          std::cout << "\t" << cp.domain(job->getOperation(i));
          if (!cp.isPresent(job->getOperation(i)))
            std::cout << " is Relaxed";
          std::cout << std::endl;
        }
      }
    } else if (job->isRepresentant()) {
      std::cout << job->getTop().getName();
      std::cout << " is Unperformed" << std::endl;
    }
  }
}

int main(int, const char*[]) {
  IloEnv env;
  try {
    IloAnyArray jobs(env);
    IloModel model = DefineModel(env,
                                 NumberOfJobsA,
                                 NumberOfJobsB,
                                 DeadlinesA,
                                 DeadlinesB,
                                 jobs);

    IloCP cp(model);
    // creation of the search wrapper
    RelaxGreedyScheduler* s =
      new (env) RelaxGreedyScheduler(jobs);
    IloGoal g = RelaxGreedSchedulerGoal(env, s);
    if (cp.solve(g))
      PrintSolution(cp, jobs);
    else 
      std::cout << "No solution found." << std::endl;
  } catch (IloException& exc) {
    std::cout << exc << std::endl;
  } 
  env.end();
  return 0;
}
