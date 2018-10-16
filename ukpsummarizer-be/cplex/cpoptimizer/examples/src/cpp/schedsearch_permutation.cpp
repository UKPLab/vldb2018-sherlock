// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/schedsearch_permutation.cpp
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

#include <ilcp/cpext.h>


class SamePermutationCtI : public IlcConstraintI {
  /*
   *
   * Rationale.
   *
   * The Same Permutation constraint states that two sequence 
   * variables are synchronized for the present intervals. 
   * That is, given a bijection between the interval variables 
   * of the two sequences, the restriction of the pairwise 
   * present interval subsequences are the in the same order
   * (follows the same permutation).
   *
   * Applications, which may require some adjustments in the
   * semantic of the constraint,include Permutation Flow Shop 
   * (academic), devices like conveyor belt (transportation) 
   * and Material Flow Synchronization 
   * (manufacturing or supply chain).
   *
   * More formally the constraint states:
   * Let s1 and s2 two sequence variables and B a bijection
   * between the sets of intervals of the two sequences.
   * let x and y be interval variables of s1, B(x), B(y) the
   * two intervals of s2 corresponding by B to x and y,
   * if x, y, B(x), and  B(y) are all present, then
   * x is before y in s1 if and only B(x) is before B(y) in s2
   *
   * We design the implementation in a way that 
   * applies for every constraint on or between sequences.
   *
   *
   * Filtering algorithm
   *
   * The constraint is based on the domain and events of
   * the head-tail graph of the precedence graph of a sequence.
   *
   * The idea of the filtering algorithm is the following:
   *   1) The head part is synchronized with the condition to satisfy. 
   * In other words, a filtering function maintains an interval
   * variable in the head such that the condition cannot be violated 
   * regardless of the changes in the earlier subpart of the head.
   *      1-1) The condition of the constraint at the boundary allows to 
   *          - deduce not candidate head intervals,
   *          - eventually to extend the head, 
   *          - to connect head and tail when the sequence is sequenced 
   *          - and, finally, to deduce presence incompatibilities between 
   *            set of intervals.
   *    This filtering algorithm is sound for the constraint.
   *      1-2)  In case of an interval becomes absent, it is 
   * removed from the head. If this interval is the boundary of 
   * synchronization, we move this boundary back.
   *      1-3) For the part of the head that is after the boundary 
   * of synchronization, a filtering algorithm deals with the
   * neighborhood of the changed interval variables in the head.
   * For sake of simplicity and keeping a O(1) update in the
   * propagation algorithm, we only consider the one earlier  
   * and one later intervals in the head.
   * 
   *   2) A similar filtering algorithm is written for the tail.
   *
   *   3) If the tails (resp. heads) are fully synchronized, the
   * candidate tail (resp. head) sets are the same for the
   * two sequences. We implement a filtering algorithm for
   * not sequenced intervals that synchronizes not candidate
   * head (respectively not candidate tail)
   *
   * The 1) to 2) filtering algorithms are used to implement the 
   * events of head/tail extensions and presence change of an 
   * interval. There are in amortized O(1) in cpu and memory for 
   * the change of one interval in the head-tail graph.
   *
   * The "not sequenced" algorithm requires the complete
   * head/tail synchronization to be done and is O(n) in cpu time. 
   * The whenNotSequenced event of the sequence variable is used
   * for that purpose.
   *
   * For any of your constraints that involve sequences, we
   * invite you to follow the same design practices.
   *
   *
   *
   * Dealing with logical relation between presence of intervals
   *
   * An absent interval has no impact on the domain of the other
   * intervals. That means that its position in the sequence is
   * unimportant and if an interval of a pair is absent,
   * the other interval necesseraly enforces the constraint
   * condition: if an interval is absent in a solution, all formal 
   * sequences including this interval are the same solution.
   * Moreover, suppose you know the two intervals have the same 
   * presence value. Either they are present and positionned at the 
   * same place in the solution or absent and, by symetry breaking, 
   * can be positionned where there would be if present in the solution. 
   * Consequently, the propagation rules applies for pairwise same presence 
   * intervals in bijection whatever is their actual presence status.
   *
   * Example Given : Part of the deductions consists in finding subsets of 
   * intervals that cannot be all present in a solution. Let x, y and z be 
   * three different interval variables of s1, and B(x), B(y) and B(z)
   * be their couterparts in s2. Suppose you have a subsequence x->y in the 
   * head of sequence 1 and B(x)->B(z) in the head of  sequence 2. At least 
   * one interval among of the sextuplet (x, B(x), y, B(y), z, B(z)) is 
   * absent in a solution. Suppose now you know that x (resp. y, z) and B(x) 
   * (resp. B(y), B(z)) have the same presence value, then one of intervals 
   * of the triplet (x, y, z) is absent in a solution. 
   *
   * Two cases may occur: 
   *      All intervals in the bijection have the same presence value. The
   * sequences are fully synchronized. This allows strong deduction like head/tail 
   * extension and a full synchronization of the not sequenced interval variables.
   *      Only the restriction of same presence value intervals in the solution,
   * and in particular in the model, is constrained. This only allows to deduce 
   * not candidate head and tail for these kind of intervals.
   *
   * Please refer to extendHead, extendTail and propagateNotSequenced
   * functions in this file to see the implementation.
   *
   * The symmetry breaking by locally adding constraints during
   * propagation is a typical application of the usage of dominance rules in
   * the optimizer engine.
   *
   * In practice, having the same or different presence values for a
   * pair of interval variables in bijection may be possible:
   * 1) In a conveyor belt, the intervals are the loading and the
   * unloading of an item transportation: start
   * and end intervals have the same presence value.
   * 2) In material flow, the intervals that models the same
   * recipe of a production order on the different stages of the process 
   * have not necesarelythe same presence value if there is an alternative
   * of machines to execute a task in a stage of the process.
   *
   *
   * Summary
   *
   * This sample shows the usage of the Head-Tail graph as
   * computational instantiation of a sequence variable in
   * the CP Optimizer search engine for expressing new
   * constraints. It shows:
   *
   *     the usage of incremental invariant (the frontier of anytime 
   * true condition on head/tail subsequences) to speed up the filtering 
   * algorithm.
   *     the conjunctive usage of the logical relations 
   * between presence values of intervals to improve a 
   * filtering algorithm.
   *     how to extend the scheduling modeling for interval sequence
   * variables in the CP Optimizer for an efficient description of complex 
   * machines or flow of materials.
   * 
   */
private:
  // sequences
  IlcIntervalSequenceVar _s1;
  IlcIntervalSequenceVar _s2;
  // array of intervals and same presence relations
  IlcIntervalVarArray _intervals1;
  IlcIntervalVarArray _intervals2;
  IlcIntArray _samePresences;
  // index of tail/head synchronized sub sequences
  IlcRevInt _markSyncHead1;
  IlcRevInt _markSyncTail1;
  IlcRevInt _markSyncHead2;
  IlcRevInt _markSyncTail2;
private:
  // incremental supports
  
