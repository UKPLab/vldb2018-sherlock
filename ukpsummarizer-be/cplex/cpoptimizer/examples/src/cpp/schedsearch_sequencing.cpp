// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/schedsearch_sequencing.cpp
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

This problem is an extension of the classical Job-Shop Scheduling
problem which allows an operation to be processed by any machine 
from a given set. The processing time of an operation depends upon 
the allocated machine. The problem is to assign each operation to 
a machine and to schedule the operations on the machines such that 
the maximal completion time (makespan) of all operations is minimized.

------------------------------------------------------------ */

/* ------------------------------------------------------------

Preliminary remarks
-------------------

This sample is based upon a standard academic benchmark in Resource Scheduling.
It is a reduction of real applications. Some instances of Flexible Job-Shop
are still open. The goal of this sample is not to try  to be more efficient 
than the automatic search procedure of  CP Optimizer. The goal of this sample 
is to give to the user principles of search procedure heuristics, design 
implementation hints, and source code for building blocks.

------------------------------------------------------------ */

/* ------------------------------------------------------------

Rationale
---------

When solving scheduling problems, the main difficulty is often 
fixing sequence variables with no overlap constraints. For these 
types of problems, a good practice is to build the subsequences before 
scheduling the sequenced interval variables. An advantage of this 
practice is that the decisions to take are fixing the presence of the 
intervals and fixing the immediate successor of the intervals in the 
sequence; that is, the decisions depend only on the number of intervals 
and are enumerable. 

To build a chronological schedule, the Head-Tail graph of sequence 
variables is perfectly suited. That is, at a given state of decision 
taking, we select the interval that either is to be set present or
extends the head of a sequence. The search tree traversal can be
summarized by:

until all intervals are fixed and sequenced
  select a candidate sequence and interval
  if (interval is not present)
     try
           Set interval Present
        Or Set interval Absent
  else
     try to extend sequence head with the interval


Such a search tree traversal does not build a schedule: The end-points 
of present intervals are not fixed. Building the actual schedule requires 
interleaving phases of chronological sequencing and phases of chronological 
scheduling. An internal ad-hoc policy controls the swap from one phase to
another. In this sample, the parameter control is a date at which the
chronological sequencing stops and a phase of chronological sequencing on 
sequenced operations starts.

The chronological scheduling needs to choose the set of intervals to fix,
the scheduling algorithm and the horizon at which the scheduling phase
stop:
  The intervals are either operations or alternatives of operations on a
  machine. The scheduling phase only considers the intervals that are present and 
  sequenced.
  Because of the very simple structure of the problem, we choose to use a greedy 
  algorithm which consists in fixing the start of the sequenced intervals
  at their lower bounds. Industrial problems may require a more sophisticated 
  chronological scheduling algorithm such as a variant of the setTimes algorithm .
  The horizon is given by the control parameters that is used to stop the
  sequencing phase. The parameter value is equivalent to about fifteen sequenced 
  operations.
  
A last point is the selection policy of an interval to sequence: The primary selection 
of an interval from among candidate head is the one having the minimun 
lower bound of start. The tie breaker is the minimun of lower bound of start of  
interval. This is a very classical selection policy for chronological 
sequencing/scheduling. Anyway, it is often known to be too sytematic and of
poor quality for it lacks of diversity when a problem becomes locally hard.
It is well known in combinatorial optimization that randomizing the 
decisions selection is a good practice. This sample uses this hint to soften the  
tie breaking measurement. Note, a specific treatment applies on sequenced but 
not present intervals.

------------------------------------------------------------ */

/* ------------------------------------------------------------

Summary
-------

It is quite common that a fine grain knowledge of the problem structure 
and details (such that the critical resources of a problem or the structure 
of the objective function) allows to prepare the chronological schedule to 
take good decision. In such a case, you will have to design a two phases search 
tree traversal.

   The first phase will take precedence or logical decision on a
      subset of intervals (present interval, sequencing, batching and
      other synchronizations)
   The second phase will build the schedule in the range of date and
      the set of intervals that were covered by the first phase.
   The two phases are repeated until the schedule is completed.

In the Flexible Job-Shop Scheduling problem sample, we learn
  How to build a chronological sequencing search algorithm.
  How to interleave it with a chronological scheduling building
  algorithm.
  How to use randomization to diversify the selection of variables.


------------------------------------------------------------ */

#include <ilcp/cpext.h>

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING Class Declarations and goals
//
///////////////////////////////////////////////////////////////////////////////

class SequenceHeadSearch {
private:
  IlcIntervalSequenceVarArray _sequences;
  IlcIntExp _makespan;
  IlcRevInt _boundary;
public:
  SequenceHeadSearch(IloCPEngine cp,
                     IloIntervalSequenceVarArray sequences,
                     IloIntVar makespan);
  IloCPEngine getCPEngine() const { return _sequences.getCPEngine();}
  IloInt getBoundary() const { return _boundary;}
  IloBool select(IlcIntervalSequenceVar &s, IlcIntervalVar &p) const;
  void complete(IloInt boundary = IloIntervalMax + 1);
  IlcGoal makeGoal(IloCPEngine cp);
};