  IlcIntervalVar getHeadInterval1() const {
    return (_markSyncHead1.getValue() >= 0)
      ? _intervals1[_markSyncHead1.getValue()] : 0;
  }
  IlcIntervalVar getHeadInterval2() const {
    return (_markSyncHead2.getValue() >= 0)
      ? _intervals2[_markSyncHead2.getValue()] : 0;
  }
  IlcIntervalVar getTailInterval1() const {
    return (_markSyncTail1.getValue() >= 0)
      ? _intervals1[_markSyncTail1.getValue()] : 0;
  }
  IlcIntervalVar getTailInterval2() const {
    return (_markSyncTail2.getValue() >= 0)
      ? _intervals2[_markSyncTail2.getValue()] : 0;
  }
  
  IlcIntervalVar
  getHeadInterval(IlcIntervalSequenceVar s) const {
    return (s.getImpl() == _s1.getImpl())
      ? getHeadInterval1() : getHeadInterval2();
  }
  IlcIntervalVar
  getTailInterval(IlcIntervalSequenceVar s) const {
    return (s.getImpl() == _s1.getImpl())
      ? getTailInterval1() : getTailInterval2();
  }

  // filtering algorithms
  void skipOne(IlcIntervalSequenceVar s1,
               IlcIntervalVar ti1,
               IlcIntervalSequenceVar s2,
               IlcIntervalVar ti2) const;
  void skipOneFrom(IlcIntervalSequenceVar s,
                   IlcIntervalVar ti,
                   IlcIntervalSequenceVar s1,
                   IlcIntervalVar ti1,
                   IlcIntervalSequenceVar s2,
                   IlcIntervalVar ti2) const;
  void moveHeadEarlier();
  void extendHead(IlcIntervalSequenceVar s1, IlcIntervalVar ti1);
  void syncHead();
  void moveTailEarlier();
  void extendTail(IlcIntervalSequenceVar s1, IlcIntervalVar ti1);  
  void syncTail();
  void checkHeadNeighbour(IlcIntervalSequenceVar s, IloInt index);
  void checkTailNeighbour(IlcIntervalSequenceVar s, IloInt index);
  void propagateHead(IlcIntervalSequenceVar sequence, IlcIntervalVar from);
  void propagateTail(IlcIntervalSequenceVar sequence, IlcIntervalVar from);
  void propagatePresence(IlcIntervalSequenceVar sequence, IlcIntervalVar var);
  void checkPartialChain(IlcIntervalVar v1,
                         IlcIntervalVar earl1,
                         IlcIntervalVar latr1,
                         IlcIntervalSequenceVar s1,
                         IlcIntervalVar v2,
                         IlcIntervalVar earl2,
                         IlcIntervalVar latr2,
                         IlcIntervalSequenceVar s2) const;
  void propagateNotSequenced(IlcIntervalSequenceVar s);
public:
  // By default, if samePresences is a NULL handle, then any
  // associated intervals have the same presence value
  SamePermutationCtI(IloCPEngine cp,
                     IlcIntervalSequenceVar s1,
                     IlcIntervalSequenceVar s2,
                     IlcIntervalVarArray intervals1,
                     IlcIntervalVarArray intervals2,
                     IlcIntArray samePresences);
  
  static IloInt getIndex(IlcIntervalVar var) {
    // for sake of simplicity we use the object field
    // of an interval. This could not be so easy in a
    // real application
    return (var.getImpl()) ?(IloInt) var.getObject() : - 1;
  }
  IlcIntervalSequenceVar getSequence1() const {
    return _s1;
  }
  IlcIntervalSequenceVar getSequence2() const {
    return _s2;
  }
  IlcIntervalVar getInterval1(IloInt index) const {
    return (index < 0) ? 0 : _intervals1[index];
  }
  IlcIntervalVar getInterval2(IloInt index) const {
    return (index < 0) ? 0 : _intervals2[index];
  }
  IlcIntervalVar getInterval(IloInt index, IlcIntervalSequenceVar s) const {
    return (index < 0)
      ? 0
      : ((s.getImpl() == _s1.getImpl())
        ? _intervals1[index]
        : _intervals2[index]);
  }
  IlcIntervalSequenceVar
  getOtherSequence(IlcIntervalSequenceVar s) const {
    return (s.getImpl() == _s1.getImpl()) ? _s2 : _s1;
  }
  IloBool isSamePresence(IloInt index) const {
    return (!_samePresences.getImpl() || (index < 0))
      ? IloTrue : _samePresences[index];
  }
  
  IloBool allSamePresence() const {
    return !_samePresences.getImpl();
  }
  
  // constraint interface
  virtual void post();
  virtual void propagate();
  virtual void display(ILOSTD(ostream) &) const;
  
  // propagation demons
  void propagateDeltaHead1() {
    IlcIntervalSequenceVar sequence = getSequence1();
    propagateHead(sequence, sequence.getEarliestNewInHead());
  }
  void propagateDeltaHead2() {
    IlcIntervalSequenceVar sequence = getSequence2();
    propagateHead(sequence, sequence.getEarliestNewInHead());
  }
  void propagateDeltaTail1()  {
    IlcIntervalSequenceVar sequence = getSequence1();
    propagateTail(sequence, sequence.getEarliestNewInTail());
  }
  void propagateDeltaTail2() {
    IlcIntervalSequenceVar sequence = getSequence2();
    propagateTail(sequence, sequence.getEarliestNewInTail());
  }
  void propagateDeltaPresence1() {
    IlcIntervalSequenceVar sequence = getSequence1();
    propagatePresence(sequence, sequence.getDeltaPresence());
  }
  void propagateDeltaPresence2() {
    IlcIntervalSequenceVar sequence = getSequence2();
    propagatePresence(sequence, sequence.getDeltaPresence());
  }
  void propagateNotSequenced1() {
    IlcIntervalSequenceVar sequence = getSequence1();
    propagateNotSequenced(sequence);
  }
  void propagateNotSequenced2() {
    IlcIntervalSequenceVar sequence = getSequence2();
    propagateNotSequenced(sequence);
  }
};

#ifndef NDEBUG
void ValidateArgs(IlcIntervalSequenceVar s1,
                  IlcIntervalSequenceVar s2,
                  IlcIntervalVarArray intervals1,
                  IlcIntervalVarArray intervals2,
                  IlcIntArray samePresences) {
  IloInt size = intervals1.getSize();
  IloAssert(size == intervals2.getSize(),
            "ValidateArgs: Not Same Size  Intervals Arrays");
  if (samePresences.getImpl()) 
    IloAssert(size == samePresences.getSize(),
              "ValidateArgs: Same Presences Array Bad Size");
  IloInt i;
  for(i = 0; i < size; ++i) {
    IloAssert(s1.isIn(intervals1[i]),
              "ValidateArgs: Wrong Interval Variable");
    IloAssert(s2.isIn(intervals2[i]),
              "ValidateArgs: Wrong Interval Variable");
  }
  for(i = 0; i < size; ++i) {
    for(IloInt j = i + 1; j < size; ++j) {
      IloAssert(intervals1[i].getImpl() != intervals1[j].getImpl(),
                "ValidateArgs: Intervals must all be different");
      IloAssert(intervals2[i].getImpl() != intervals2[j].getImpl(),
                "ValidateArgs: Intervals must all be different");
      
    }
  }
}
#endif

SamePermutationCtI::SamePermutationCtI
(IloCPEngine cp,
 IlcIntervalSequenceVar s1,
 IlcIntervalSequenceVar s2,
 IlcIntervalVarArray intervals1,
 IlcIntervalVarArray intervals2,
 IlcIntArray samePresences)
  :IlcConstraintI(cp)
  ,_s1(s1)
  ,_s2(s2)
  ,_intervals1(intervals1)
  ,_intervals2(intervals2)
  ,_samePresences(samePresences)
  ,_markSyncHead1(cp, -1)
  ,_markSyncTail1(cp, -1)
  ,_markSyncHead2(cp, -1)
  ,_markSyncTail2(cp, -1)
{
#ifndef NDEBUG
  ValidateArgs(s1, s2, intervals1, intervals2, samePresences);
#endif
  IloInt size = intervals1.getSize();
  IloInt i;
  for(i = 0; i < size; ++i) {
    intervals1[i].setObject((IloAny) i);
    intervals2[i].setObject((IloAny) i);
  }
  if (_samePresences.getImpl()) {
    size = samePresences.getSize();
    IloBool allSamePresences = IloTrue;
    for(i = 0; i < size; ++i) {
      if (_samePresences[i])
        continue;
      _samePresences[i] =
        (intervals1[i].getImpl() == intervals2[i].getImpl()) ||
        (s1.isPresent(intervals1[i]) && s2.isPresent(intervals2[i])) ||
        (s1.isAbsent(intervals1[i]) && s2.isAbsent(intervals2[i]));
      if (!_samePresences[i])
        allSamePresences = IloFalse;
    }
    if (allSamePresences)
      _samePresences = 0;
  }
}

ILCCTDEMON0(DeltaHead1Demon,
  SamePermutationCtI, propagateDeltaHead1)
ILCCTDEMON0(DeltaHead2Demon,
  SamePermutationCtI, propagateDeltaHead2)
ILCCTDEMON0(DeltaTail1Demon,
  SamePermutationCtI, propagateDeltaTail1)
ILCCTDEMON0(DeltaTail2Demon,
  SamePermutationCtI, propagateDeltaTail2)
ILCCTDEMON0(DeltaPresence1Demon,
  SamePermutationCtI, propagateDeltaPresence1)
ILCCTDEMON0(DeltaPresence2Demon,
  SamePermutationCtI, propagateDeltaPresence2)
ILCCTDEMON0(NotSequenced1Demon,
  SamePermutationCtI, propagateNotSequenced1)
ILCCTDEMON0(NotSequenced2Demon,
  SamePermutationCtI, propagateNotSequenced2)


void BothNotPresent(IlcIntervalVar a1,
                    IlcIntervalSequenceVar s1,
                    IlcIntervalVar a2,
                    IlcIntervalSequenceVar s2) {
  // a1 and a2 can eventually be NULL handle interval for
  // they can be the source of the head or the tail
  if (s1.isPresent(a1)) {
    s2.setAbsent(a2);
  } else if (!s1.isAbsent(a1)) {
    if (s2.isPresent(a2)) {
      s1.setAbsent(a1);
    } else if (!s2.isAbsent(a2)) {
      // this function requires a1 and a2 are
      // not null handle
      IlcPresenceImplyNot(a1, a2);
    }
  }
}

void AllNotPresent(IlcIntervalVar a1,
                   IlcIntervalSequenceVar s1,
                   IlcIntervalVar a2,
                   IlcIntervalSequenceVar s2,
                   IlcIntervalVar a3,
                   IlcIntervalSequenceVar s3) {
  // this function tells that at least one of the three
  // intervals is absent.
  if (s1.isPresent(a1))
    BothNotPresent(a2, s2, a3, s3);
  else if (!s1.isAbsent(a1)) {
    if (s2.isPresent(a2))
      BothNotPresent(a1, s1, a3, s3);
    else if (!s2.isAbsent(a2) && s3.isPresent(a3))
      BothNotPresent(a1, s1, a2, s2);
  }
}

void AllNotPresent(IlcIntervalVar a1,
                   IlcIntervalSequenceVar s1,
                   IlcIntervalVar a2,
                   IlcIntervalSequenceVar s2,
                   IlcIntervalVar a3,
                   IlcIntervalSequenceVar s3,
                   IlcIntervalVar a4,
                   IlcIntervalSequenceVar s4) {
  // this function tells that at least one of the foor
  // intervals is absent.
  if (s1.isPresent(a1))
    AllNotPresent(a2, s2, a3, s3, a4, s4);
  else if (!s1.isAbsent(a1)) {
    if (s2.isPresent(a2))
      AllNotPresent(a1, s1, a3, s3, a4, s4);
    else if (!s2.isAbsent(a2)) {
      if (s3.isPresent(a3))
        AllNotPresent(a1, s1, a2, s2, a4, s4);
      else if (!s3.isAbsent(a3) && s4.isPresent(a4))
        AllNotPresent(a1, s1, a2, s2, a3, s3);;
    }
  }
}

void
SamePermutationCtI::skipOne(IlcIntervalSequenceVar s1,
                            IlcIntervalVar ti1,
                            IlcIntervalSequenceVar s2,
                            IlcIntervalVar ti2) const {
  // an interval is skipped if it is absent or its counterpart 
  // is absent 
  IloInt index1 = getIndex(ti1);
  IloInt index2 = getIndex(ti2);
  if (allSamePresence())
    BothNotPresent(ti1, s1, ti2, s2);
  else if (isSamePresence(index1)) {
    if (isSamePresence(index2)) {
      BothNotPresent(ti1, s1, ti2, s2);
    } else {
      IlcIntervalSequenceVar s3 = getOtherSequence(s2);
      IlcIntervalVar v3 = getInterval(index2, s3);
      AllNotPresent(ti1, s1, ti2, s2, v3, s3);
    }
  } else if (isSamePresence(index2)) {
    IlcIntervalSequenceVar s3 = getOtherSequence(s1);
    IlcIntervalVar v3 = getInterval(index1, s3);
    AllNotPresent(ti1, s1, ti2, s2, v3, s3);
  } else {
    IlcIntervalSequenceVar s3 = getOtherSequence(s1);
    IlcIntervalVar v3 = getInterval(index1, s3);
    IlcIntervalSequenceVar s4 = getOtherSequence(s2);
    IlcIntervalVar v4 = getInterval(index2, s4);
    AllNotPresent(ti1, s1, ti2, s2, v3, s3, v4, s4);
  }
}

/////////////////////////////////////////////////////////////
//
//
// Filtering Algorithm for Head Boundary Maintenance And Extension
//
//
////////////////////////////////////////////////////////////