SequenceHeadSearch::SequenceHeadSearch(IloCPEngine cp,
                                       IloIntervalSequenceVarArray sequences,
                                       IloIntVar makespan)
  :_sequences(IlcIntervalSequenceVarArray(cp, sequences.getSize()))
  ,_makespan(cp.getIntVar(makespan))
  ,_boundary()
{
  IloInt size = sequences.getSize();
  for(IloInt i = 0; i < size; ++i)
    _sequences[i] = cp.getIntervalSequence(sequences[i]);
  _boundary.setValue(cp, 0);
}

IloBool MinStartDomainPrice(IlcIntervalVar p,
                            IlcIntervalSequenceVar s,
                            IloInt minsmin, IloInt minsmax) {
  IloInt smin = p.getStartMin();
  if (smin <= minsmin) {
    if (smin < minsmin)
      return IloTrue;
    if (s.isSequenced(p)) // that is p is not present
      return IlcTrue;
    // randomly amortized tie break 
    if (p.getStartMax() <= minsmax)
      return s.getCPEngine().getRandomNum() < .5;
  }
  return IlcFalse;
}

IloBool
SequenceHeadSearch::select(IlcIntervalSequenceVar &s,
                           IlcIntervalVar &var) const {
  IloInt minsmin = IloIntervalMax + 1;
  IloInt minsmax = IloIntervalMax + 1;
  IloInt size = _sequences.getSize();
  for(IloInt i = 0; i < size; ++i) {
    IlcIntervalSequenceVar ls = _sequences[i];
    if (ls.isFixed())
      continue;
    if (ls.getLatestInHead().getImpl() !=
        (ls.getLatestPresentInHead().getImpl())) {
      // top priority: sequenced in head but not present intervals
      IlcIntervalVar p =
        ls.getOneLaterInHead(ls.getLatestPresentInHead());
      if (MinStartDomainPrice(p, ls, minsmin, minsmax)) {
        minsmin = p.getStartMin();
        minsmax = IlcMin(p.getStartMax(), minsmax);
        s = ls;
        var = p;
      }
      continue; // the soonest not present candidate head stop for this sequence
    }
    IlcBool seen = IloFalse;
    for(IlcIntervalSequenceVar::Iterator
        it(ls, IlcIntervalSequenceVar::CandidateHead);
        it.ok(); ++it) {
      // 2nd priority: candidate head
      seen = IloTrue;
      IlcIntervalVar p = *it;
      if (MinStartDomainPrice(p, ls, minsmin, minsmax)) {
        minsmin = p.getStartMin();
        minsmax = IlcMin(p.getStartMax(), minsmax);
        s = ls;
        var = p;
      }
    }
    if (seen)
      continue; // a not sequenced candidate head
    if (ls.getLatestInTail().getImpl() !=
        (ls.getLatestPresentInTail().getImpl())) {
      for(IlcIntervalSequenceVar::Iterator
            itt(ls, IlcIntervalSequenceVar::Tail,
                ls.getLatestPresentInTail());
          itt.ok(); --itt) {
        // Lowest priority: sequenced in tail but not present intervals
        IlcIntervalVar p = *itt;
        if (ls.isPresent(p))
          continue;
        if (MinStartDomainPrice(p, ls, minsmin, minsmax)) {
          minsmin = p.getStartMin();
          minsmax = IlcMin(p.getStartMax(), minsmax);
          s = ls;
          var = p;
          break;
        }
      }
    }
  }
  return (minsmin <= IloIntervalMax);
}

void
SequenceHeadSearch::complete(IloInt boundary) {
  // boundary > IloIntervalMax means fixing all sequences
  IlcBool fixed = (boundary > IloIntervalMax);
  if (fixed) 
    _makespan.setMax(_makespan.getMin());
  IloInt size = _sequences.getSize();
  for(IloInt i = 0; i < size; ++i) {
    IlcIntervalSequenceVar ls = _sequences[i];
    assert(!fixed || ls.isFixed()); 
    if (fixed) { // see assert: the sequence is fixed 
      // sequencing terminated, schedule the intervals in tail
      for(IlcIntervalSequenceVar::Iterator
            it1(ls, IlcIntervalSequenceVar::Tail);
          it1.ok(); ++it1) {
        IlcIntervalVar p = *it1;
        if (p.isFixed())
          continue;
        p.setStart(p.getStartMin());
      }
    }
    // Partial schedule of the interval in head
    for(IlcIntervalSequenceVar::Iterator
          it2(ls, IlcIntervalSequenceVar::Head);
        it2.ok(); ++it2) {
      IlcIntervalVar p = *it2;
      if (p.isFixed())
        continue;
      IloInt smin = p.getStartMin();
      if (!(fixed || (p.isPresent() && (boundary > smin))))
        break; // end of fully fixed head
      p.setStart(smin);
    }
  }
  _boundary.setValue(getCPEngine(), boundary);
}