void
SamePermutationCtI::extendHead(IlcIntervalSequenceVar s1,
                               IlcIntervalVar ti1) {
  // Boundary of s1 head is the one earlier ti1
  // and all head of s2 is traversed
  IlcIntervalSequenceVar s2 = getOtherSequence(s1);
  if (s2.isSequenced()) {
    // We try to join the head and tail of sequence 2
    // by following sequence 1 from ti1 and
    // traversing the tail until a contradiction is found. 
    // The tail traversal stops before the tail boundary
    IlcIntervalVar ti2 = s2.getLatestInTail();
    if (!ti2.getImpl()) { // empty tail is not allowed
      // this test is required as we traverse to earlier the 
      // tail: the iterator must be initialized with a not NULL 
      // handle interval variable.
      skipOne(s1, ti1, s2, 0);
      return;
    }
    IlcIntervalSequenceVar::Iterator
      ite1(s1, IlcIntervalSequenceVar::Head, ti1);
    IlcIntervalSequenceVar::Iterator
      ite2(s2, IlcIntervalSequenceVar::Tail, ti2);
    while(ite1.ok()) {
      ti1 = *ite1;
      IlcIntervalVar o1 = getInterval(getIndex(ti1), s2);
      ti2 = 0;
      if (ite2.ok()) {
        // cases for which ti1 or ti2 must be skipped
        ti2 = *ite2;
        IlcIntervalVar o2 = getInterval(getIndex(ti2), s1);
        if (s1.isAbsent(ti1) || s2.isAbsent(o1)) {
          ++ite1;
          continue;
        } else if (s2.isAbsent(ti2) || s1.isAbsent(o2)) {
          --ite2;
          continue;
        }
      }
      if (o1.getImpl() == ti2.getImpl()) {
        ++ite1;
        --ite2;
      } else { // contradiction
        skipOne(s1, ti1, s2, ti2);
        return;
      }
    }
  } else if (allSamePresence()) {
    // symetry breaking : an absent interval can be
    // everywhere in the sequence: we choose to
    // break the symetry by telling it is sequenced at the same
    // position as it were present
    // 
    // Simple Case: ti2 is always at the same position than ti1
    // in the sequence we can extend the head by ti2
    IlcIntervalVar ti2 = getInterval(getIndex(ti1), s2);
    if (s2.isInTail(ti2)) {
      // if ti2 in tail use of setPrevious member function
      // to join head and tail
      IlcIntervalVar prev1 = s1.getOneEarlierInHead(ti1);
      IlcIntervalVar prev2 = getInterval(getIndex(prev1), s2);
      s2.setPrevious(prev2, ti2);
    } else // regular case
      s2.extendHead(ti2);
  } else {
    // symetry breaking : an absent interval can be
    // everywhere in the sequence: we choose to
    // break the symetry by telling it is sequenced at the same
    // position as it were present.
    //
    // generic case:
    // sequence 2 can not be extended by an interval whose
    // counterpart in sequence 1 is of same presence
    IloInt index1 = getIndex(ti1);
    if (isSamePresence(index1) || s1.isPresent(ti1)) {
      IlcIntervalVar ti2 = getInterval(index1, s2);
      // ti2 is not absent
      // so, ignoring absent interval, ti2 is at the same
      // position than ti1. So all interval in head of s2
      // and of same presence must be remove from head
      for(IlcIntervalSequenceVar::Iterator
          it(s2, IlcIntervalSequenceVar::CandidateHead);
          it.ok(); ++it) {
        IlcIntervalVar var = *it;
        if ((var.getImpl() != ti2.getImpl()) && isSamePresence(getIndex(var)))
          s2.removeCandidateHead(var);
      }
    }
  }
}

void SamePermutationCtI::moveHeadEarlier() {
  // This function move earlier the head frontier of 
  // synchronisation in case of absent interval.
  IloCPEngine cp = getCPEngine();
  IlcIntervalSequenceVar s1 = getSequence1();
  IlcIntervalSequenceVar s2 = getSequence2();
  IlcIntervalVar head = getHeadInterval1();
  if (s1.isAbsent(head)) {
    IloInt mark = -1;
    for(IlcIntervalSequenceVar::Iterator
          ite1(s1, IlcIntervalSequenceVar::Head, head);
        ite1.ok(); --ite1) {
      // Support must move earlier until a non absent interval.
      // An absent interval in the sequence will disappear of
      // the head/tail
      IlcIntervalVar ti1 = *ite1;
      if (!s1.isAbsent(ti1)) {
        mark = getIndex(ti1);
        break;
      }
    }
    _markSyncHead1.setValue(cp, mark);
  }
  head = getHeadInterval2();
  if (s2.isAbsent(head)) {
    IloInt mark = -1;
    for(IlcIntervalSequenceVar::Iterator
          ite2(s2, IlcIntervalSequenceVar::Head, head);
        ite2.ok(); --ite2) {
      // Support must move earlier until a non absent interval.
      // An absent interval in the sequence will disappear of
      // the head/tail
      IlcIntervalVar ti2 = *ite2;
      if (!s2.isAbsent(ti2)) {
        mark = getIndex(ti2);
        break;
      }
    }
    _markSyncHead1.setValue(cp, mark);
  }
}

void SamePermutationCtI::syncHead() {
  // This function filters the absolute position of the interval
  // in head. It maintains a incremental mark on the latest 
  // interval such that the condition anytime holds.
  //
  // If the head boundary is absent, the boundary moves earlier.
  // See member function moveHeadEarlier
  //
  // The algorithm stops when the condition is not
  // surely enforced. There is two cases:
  //
  //   When the two heads locally violate the condition,
  //   The function tells that the intervals cannot be both
  //   present. See member skipOne.
  //
  //   If the latest interval of one sequence is not 
  //   synchronized, the other sequence head is extended. 
  //   See member function extendHead
  //
  moveHeadEarlier();
  IloCPEngine cp = getCPEngine();
  IlcIntervalSequenceVar s1 = getSequence1();
  IlcIntervalSequenceVar s2 = getSequence2();
  IloInt nindex1 = _markSyncHead1.getValue();
  IloInt nindex2 = _markSyncHead2.getValue();
  IlcIntervalSequenceVar::Iterator
    ite1(s1, IlcIntervalSequenceVar::Head, getHeadInterval1());
  IlcIntervalSequenceVar::Iterator
    ite2(s2, IlcIntervalSequenceVar::Head, getHeadInterval2());  
  if (getHeadInterval1().getImpl())
    ++ite1;
  if (getHeadInterval2().getImpl())
    ++ite2;
  while(ite1.ok() || ite2.ok()) {
    IlcIntervalVar ti1;
    IlcIntervalVar ti2;
    IloInt index1 = IloIntMax;
    IloInt index2 = IloIntMax;
    if (ite1.ok()) {
     ti1 = *ite1;
     index1 = getIndex(ti1);
     ++ite1;
     if (s1.isAbsent(ti1)) //skipping absent interval
       continue;
     else if (s2.isAbsent(getInterval2(index1))) {
       //if an interval of the pair is absent,
       //the condition is enforced
       nindex1 = index1;
       continue;
      }
    }
    if (ite2.ok()) {
      ti2 = *ite2;
      index2 = getIndex(ti2);
      ++ite2;
      if (s2.isAbsent(ti2)) //skipping absent interval
        continue;
      else if (s1.isAbsent(getInterval1(index2))) {
       //if an interval of the pair is absent,
       //the condition is enforced
        nindex2 = index2;
        continue;
      }
    }
    if (ti1.getImpl() && ti2.getImpl()) {
      if (index1 == index2) { // condition enforced
        nindex1 = index1;
        nindex2 = index2;
        continue;
      }
      skipOne(s1, ti1, s2, ti2);
    } else if (ti1.getImpl()) {
      extendHead(s1, ti1);
    } else if (ti2.getImpl()) {
      extendHead(s2, ti2);
    }
    break;
  }
  _markSyncHead1.setValue(cp, nindex1);
  _markSyncHead2.setValue(cp, nindex2);
}

/////////////////////////////////////////////////////////////
//
//
// Filtering Algorithm for Tail extension
//
//
////////////////////////////////////////////////////////////

void
SamePermutationCtI::extendTail(IlcIntervalSequenceVar s1,
                               IlcIntervalVar ti1) {
  // boundary of s1 tail support is the one earlier ti1
  // and all tail of s2 is traversed
  IlcIntervalSequenceVar s2 = getOtherSequence(s1);
  if (s2.isSequenced()) {
    // we try to join the head and tail of sequence 1
    // by following sequence 1 from ti1.
    // traverse the head until a contradiction is found. 
    // the head traversal stops at the head boundary
    IlcIntervalVar ti2 = s2.getLatestInHead();
    if (!ti2.getImpl()) { // empty head is not allowed
      // this test is required as we traverse to earlier the 
      // tail: the iterator must be initialized with a not NULL 
      // handle interval variable.
      skipOne(s1, ti1, s2, 0);
      return;
    }
    IlcIntervalSequenceVar::Iterator
      ite1(s1, IlcIntervalSequenceVar::Tail, ti1);
    IlcIntervalSequenceVar::Iterator
      ite2(s2, IlcIntervalSequenceVar::Head, ti2);
    while(ite1.ok()) {
      ti1 = *ite1;
      IlcIntervalVar o1 = getInterval(getIndex(ti1), s2);
      ti2 = 0;
      if (ite2.ok()) {
        // cases for which ti1 or ti2 must be skipped
        ti2 = *ite2;
        IlcIntervalVar o2 = getInterval(getIndex(ti2), s1);
        if (s1.isAbsent(ti1) || s2.isAbsent(o1)) {
          ++ite1;
          continue;
        } else if (s2.isAbsent(ti2) || s1.isAbsent(o2)) {
          --ite2;
          continue;
        }
      }
      if (o1.getImpl() == ti2.getImpl()) {
        ++ite1;
        --ite2;
      } else { // contradiction
        skipOne(s1, ti1, s2, ti2);
        return;
      }
    }
  } else if (allSamePresence()) {
    // symetry breaking : an absent interval can be
    // everywhere in the sequence: we choose to
    // break the symetry by telling it is sequenced at the same
    // position as it were present
    // 
    // Simple Case: ti2 is always at the same position than ti1
    // in the sequence we can extend the head by ti2
    IlcIntervalVar ti2 = getInterval(getIndex(ti1), s2);
    if (s2.isInHead(ti2)) {
      // if ti2 in tail use of setPrevious member function
      // to join tail and head.
      IlcIntervalVar next1 = s1.getOneEarlierInTail(ti1);
      IlcIntervalVar next2 = getInterval(getIndex(next1), s2);
      s2.setPrevious(ti2, next2);
    } else // regular case
      s2.extendTail(ti2);
  } else {
    // symetry breaking : an absent interval can be
    // everywhere in the sequence: we choose to
    // break the symetry by telling it sequenced at the same
    // position as it were present.
    //
    // generic case:
    // sequence 2 can not be extended by an interval whose
    // counterpart in sequence 1 is of same presence
    IloInt index1 = getIndex(ti1);
    if (isSamePresence(index1) || s1.isPresent(ti1)) {
      IlcIntervalVar ti2 = getInterval(index1, s2);
      // ti2 is not absent
      // so, ignoring absent interval, ti2 is at the same
      // position than ti1. So all interval in head of s2
      // and of same presence must be remove from tail
      for(IlcIntervalSequenceVar::Iterator
            it(s2, IlcIntervalSequenceVar::CandidateTail);
          it.ok(); ++it) {
        IlcIntervalVar var = *it;
        if ((var.getImpl() != ti2.getImpl()) &&
            isSamePresence(getIndex(var)))
          s2.removeCandidateTail(var);
      }
    }
  }
}

void SamePermutationCtI::moveTailEarlier() {
  // This function move earlier the tail frontier of 
  // synchronisation in case of absent interval.
  IloCPEngine cp = getCPEngine();
  IlcIntervalSequenceVar s1 = getSequence1();
  IlcIntervalSequenceVar s2 = getSequence2();
  IlcIntervalVar tail = getTailInterval1();
  if (s1.isAbsent(tail)) {
    // Support must move earlier until a non absent interval.
    // An absent interval in the sequence will disappear of
    // the head/tail
    IloInt mark = -1;
    for(IlcIntervalSequenceVar::Iterator
          ite1(s1, IlcIntervalSequenceVar::Tail, tail);
        ite1.ok(); --ite1) {
      IlcIntervalVar ti1 = *ite1;
      if (!s1.isAbsent(ti1)) {
        mark = getIndex(ti1);
        break;
      }
    }
    _markSyncTail1.setValue(cp, mark);
  }
  tail = getTailInterval2();
  if (s2.isAbsent(tail)) {
    // Support must move earlier until a non absent interval.
    // An absent interval in the sequence will disappear of
    // the head/tail
    IloInt mark = -1;
    for(IlcIntervalSequenceVar::Iterator
          it2(s2, IlcIntervalSequenceVar::Tail, tail);
         it2.ok(); --it2) {
      IlcIntervalVar ti2 = *it2;
      if (!s2.isAbsent(ti2)) {
        mark = getIndex(ti2);
        break;
      }
    }
    _markSyncTail2.setValue(cp, mark);
  }
}

void SamePermutationCtI::syncTail() {
  // This function filters the absolute position of the interval
  // in tail. It maintains a incremental mark on the latest 
  // interval such that the condition anytime holds.
  //
  // If the head boundary is absent, the boundary moves earlier.
  // See member function moveHeadEarlier
  //
  // The algorithm stops when the condition is not
  // surely enforced. There is two cases:
  //
  //   When the two tails locally violate the condition,
  //   The function tells that the intervals cannot be both
  //   present. See member skipOne.
  //
  //   If the latest interval of one sequence is not 
  //   synchronized, the other sequence tail is extended. 
  //   But, when the interval fits the tail sequence, we use 
  //   See member function extendTail
  //
  moveTailEarlier();
  IloCPEngine cp = getCPEngine();
  IlcIntervalSequenceVar s1 = getSequence1();
  IlcIntervalSequenceVar s2 = getSequence2();
  IloInt nindex1 = _markSyncTail1.getValue();
  IloInt nindex2 = _markSyncTail2.getValue();
  IlcIntervalSequenceVar::Iterator
    ite1(s1, IlcIntervalSequenceVar::Tail, getTailInterval1());
  IlcIntervalSequenceVar::Iterator
    ite2(s2, IlcIntervalSequenceVar::Tail, getTailInterval2());
  if (getTailInterval1().getImpl())
    ++ite1;
  if (getTailInterval2().getImpl())
    ++ite2;
  while(ite1.ok() || ite2.ok()) {
    IlcIntervalVar ti1;
    IlcIntervalVar ti2;
    IloInt index1 = IloIntMax;
    IloInt index2 = IloIntMax;
    if (ite1.ok()) {
      ti1 = *ite1;
      index1 = getIndex(ti1);
      ++ite1;
      if (s1.isAbsent(ti1)) //skipping absent interval
        continue;
      else if (s2.isAbsent(getInterval2(index1))) {
       //if an interval of the pair is absent,
       //the condition is enforced
        nindex1 = index1;
        continue;
      }
    }
    if (ite2.ok()) {
      ti2 = *ite2;
      index2 = getIndex(ti2);
      ++ite2;
      if (s2.isAbsent(ti2)) //skipping absent interval
        continue;
      else if (s1.isAbsent(getInterval1(index2))) {
       //if an interval of the pair is absent,
       //the condition is enforced
        nindex2 = index2;
        continue;
      }
    }
    if (ti1.getImpl() && ti2.getImpl()) {
      if (index1 == index2) {// condition enforced
        nindex1 = index1;
        nindex2 = index2;
        continue;
      }
      skipOne(s1, ti1, s2, ti2);
    } else if (ti1.getImpl()) {
      extendTail(s1, ti1);
    } else if (ti2.getImpl()) {
      extendTail(s2, ti2);
    }
    break;
  }
  _markSyncTail1.setValue(cp, nindex1);
  _markSyncTail2.setValue(cp, nindex2);
}