ILCGOAL1(SetPresentGoal, IlcIntervalVar, var) {
  var.setPresent();
  return 0;
}

ILCGOAL1(SetAbsentGoal, IlcIntervalVar, var) {
  var.setAbsent();
  return 0;
}

ILCGOAL1(SequenceHeadSearchGoal, SequenceHeadSearch*, r) {
  IlcIntervalVar var;
  IlcIntervalSequenceVar s;
  if (r->select(s, var)) {
    // scheduling interleaving:
    // every time the boundary of the schedule shifts of
    // 20 units, we schedule the sequenced in head intervals
    IloInt boundary = r->getBoundary() + 20;
    if (var.getStartMin() >= boundary) {
      r->complete(boundary);
      return this;
    }
    IloCPEngine cp = getCPEngine();
    IlcGoal g;
    if (s.isSequenced(var)) // sequenced but not fixed
      g = IlcOr(SetPresentGoal(cp, var),
                SetAbsentGoal(cp, var));
    else if (!var.isPresent()) // candidate head not surely present
      g = IlcOr(IlcAnd(SetPresentGoal(cp, var),
                       s.tryExtendHead(var)),
                SetAbsentGoal(cp, var));
    else // candidate head surely present
      g = s.tryExtendHead(var);
    return IlcAnd(g, this);
  }
  r->complete();
  return 0;
}

IlcGoal SequenceHeadSearch::makeGoal(IloCPEngine cp) {
  return SequenceHeadSearchGoal(cp, this);
}

///////////////////////////////////////////////////////////////////////////////
//
// SOLVING : WRAPPER MODEL ENGINE  
//
///////////////////////////////////////////////////////////////////////////////

class JSSPFLEXScheduler {
private:
  IloIntervalSequenceVarArray _sequences;
  IloIntVar _makespan;
public:
  JSSPFLEXScheduler(IloIntervalSequenceVarArray sequences,
                    IloIntVar makespan)
    :_sequences(sequences),_makespan(makespan) {}
  IloIntervalSequenceVarArray getSequences() const { return _sequences;}
  IloIntVar getMakespan() const { return _makespan;}
  IlcGoal initSearch(IloCPEngine cp);
};

IlcGoal JSSPFLEXScheduler::initSearch(IloCPEngine cp) {
  SequenceHeadSearch* r = new (cp.getHeap())
    SequenceHeadSearch(cp, getSequences(), getMakespan());
  return r->makeGoal(cp);
}

ILOCPGOALWRAPPER1(JSSPFLEXSchedulerGoal, cp, JSSPFLEXScheduler*, s) {
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
    const char* filename = "../../../examples/data/jobshopflex_default.data";
    IloInt mode = 0;
    IloInt failLimit = 5000;
    if (argc > 1) 
      mode = atoi(argv[1]);
    if (argc > 2)
      filename = argv[2];
    if (argc > 3)
      failLimit = atoi(argv[3]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    for (IloInt j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbJobs; i++) {
      IloInt nbOperations;
      file >> nbOperations;
      IloIntervalVar prec;
      for (IloInt j = 0; j < nbOperations; j++) {
        IloInt nbOpMachines, k;
        file >> nbOpMachines;
        IloIntervalVar master(env);
        IloIntervalVarArray members(env);
        for (k = 0; k < nbOpMachines; k++) {
          IloInt m, d;
          file >> m;
          file >> d;
          IloIntervalVar member(env, d);
          member.setOptional();
          members.add(member);
          machines[m-1].add(member);
        }
        model.add(IloAlternative(env, master, members));
        if (0 != prec.getImpl())
          model.add(IloEndBeforeStart(env, prec, master));
        prec = master;
      }
      ends.add(IloEndOf(prec));
    }

    IloIntervalSequenceVarArray sequences(env);
    for (IloInt j = 0; j < nbMachines; j++) {
      IloIntervalSequenceVar s(env, machines[j]);
      sequences.add(s);
      model.add(IloNoOverlap(env, s));
    }
    IloIntVar makespan(env);
    model.add(makespan == IloMax(ends));
    IloObjective objective = IloMinimize(env,makespan);
    model.add(objective);
    
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.out() << "Instance \t: " << filename << std::endl;
    IlcBool solved;
    if (mode) {
      std::cout << "+++++ Automatic Search +++++" << std::endl;
      cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
      solved = cp.solve();
    } else {
      std::cout << "+++++ Hail Extension Search +++++" << std::endl;
      JSSPFLEXScheduler* jssp =
        new (env) JSSPFLEXScheduler(sequences, makespan);
      IloGoal g = JSSPFLEXSchedulerGoal(env, jssp);
      solved = cp.solve(g);
    }
    if (solved) {
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