/////////////////////////////////////////////////////////////
//
//
// Filtering Algorithm for Head Neighbooring
//
//
////////////////////////////////////////////////////////////

void
SamePermutationCtI::skipOneFrom(IlcIntervalSequenceVar s,
                                IlcIntervalVar ti,
                                IlcIntervalSequenceVar s1,
                                IlcIntervalVar ti1,
                                IlcIntervalSequenceVar s2,
                                IlcIntervalVar ti2) const {
  // an interval is skipped if s or its counterpart or s1 
  // or s2 is absent 
  assert(getIndex(ti1) != getIndex(ti2));
  IloInt index = getIndex(ti);
  if (isSamePresence(index)) 
    AllNotPresent(ti, s, ti1, s1, ti2, s2);
  else {
    IlcIntervalSequenceVar so = getOtherSequence(s);
    IlcIntervalVar o = getInterval(index, so);
    AllNotPresent(ti, s, o, so, ti1, s1, ti2, s2);
  }
}

void
SamePermutationCtI::checkPartialChain(IlcIntervalVar v1,
                                      IlcIntervalVar earl1,
                                      IlcIntervalVar latr1,
                                      IlcIntervalSequenceVar s1,
                                      IlcIntervalVar v2,
                                      IlcIntervalVar earl2,
                                      IlcIntervalVar latr2,
                                      IlcIntervalSequenceVar s2)
  const {
  // We use the knowldge about logical presence equivalence
  // between intervals of same index to improve the deduction
  IloInt index = getIndex(v1);
#ifndef NDEBUG
  IloAssert(index == getIndex(v2),
            "checkPartialChain: Bad index");
#endif
  IloInt iearl1 = getIndex(earl1);
  IloInt iearl2 = getIndex(earl2);
  IloInt ilatr1 = getIndex(latr1);
  IloInt ilatr2 = getIndex(latr2);
  if ((v1.getImpl() == s1.getLatestInHead().getImpl()) ||
      (v2.getImpl() == s2.getLatestInHead().getImpl())) {
    ilatr1 = -2;
    ilatr2 = -2;
  }
  if ((v1.getImpl() == s1.getLatestInTail().getImpl()) ||
      (v2.getImpl() == s2.getLatestInTail().getImpl())) {
    iearl1 = -2;
    iearl2 = -2;
  }
  if ((ilatr1 >= -1) && (iearl1 >= -1) &&
      (iearl1 == ilatr2) && (ilatr1 == iearl2)) {
    // earl and latr are in reversed order
    skipOne(s1, earl1, s1, latr1);
    return;
  }
  IloBool same = isSamePresence(index);
  IloBool bothPresent =
    (same) ? (s1.isPresent(v1) || s2.isPresent(v2))
    : (s1.isPresent(v1) && s2.isPresent(v2));
  if (iearl1 != iearl2) {  
    if (bothPresent) {
      // incompatible chain earl1-v1 <=> earl2-v2
      skipOne(s1, earl1, s2, earl2);
    } else if ((ilatr1 >= -1) && (ilatr1 == ilatr2)) { //same latr
      // both incompatible chains
      //   not v1 and v2 present  : earl1-latr1 <=> iearl2-latr2
      //   v1 and v2 present      : earl1-v1-latr1 <=> iearl2-v2-latr2
      skipOneFrom(s1, latr1, s1, earl1, s1, earl2);
    }
  }
  if (ilatr1 != ilatr2) {
    if (bothPresent) {
      // incompatible chain v1-latr1 <=> v2-latr2
      skipOne(s1, latr1, s2, latr2);
    } else if ((iearl2 >= -1) && (iearl1 == iearl2)) { //same iearl
      // both incompatible chains
      //   not v1 and v2 present  : earl1-latr1 <=> iearl2-latr2
      //   v1 and v2 present      : earl1-v1-latr1 <=> iearl2-v2-latr2
      skipOneFrom(s2, earl1, s1, latr1, s2, latr2);
    }
  }
}

#ifndef NDEBUG
void
ValidateCheckHeadNeighbour(IlcIntervalSequenceVar s,
                           IlcIntervalVar v,
                           IlcIntervalVar head) {
  // assertion that the Neighbour checking is to be done
  IloAssert(v.getImpl(),
            "ValidateCheckHeadNeighbour: Null Handle Interval Variable");
  IloAssert(s.isIn(v),
            "ValidateCheckHeadNeighbour: Wrong Interval Variable");
  IloAssert(s.isInHead(v),
            "ValidateCheckHeadNeighbour: Interval Variable Not In Head");
  IloAssert(head.getImpl() != v.getImpl(),
            "ValidateCheckHeadNeighbour: Interval Variable Not Later Than Head Mark");  
  IloAssert(s.isEarlierInHead(head, v),
            "ValidateCheckHeadNeighbour: Interval Variable Earlier Than Head Mark");
}
#endif
           
void
SamePermutationCtI::checkHeadNeighbour(IlcIntervalSequenceVar s1,
                                       IloInt index) {
  // index is the index of an interval of s1 that is in head
  // and later than the head incremental mark
  //
  // This function allows to check the relative position of
  // the immediate neighbours of the intervals indexed by
  // index in the head of s.
  //
  // We use the knowledge about presence relation to improve
  // deductions
  // 
  IlcIntervalVar v1 = getInterval(index, s1);
#ifndef NDEBUG
  IlcIntervalVar head = getHeadInterval(s1);
  ValidateCheckHeadNeighbour(s1, v1, head);
#endif
  IlcIntervalSequenceVar s2 = getOtherSequence(s1);
  IlcIntervalVar v2 = getInterval(index, s2);
  if (s2.isInHead(v2))
    checkPartialChain(v1,
                      s1.getOneEarlierInHead(v1),
                      s1.getOneLaterInHead(v1),
                      s1,
                      v2,
                      s2.getOneEarlierInHead(v2),
                      s2.getOneLaterInHead(v2),
                      s2);
}

/////////////////////////////////////////////////////////////
//
//
// Filtering Algorithm for Tail Neighbooring
//
//
////////////////////////////////////////////////////////////

#ifndef NDEBUG
void
ValidateCheckTailNeighbour(IlcIntervalSequenceVar s,
                           IlcIntervalVar v,
                           IlcIntervalVar tail) {
  // assertion that the Neihbour checking is to be done
  IloAssert(v.getImpl(),
            "ValidateCheckTailNeighbour: Null Handle Interval Variable");
  IloAssert(s.isIn(v),
            "ValidateCheckTailNeighbour: Wrong Interval Variable");
  IloAssert(s.isInTail(v),
            "ValidateCheckTailNeighbour: Interval Variable Not In Tail");
  IloAssert(tail.getImpl() != v.getImpl(),
            "ValidateCheckTailNeighbour: Interval Variable Not Later Than Tail Mark");  
  IloAssert(s.isEarlierInTail(tail, v),
            "ValidateCheckTailNeighbour: Interval Variable Earlier Than Tail Mark");
}
#endif

void
SamePermutationCtI::checkTailNeighbour(IlcIntervalSequenceVar s1,
                                       IloInt index) {
  // This function allows to check the relative position of
  // the immediate neighbours of the intervals indexed by
  // index in the tail of s.
  //
  // We assert this function is called only for index of 
  // intervals strictly later than the synchronization mark.
  //
  IlcIntervalVar v1 = getInterval(index, s1);
#ifndef NDEBUG
  IlcIntervalVar tail = getTailInterval(s1);
  ValidateCheckTailNeighbour(s1, v1, tail);
#endif
  IlcIntervalSequenceVar s2 = getOtherSequence(s1);
  IlcIntervalVar v2 = getInterval(index, s2);
  if (s2.isInTail(v2)) 
    checkPartialChain(v1,
                      s1.getOneEarlierInTail(v1),
                      s1.getOneLaterInTail(v1),
                      s1,
                      v2,
                      s2.getOneEarlierInTail(v2),
                      s2.getOneLaterInTail(v2),
                      s2);
}

/////////////////////////////////////////////////////////////
//
//
// Filtering Algorithm for Not Sequenced
//
//
////////////////////////////////////////////////////////////

void
SamePermutationCtI::propagateNotSequenced
(IlcIntervalSequenceVar s1) {
  // we suppose we all pair of interval has same presence
  // status
  // if the head of s2 is fully synchronized
  //   1) if the head of s1 is fully synchronized
  //     not candidate head of s1 and s2 are the same sets
  //   2) either, 
  //  let h1 the set of in head and after the synchronized
  //  boundary of s1. Let c2 the sets of companion of h1.
  //     2-1) If one of the c2 is the next earlier in head, 
  //  and, so forth is candidate head. Let a not candidate 
  //  head in s1, its companion in s2 is not in c2, so 
  //  after one candidate head: it is not candidate head.
  //     2-2) either, each of the c2 or its companion in h1 
  //  is absent in the solution. So all c2 or h1 are absent
  //  and the head of s1 is actually fully synchronized:
  //  ase 1) applies.
  //
  // if not allSamepresence, then this rule hold
  //   if any candidateHead of s2 whose companion in s1 is not
  // not candidate head in s1 is of same presence.
  //   by removing candidate head only x of s2 which is in
  //   same presence than its companoinh in s1
  assert(allSamePresence());
  IlcIntervalSequenceVar s2 = getOtherSequence(s1);
  if (s1.isSequenced() || s2.isSequenced())
    return;
  if ((getHeadInterval(s2).getImpl() ==
       s2.getLatestInHead().getImpl())) {
    for (IlcIntervalSequenceVar::Iterator
           it(s1, IlcIntervalSequenceVar::NotSequenced);
         it.ok(); ++it) {
      IlcIntervalVar var = *it;
      if (!s1.isCandidateHead(var)) {
        IloInt index = getIndex(var);
        var = getInterval(index, s2);
        s2.removeCandidateHead(var);
      }
    }
  }
  if ((getTailInterval(s2).getImpl() ==
       s2.getLatestInTail().getImpl())) {
    for (IlcIntervalSequenceVar::Iterator
           it(s1, IlcIntervalSequenceVar::NotSequenced);
         it.ok(); ++it) {
      IlcIntervalVar var = *it;
      if (!s1.isCandidateTail(var)) {
        IloInt index = getIndex(var);
        var = getInterval(index, s2);
        s2.removeCandidateTail(var);
      }
    }
  }
}


/////////////////////////////////////////////////////////////
//
//
// Members called by demons
//
//
////////////////////////////////////////////////////////////

void
SamePermutationCtI::propagateHead(IlcIntervalSequenceVar s,
                                  IlcIntervalVar from) {
  syncHead();
  IlcIntervalVar head = getHeadInterval(s);
  if (s.isEarlierInHead(head, s.getLatestInHead())) {
    for(IlcIntervalSequenceVar::Iterator
          it(s, IlcIntervalSequenceVar::Head, from);
        it.ok(); ++it) {
      IlcIntervalVar var = *it;
      if (s.isEarlierInHead(head, var)) 
        checkHeadNeighbour(s, getIndex(var));
    }
  }
}

void
SamePermutationCtI::propagateTail(IlcIntervalSequenceVar s,
                                  IlcIntervalVar from) {
  syncTail();
  IlcIntervalVar tail = getTailInterval(s);
  if (s.isEarlierInTail(tail, s.getLatestInTail())) {
    for(IlcIntervalSequenceVar::Iterator
          it(s, IlcIntervalSequenceVar::Tail, from);
        it.ok(); ++it) {
      IlcIntervalVar var = *it;
      if (s.isEarlierInTail(tail, var)) 
        checkTailNeighbour(s, getIndex(var));
    }
  }
}

void
SamePermutationCtI::propagatePresence
(IlcIntervalSequenceVar s, IlcIntervalVar var) {
  if (s.isInHead(var) &&
      !s.isEarlierInHead(var, getHeadInterval(s))) {
    syncHead(); // To Do as required in many cases
    if (s.isEarlierInHead(getHeadInterval(s),
                          s.getOneEarlierInHead(var)))
      checkHeadNeighbour(s, getIndex(var));
  }
  if (s.isInTail(var) &&
      !s.isEarlierInTail(var, getTailInterval(s))) {
    syncTail();
    if (s.isEarlierInTail(getTailInterval(s),
                          s.getOneEarlierInTail(var)))
      checkTailNeighbour(s, getIndex(var));
  }
}

/////////////////////////////////////////////////////////////
//
//
// Members Interface of constraint class
//
//
////////////////////////////////////////////////////////////

void SamePermutationCtI::post() {
  IloCPEngine cp = getCPEngine();
  IlcIntervalSequenceVar var = getSequence1();
  var.whenExtendHead(DeltaHead1Demon(cp, this));
  var.whenExtendTail(DeltaTail1Demon(cp, this));
  var.whenPresence(DeltaPresence1Demon(cp, this));
  if (allSamePresence())
    var.whenNotSequenced(NotSequenced1Demon(cp, this));
  var = getSequence2();
  var.whenExtendHead(DeltaHead2Demon(cp, this));
  var.whenExtendTail(DeltaTail2Demon(cp, this));
  var.whenPresence(DeltaPresence2Demon(cp, this));
  if (allSamePresence())
    var.whenNotSequenced(NotSequenced2Demon(cp, this));
}

void SamePermutationCtI::propagate() {
  IlcIntervalSequenceVar s = getSequence1();
  propagateHead(s, 0);
  propagateTail(s, 0);
  s = getSequence2();
  propagateHead(s, 0);
  propagateTail(s, 0);
  if (allSamePresence()) {
    propagateNotSequenced(getSequence1());
    propagateNotSequenced(s);
  }
}

void SamePermutationCtI::display(ILOSTD(ostream) &str) const {
  if (getName())
    str << getName();
  else
    str << "SamePermutationCtI";
  str << "[ " << getSequence1()
      << ", " << getSequence2() << "]";
}


/////////////////////////////////////////////////////////////
//
//
// MODEL-ENGINE CONSTRAINT WRAPPING
//
//
////////////////////////////////////////////////////////////

ILOCPCONSTRAINTWRAPPER5(IloSamePermutation, cp,
                        IloIntervalSequenceVar, s1,
                        IloIntervalVarArray, vs1,
                        IloIntervalSequenceVar, s2,
                        IloIntervalVarArray, vs2,
                        IloIntArray, samePresences) {
  // Extraction from model data.
  // Extracting sequence, it also extract intervals of sequences
  use(cp, s1);
  use(cp, s2);
  // Create engine data structures
  IloInt size = vs1.getSize();
  IlcIntervalVarArray intervals1(cp, size);
  IlcIntervalVarArray intervals2(cp, size);
  IloInt i;
  for(i = 0; i < size; ++i) {
    intervals1[i] = cp.getInterval(vs1[i]);
    intervals2[i] = cp.getInterval(vs2[i]);
    IloAssert(intervals1[i].getImpl(),
              "IloSamePermutation: Interval not extracted");
  }
  IlcIntArray same;
  if (samePresences.getImpl()) {
    // always copy array from model
    size = samePresences.getSize();
    same = IlcIntArray(cp, size);
    for(i = 0; i < size; ++i) 
      same[i] = ((vs1[i].getImpl() == vs2[i].getImpl()) ||
                samePresences[i]) ? IloTrue : IloFalse;
  }
  // Create constraint
  IlcIntervalSequenceVar cs1 = cp.getIntervalSequence(s1);
  IlcIntervalSequenceVar cs2 = cp.getIntervalSequence(s2);
  return new (cp.getHeap())
    SamePermutationCtI(cp, cs1, cs2, intervals1, intervals2, same);
}

/////////////////////////////////////////////////////////////
//
//
// A CONVEYOR BELT SAMPLE MODEl
//
//
////////////////////////////////////////////////////////////

void MakeConveyorBelt(IloModel model,
                      IloIntArray sizes,
                      IloIntArray weigths,
                      IloIntArray optionals,
                      IloInt transTime,
                      IloInt loadTime,
                      IloInt unloadTime,
                      IloInt weigthMax,
                      IloInt sizeMax) {
  // This function creates a conveyor belt model for a
  // set of items
  // The load, transporation, and unload time of an item are
  // interger parameters load, trans, and unload.
  // The maximum weigth and size that can instantaneously
  // support the conveyor belt are weigthMax and sizeMax
  // we suppose there is no other limitation rules on the
  // conveyor belt
  // each item as a size and a weight
  // each item is present at the solution
  
  char buffer[128];
  IloEnv env = model.getEnv();

  IloCumulFunctionExpr weigth(env);
  IloCumulFunctionExpr size(env);
  IloIntervalVarArray intervals(env);
  IloIntervalVarArray loads(env);
  IloIntervalVarArray unloads(env);
  IloInt nb = intervals.getSize();
  
  for(IloInt i = 0; i < nb; ++nb) {

    IloIntervalVar trans(env);
    trans.setLengthMin(loadTime + unloadTime + transTime);
    trans.setOptional();
    sprintf(buffer, "CB<Item%ld>", (long)i);
    trans.setName(buffer);
    intervals.add(trans);
    
    
    IloIntervalVar load(env, loadTime);
    load.setOptional();
    sprintf(buffer, "Load<Item%ld>", (long)i);
    load.setName(buffer);
    loads.add(load);

    IloIntervalVar unload(env, unloadTime);
    unload.setOptional();
    sprintf(buffer, "Unload<Item%ld>", (long)i);
    unload.setName(buffer);
    unloads.add(unload);

    // the trans load and item model constraints
    // temporal synchronization
    model.add(IloStartAtStart(env, load, trans));
    model.add(IloEndBeforeStart(env, load, unload, transTime));
    model.add(IloEndAtEnd(env, trans, unload));
    // logical synchronization and presence
    if (!optionals[i]) {
      model.add(IloPresenceOf(env, load));
      model.add(IloPresenceOf(env, trans));
      model.add(IloPresenceOf(env, unload));
    } else {
      model.add(IloPresenceOf(env, load) == IloPresenceOf(env, trans));
      model.add(IloPresenceOf(env, load) == IloPresenceOf(env, unload));
    }
    weigth += IloPulse(intervals[i], weigths[i]);
    size += IloPulse(intervals[i], sizes[i]);
  }
  
  model.add(weigth <= weigthMax);
  model.add(size <= sizeMax);
  
  IloIntervalSequenceVar s1(env, loads);
  IloIntervalSequenceVar s2(env, unloads);
  model.add(IloNoOverlap(env, s1));
  model.add(IloNoOverlap(env, s2));
  model.add(IloSamePermutation(env, s1, loads, s2, unloads, 0));
}

/* ------------------------------------------------------------

Problem Description
-------------------

This problem is a special case of Job-Shop Scheduling problem (see
sched_jobshop.cpp) for which all jobs have the same processing order
on machines because there is a technological order on the machines for
the different jobs to follow.

------------------------------------------------------------ */

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/data/flowshop_default.data";
    IloInt failLimit = 10000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    char name[128];
    IloInt nbJobs, nbMachines;
    file >> nbJobs;
    file >> nbMachines;
    IloIntervalVarArray2 machines(env, nbMachines);
    for (IloInt j = 0; j < nbMachines; j++)
      machines[j] = IloIntervalVarArray(env);
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbJobs; i++) {
      IloIntervalVar prec;
      for (IloInt j = 0; j < nbMachines; j++) {
        IloInt d;
        file >> d;
        IloIntervalVar ti(env, d);
        sprintf(name, "Job%ldShop%ld", (long)i, (long)j);
        ti.setName(name);
        machines[j].add(ti);
        if (0 != prec.getImpl())
          model.add(IloEndBeforeStart(env, prec, ti));
        prec = ti;
      }
      ends.add(IloEndOf(prec));
    }
    IloIntervalSequenceVarArray sequences(env);
    IloIntervalSequenceVar last;
    for (IloInt j = 0; j < nbMachines; j++) {
      IloIntervalSequenceVar var(env, machines[j]);
      sprintf(name, "Shop%ld", (long)j);
      var.setName(name);
      sequences.add(var);
      model.add(IloNoOverlap(env, var));
      if (last.getImpl())
        model.add(IloSamePermutation(env,
                                     last, machines[j -1],
                                     var, machines[j],
                                     0));
      last = var;
    }
    IloObjective objective = IloMinimize(env,IloMax(ends));
    model.add(objective);

    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
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
