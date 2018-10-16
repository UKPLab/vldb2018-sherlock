
// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilcp/cp.h
// --------------------------------------------------------------------------
//
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2017 All Rights Reserved.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
//
// --------------------------------------------------------------------------
#ifndef __CP_cpH
#define __CP_cpH

#ifdef _MSC_VER
#pragma pack(push,8)
#endif


#ifndef USE_ILCENGINE_CLASS
#define USE_ILCENGINE_CLASS
#endif


#ifndef ILCCONCERTLESS
# define ILCCONCERTLESS
#endif



#ifndef CPPREF_GENERATION

#define IlcAbs IlcCPOAbs
#define IlcAllocationStack IlcCPOAllocationStack

#define IlcBranchSelectorI IlcCPOBranchSelectorI

#define IlcChooseIntVarI IlcCPOChooseIntVarI
#define IlcChooseIntVar IlcCPOChooseIntVar
#define IlcConstIntArray IlcCPOConstIntArray
#define IlcConstraintArray IlcCPOConstraintArray
#define IlcConstraintI IlcCPOConstraintI
#define IlcConstraint IlcCPOConstraint
#define IlcCPOFloatVarI IlcCPOFloatExpI
#define IlcDemonI IlcCPODemonI
#define IlcDemon IlcCPODemon
#define IlcExponent IlcCPOExponent
#define IlcExprI IlcCPOExprI
#define IlcExtension IlcCPOExtension
#define IlcFloatArrayI IlcCPOFloatArrayI
#define IlcFloatArray IlcCPOFloatArray
#define IlcFloatExpI IlcCPOFloatExpI
#define IlcFloatExp IlcCPOFloatExp
#define IlcFloatMax IlcCPOFloatMax
#define IlcFloatMin IlcCPOFloatMin
#define IlcFloatVarArrayI IlcCPOFloatVarArrayI
#define IlcFloatVarArray IlcCPOFloatVarArray
#define IlcFloatVarArrayIterator IlcCPOFloatVarArrayIterator
#define IlcFloatVarI IlcCPOFloatExpI
#define IlcFloatVar IlcCPOFloatVar
#define IlcGoalArray IlcCPOGoalArray
#define IlcGoalI IlcCPOGoalI
#define IlcGoal IlcCPOGoal
#define IlcIntArray IlcCPOIntArray
#define IlcIntExpI IlcCPOIntExpI
#define IlcIntExp IlcCPOIntExp
#define IlcIntExpIterator IlcCPOIntExpIterator
#define IlcIntPredicateI IlcCPOIntPredicateI
#define IlcIntPredicate IlcCPOIntPredicate
#define IlcIntSelectEvalI IlcCPOIntSelectEvalI
#define IlcIntSelectI IlcCPOIntSelectI
#define IlcIntSelect IlcCPOIntSelect
#define IlcIntSetArray IlcCPOIntSetArray
#define IlcIntSetI IlcCPOIntSetI
#define IlcIntSet IlcCPOIntSet
#define IlcIntSetIterator IlcCPOIntSetIterator
#define IlcIntSetVarArray IlcCPOIntSetVarArray
#define IlcIntSetVarDeltaIterator IlcCPOIntSetVarDeltaIterator
#define IlcIntSetVarI IlcCPOIntSetVarI
#define IlcIntSetVar IlcCPOIntSetVar
#define IlcIntSetVarIterator IlcCPOIntSetVarIterator
#define IlcIntTupleSet IlcCPOIntTupleSet
#define IlcIntTupleSetIterator IlcCPOIntTupleSetIterator
#define IlcIntVarArrayI IlcCPOIntVarArrayI
#define IlcIntVarArray IlcCPOIntVarArray
#define IlcIntVarDeltaIterator IlcCPOIntVarDeltaIterator
#define IlcIntVarI IlcCPOIntVarI
#define IlcIntVar IlcCPOIntVar
#define IlcLog IlcCPOLog
#define IlcManagerI IlcCPOManagerI
#define IlcManager IlcCPOManager
#define IlcMax IlcCPOMax
#define IlcMin IlcCPOMin
#define IlcPower IlcCPOPower
#define IlcRandomI IlcCPORandomI
#define IlcRandom IlcCPORandom
#define IlcRevAny IlcCPORevAny
#define IlcRevBool IlcCPORevBool
#define IlcRevFloat IlcCPORevFloat
#define IlcRevInt IlcCPORevInt
#define IlcSearchI IlcCPOSearchI
#define IlcSearchLimitI IlcCPOSearchLimitI
#define IlcSearchMonitorI IlcCPOSearchMonitorI
#define IlcSearchMonitor IlcCPOSearchMonitor
#define IlcStamp IlcCPOStamp
#define IloCPConstraintI IloCPOCPConstraintI
#define IloFailLimit IloCPOFailLimit
#define IloGoalFail IloCPOGoalFail
#define IloGoalI IloCPOGoalI
#define IloGoal IloCPOGoal
#define IloGoalTrue IloCPOGoalTrue
#define IloOrLimit IloCPOOrLimit
#define IloSearchLimitI IloCPOSearchLimitI
#define IloSearchLimit IloCPOSearchLimit
#define IloSolver IloCPOSolver
#define IloTimeLimit IloCPOTimeLimit
#define IlcIntSetIteratorI IlcCPOIntSetIteratorI

#endif

//----------------------------------------------------------------------
// Macros for GCC attributes

#ifdef NDEBUG
#define ILC_GCC_VISIBILITY
#endif

#define ILCDOCHIDINGON                          \


#define ILCDOCHIDINGOFF                         \


#define ILCDOCHIDE                              \


#if defined(__GNUC__) && (__GNUC__ >= 4) && defined(ILC_GCC_VISIBILITY)
#  define ILCGCCHIDINGENABLED
#  define ILCGCCHIDINGON      _Pragma("GCC visibility push(hidden)") ILCDOCHIDINGON
#  define ILCGCCHIDINGOFF     _Pragma("GCC visibility pop") ILCDOCHIDINGOFF
#  define ILCHIDDEN           __attribute__((visibility("hidden"))) ILCDOCHIDE
#  define ILCGCCEXPORTON      _Pragma("GCC visibility push(default)")
#  define ILCGCCEXPORTOFF     _Pragma("GCC visibility pop")
#  define ILCEXPORT           __attribute__((visibility("default")))
#else
#  define ILCGCCHIDINGON        ILCDOCHIDINGON
#  define ILCGCCHIDINGOFF       ILCDOCHIDINGOFF
#  define ILCHIDDEN             ILCDOCHIDE
#  define ILCGCCEXPORTON
#  define ILCGCCEXPORTOFF
#  define ILCEXPORT
#endif

#if defined(__GNUC__)
#define ILCDEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define ILCDEPRECATED __declspec(deprecated)
#else
#define ILCDEPRECATED
#endif

ILCGCCHIDINGON
class IloCPI;
class IloLaExtractorI;
class IlcLaMessageStore;
class IlcAllocator;
struct IlcAllocatorAllocatorTraits;
typedef IlcAllocator* IlcAllocatorPtr;
ILCGCCHIDINGOFF

//----------------------------------------------------------------------

#if !defined(__CONCERT_iloalgH)
# include <ilconcert/iloalg.h>
#endif
#if !defined(__CONCERT_ilomodelH)
# include <ilconcert/ilomodel.h>
#endif
#if !defined(__CONCERT_ilotuplesetH)
# include <ilconcert/ilotupleset.h>
#endif
#if !defined(__CONCERT_ilosmodelH)
# include <ilconcert/ilosmodel.h>
#endif
#if !defined(__CONCERT_ilosatomiH)
# include <ilconcert/ilsched/ilosatomi.h>
#endif

ILCGCCEXPORTON

//--------------------------------------------------------------------------------
// moved from cpext.h



typedef IloBool IlcBool;
#define IlcFalse IloFalse
#define IlcTrue IloTrue


typedef IloInt IlcInt;
typedef IloUInt IlcUInt;

#ifdef ILO64

typedef IlcInt IlcInt64;
typedef IlcUInt IlcUInt64;
#define IlcInt64Const(x) x##L
#define IlcUInt64Const(x) x##UL

#else 

#if defined(_MSC_VER)
  typedef __int64          IlcInt64;
  typedef unsigned __int64 IlcUInt64;
#else
  typedef int64_t  IlcInt64;
  typedef uint64_t IlcUInt64;
#endif
  // These exist on UNIX platforms as INT64_C etc, but they sometimes require
  // explicit preprocessor symbol definitions to enable them.  LL and ULL
  // seem pretty universal, so we go with that instead.
  #define IlcInt64Const(x) x##LL
  #define IlcUInt64Const(x) x##ULL
#endif 

typedef IlcInt64 IlcBigInt;
typedef IlcUInt64 IlcUBigInt;

#define IlcInt64Max  IlcInt64Const(0x7fffffffffffffff)
#define IlcInt64Min  (IlcInt64Const(-1) - IlcInt64Max)
#define IlcUInt64Max IlcUInt64Const(0xffffffffffffffff)
#define IlcBigIntMax IlcInt64Max



typedef IloAny IlcAny;

typedef const void* IlcConstAny;


typedef IloNum IlcFloat;


#define IlcInfinity HUGE_VAL




//----------------------------------------------------------------------

#ifdef NDEBUG

#define IlcCPOAssert(x,y)

#elif defined (USEILOASSERTFORILCASSERT)

#define IlcCPOAssert(x,y) IloAssert(x,y)

#else

inline int ilc_stop_assert() { return 0; }
void IlcBacktrace();
#define IlcCPOAssert(x,y) assert((x) || (IlcBacktrace(), ILOSTD(cerr) << (y) << ILOSTD(endl), ilc_stop_assert()))

#endif

#define ILCCONCAT(a,b) a##b


//----------------------------------------------------------------------

#define ILOCPVISIBLEHANDLEMINI(Hname, Iname)                       \
public:                                                            \
                                                                \
  Hname(Iname* impl = 0) : _impl(impl) { }                         \
                                                                \
  Iname* getImpl() const { return _impl; }                         \
  Iname* getImplInternal() const                                   \
    { return (Iname*)_impl; }                                      \
protected:                                                         \
  Iname* _impl;

#ifdef ILCGCCHIDINGENABLED
#define ILOCPHIDDENHANDLEMINI(Hname, Iname)                        \
public:                                                            \
                                                                \
  Hname(Iname* impl = 0) : _impl(impl) { }                         \
                                                                \
  Iname* getImpl() const { return (Iname*)_impl; }                 \
  Iname* getImplInternal() const                                   \
    { return (Iname*)_impl; }                                      \
protected:                                                         \
  void* _impl;
#else
#define ILOCPHIDDENHANDLEMINI(Hname, Iname)                        \
        ILOCPVISIBLEHANDLEMINI(Hname, Iname)
#endif

#define ILOCPVISIBLEHANDLE(Hname, Iname)                           \
  ILOCPVISIBLEHANDLEMINI(Hname, Iname)                             \
private:                                                           \
  const char *  _getName() const;                                  \
  IloAny        _getObject() const;                                \
  void          _setName(const char * name) const;                 \
  void          _setObject(IloAny obj) const;                      \
public:                                                            \
                                                                \
  const char * getName() const {                                   \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _getName();                                             \
  }                                                                \
                                                                \
  IloAny getObject() const {                                       \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _getObject();                                           \
  }                                                                \
                                                                \
  void setName(const char * name) const {                          \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _setName(name);                                                \
  }                                                                \
                                                                \
  void setObject(IloAny obj) const {                               \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _setObject(obj);                                               \
  }                                                                \

#ifdef ILCGCCHIDINGENABLED
#define ILOCPHIDDENHANDLE(Hname, Iname)                            \
  ILOCPHIDDENHANDLEMINI(Hname, Iname)                              \
private:                                                           \
  const char *  _getName() const;                                  \
  IloAny        _getObject() const;                                \
  void          _setName(const char * name) const;                 \
  void          _setObject(IloAny obj) const;                      \
public:                                                            \
                                                                \
  const char * getName() const {                                   \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _getName();                                             \
  }                                                                \
                                                                \
  IloAny getObject() const {                                       \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _getObject();                                           \
  }                                                                \
                                                                \
  void setName(const char * name) const {                          \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _setName(name);                                                \
  }                                                                \
                                                                \
  void setObject(IloAny obj) const {                               \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _setObject(obj);                                               \
  }
#else

#define ILOCPHIDDENHANDLE(Hname, Iname) ILOCPVISIBLEHANDLE(Hname, Iname)

#endif

#define ILOCPHANDLEINLINE(Hname, Iname)                            \
  ILOCPVISIBLEHANDLEMINI(Hname, Iname)                             \
public:                                                            \
                                                                \
  const char * getName() const {                                   \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _impl->getName();                                       \
  }                                                                \
                                                                \
  IloAny getObject() const {                                       \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    return _impl->getObject();                                     \
  }                                                                \
                                                               \
  void setName(const char * name) const {                          \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _impl->setName(name);                                                \
  }                                                                \
                                                                \
  void setObject(IloAny obj) const {                               \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(hname) ": empty handle");  \
    _impl->setObject(obj);                                               \
  }                                                                \

//----------------------------------------------------------------------

// Concert classes:
class IloCumulFunctionExpr;
class IloDiff;
class IloExtensibleRttiEnvObjectI;
class IloIntArray;
class IloIntExp;
class IloIntSet;
class IloIntSetVar;
class IloIntSetVarArray;
class IloIntVar;
class IloIntVarArray;
class IloStateFunctionI;

// Internal classes (not defined neither in cp.h nor cpext.h)
ILCGCCHIDINGON
class IlcAllocationStack;
class IlcExprI;
class IlcFloatExpI;
class IlcIntExpI;
class IlcIntVarI;
class IlcManagerI;
class IlcRandom;
class IlcRecomputeExprI;
class IlcStrategyManagerI;
class IloSearchPhaseI;
class IloValueSelectorI;
class IloVarSelectorI;
class IlcCPEngineI;


class IlcCPI;
class IlcEngineI;
class IlcCPServiceEngineI;
class IlcLaObjectiveValue;
class IlcEngineState;
class IlcLaIntVar;
class IlcLaFloatVar;
class IlcLaIntervalVar;
class IlcLaSequenceVar;
class IlcLaCumulFunction;
class IlcLaStateFunction;
class IlcLaObject;

ILCGCCHIDINGOFF

// External classes (defined in cp.h or cpext.h)
class IlcConstraint;
class IlcConstraintArray;
class IlcCumulElementVar;
class IlcFloatArray;
class IlcFloatExp;
class IlcFloatVar;
class IlcFloatVarArray;
class IlcGoal;
class IlcIntArray;
class IlcIntervalSequenceVar;
class IlcIntervalVar;
class IlcIntExp;
class IlcIntSet;
class IlcIntSetVar;
class IlcIntSetVarArray;
class IlcIntTupleSet;
class IlcIntVar;
class IlcIntVarArray;

class IloCPEngine;
class IlcCPEngine;
class IlcManager;

class IloCP;

class IloGoal;


////////////////////////////////////////////////////////////////////////
//
// CUSTOM SEARCH
//
////////////////////////////////////////////////////////////////////////


class IloIntVarEvalI : public IloExtensibleRttiEnvObjectI
{
 public:
  
  IloIntVarEvalI(IloEnv env):
    IloExtensibleRttiEnvObjectI(env.getImpl()){}
  
  
  virtual IloNum eval(IloCPEngine, IloIntVar) = 0;
  IlcFloat internalEval(IlcCPEngineI* engine, const void* var);       
  
  virtual ~IloIntVarEvalI();
  ILORTTIDECL
};


class IloIntVarEval {
  ILOCPVISIBLEHANDLEMINI(IloIntVarEval, IloIntVarEvalI)
public:

  void end();
};


class IloIntValueEvalI : public IloExtensibleRttiEnvObjectI 
{
 public:
  
  IloIntValueEvalI(IloEnv env) :
    IloExtensibleRttiEnvObjectI(env.getImpl()){}
  
  
  virtual IloNum eval(IloCPEngine cp, IloIntVar x, IloInt value) = 0;
  IlcFloat internalEval(IlcCPEngineI* cp, const void* x, IlcInt value);       
  
  virtual ~IloIntValueEvalI();
  ILORTTIDECL
};


class IloIntValueEval {
  ILOCPVISIBLEHANDLEMINI(IloIntValueEval, IloIntValueEvalI)
public:

  void end();
};


class IloVarSelector {
  ILOCPHIDDENHANDLEMINI(IloVarSelector, IloVarSelectorI)
public:

  void end();
};


typedef IloArray<IloVarSelector> IloVarSelectorArray;


IloVarSelector IloSelectSmallest(IloIntVarEval eval);


IloVarSelector IloSelectSmallest(IloNum minNumber, IloIntVarEval eval);


IloVarSelector IloSelectSmallest(IloIntVarEval eval, IloNum tol);


IloVarSelector IloSelectLargest(IloIntVarEval eval);


IloVarSelector IloSelectLargest(IloNum minNumber, IloIntVarEval eval);


IloVarSelector IloSelectLargest(IloIntVarEval eval, IloNum tol);


IloVarSelector IloSelectRandomVar(IloEnv env);


class IloValueSelector {
  ILOCPHIDDENHANDLEMINI(IloValueSelector, IloValueSelectorI)
public:

  void end();
};


typedef IloArray<IloValueSelector> IloValueSelectorArray;


IloValueSelector IloSelectSmallest(IloIntValueEval eval);


IloValueSelector IloSelectSmallest(IloNum minNumber, IloIntValueEval eval);


IloValueSelector IloSelectSmallest(IloIntValueEval eval, IloNum tol);


IloValueSelector IloSelectLargest(IloIntValueEval eval);


IloValueSelector IloSelectLargest(IloNum minNumber, IloIntValueEval eval);


IloValueSelector IloSelectLargest(IloIntValueEval eval, IloNum tol);


IloValueSelector IloSelectRandomValue(IloEnv env);


class IloIntVarChooserI : public IloExtensibleRttiEnvObjectI {
 public:

  IloIntVarChooserI(IloEnv env) :
    IloExtensibleRttiEnvObjectI(env.getImpl()){}

  virtual IloInt choose(IloCPEngine cp, IloIntVarArray x) = 0;
  
  virtual ~IloIntVarChooserI();
  ILORTTIDECL
};


class IloIntVarChooser {
  ILOCPVISIBLEHANDLEMINI(IloIntVarChooser, IloIntVarChooserI)
public:

  IloIntVarChooser(IloVarSelector varSel);

  IloIntVarChooser(IloVarSelectorArray varSelArray);

  IloIntVarChooser(IloEnv env, IloVarSelector varSel);

  IloIntVarChooser(IloEnv env, IloVarSelectorArray varSelArray);

  void end();
};


class IloIntValueChooserI : public IloExtensibleRttiEnvObjectI {
 public:

  IloIntValueChooserI(IloEnv env) :
    IloExtensibleRttiEnvObjectI(env.getImpl()){}

  virtual IloInt choose(IloCPEngine cp, IloIntVarArray x, IloInt index) = 0;
  
  virtual ~IloIntValueChooserI();
  ILORTTIDECL
};


class IloIntValueChooser {
  ILOCPVISIBLEHANDLEMINI(IloIntValueChooser, IloIntValueChooserI)
public:

  IloIntValueChooser(IloValueSelector valueSel);

  IloIntValueChooser(IloValueSelectorArray valueSelArray);

  IloIntValueChooser(IloEnv env, IloValueSelector valueSel);

  IloIntValueChooser(IloEnv env, IloValueSelectorArray valueSelArray);

  void end();
};

////////////////////////////////////
//  IloSearchPhaseI


class IloSearchPhase {
  friend class  IlcStrategyManagerI;
  ILOCPHIDDENHANDLE(IloSearchPhase, IloSearchPhaseI)
 public:
  
  void end();
  
  IloSearchPhase(IloEnv env,
                 IloIntVarArray vars,
                 IloIntVarChooser varChooser,
                 IloIntValueChooser valueChooser);

  
  IloSearchPhase(IloEnv env,
                 IloIntVarArray vars);
  
  IloSearchPhase(IloEnv env,
                 IloIntVarChooser varChooser,
                 IloIntValueChooser valueChooser);

  
  IloSearchPhase(IloEnv env, IloIntervalVarArray intervalVars);

  
  IloSearchPhase(IloEnv env, IloIntervalSequenceVarArray sequenceVars);

};

// Undocumented:
IloSearchPhase IloFixPresenceSearchPhase(IloEnv env, IloIntervalVarArray intervalVars);


typedef IloArray<IloSearchPhase> IloSearchPhaseArray;


IloIntValueEval IloExplicitValueEval(IloEnv env,
                                     IloIntArray valueArray,
                                     IloIntArray evalArray,
                                     IloNum defaultEval = 0);

IloIntValueEval IloExplicitValueEval(IloEnv env,
                                     IloIntArray valueArray,
                                     IloNumArray evalArray,
                                     IloNum defaultValue = 0);

IloIntValueEval IloValueIndex(IloEnv env,
                              IloIntArray valueArray,
                              IloInt defaultEval = -1);

IloIntValueEval IloValue(IloEnv env);

IloIntValueEval IloValueImpact(IloEnv env);

IloIntValueEval IloValueSuccessRate(IloEnv env);

IloIntValueEval IloValueLocalImpact(IloEnv env);


IloIntValueEval IloValueLowerObjVariation(IloEnv env);

IloIntValueEval IloValueUpperObjVariation(IloEnv env);

IloIntVarEval IloVarIndex(IloEnv env, IloIntVarArray x, IloInt defaultEval = -1);

IloIntVarEval IloExplicitVarEval(IloEnv env, IloIntVarArray x, IloIntArray evalArray, IloNum defaultEval = 0);

IloIntVarEval IloExplicitVarEval(IloEnv env, IloIntVarArray x, IloNumArray evalArray, IloNum defaultEval = 0);

IloIntVarEval IloDomainMin(IloEnv env);

IloIntVarEval IloDomainMax(IloEnv env);

IloIntVarEval IloDomainSize(IloEnv env);

IloIntVarEval IloVarSuccessRate(IloEnv env);

IloIntVarEval IloVarImpact(IloEnv env);

IloIntVarEval IloVarLocalImpact(IloEnv env, IloInt effort = -1);

IloIntVarEval IloImpactOfLastBranch(IloEnv env);


IloIntVarEval IloRegretOnMin(IloEnv env);

IloIntVarEval IloRegretOnMax(IloEnv env);

IloIntVarEval IloVarLowerObjVariation(IloEnv env);

IloIntVarEval IloVarUpperObjVariation(IloEnv env);



#ifndef CPPREF_GENERATION
//----------------------------------------------------------------------
//
// this is a handle on domain iterator
// the iterator can come from:
//  the solver's dynamical state
//  the stored solution
template<class X>
class IlcRefCountingHandle {
protected:
  X*    _impl;
  void _setImpl(X* impl);
  X * getImpl() const { return _impl; }
public:
  IlcRefCountingHandle(X* impl = 0)
    : _impl(0)
  { _setImpl(impl); }
  IlcRefCountingHandle(const IlcRefCountingHandle& x)
    : _impl(0)
  { _setImpl(x._impl); }
  IlcRefCountingHandle<X>& operator = (const IlcRefCountingHandle<X>& other) {
    _setImpl(other._impl);
    return *this;
  }
  ~IlcRefCountingHandle()
  { _setImpl(0); }
};

//----------------------------------------------------------------------
class IlcIntDomainIteratorI;
class IlcIntDomainIterator : public IlcRefCountingHandle<IlcIntDomainIteratorI> {
public:
  IlcIntDomainIterator(IlcIntDomainIteratorI* impl = 0)
    : IlcRefCountingHandle<IlcIntDomainIteratorI>(impl)
  { }
  IlcIntDomainIterator(const IlcIntDomainIterator& x)
    : IlcRefCountingHandle<IlcIntDomainIteratorI>(x)
  { }
  IlcIntDomainIterator& operator=(const IlcIntDomainIterator& x)  
  { return static_cast<IlcIntDomainIterator&>(IlcRefCountingHandle<IlcIntDomainIteratorI>::operator=(x)); }
protected:
  void _advance() const;
  IlcBool _ok() const;
  IlcInt _getValue() const;
public:
  IlcBool ok() const 
  { return _ok(); }
  IlcInt operator * () const 
  { return _getValue(); }
  IlcIntDomainIterator& operator++() 
  { _advance(); return *this; }
};
#endif 


////////////////////////////////////////////////////////////////////////
//
// ILOCP
//
////////////////////////////////////////////////////////////////////////



class IloCP : public IloAlgorithm {


public:
             
  
  enum IntParam {   
     
      
    DefaultInferenceLevel = 1,   
     
      
    AllDiffInferenceLevel = 2,   
     
      
    DistributeInferenceLevel = 3,   
     
      
    CountInferenceLevel = 4,   
     
      
    SequenceInferenceLevel = 5,   
     
      
    AllMinDistanceInferenceLevel = 6,   
     
      
    ElementInferenceLevel = 7,   
     
      
    FailLimit = 9,   
     
      
    ChoicePointLimit = 10,   
     
      
    LogVerbosity = 11,   
     
      
    LogPeriod = 12,   
     
      
    SearchType = 13,   
     
      
    RandomSeed = 14,   
     
      
    RestartFailLimit = 15,   
     
      
    MultiPointNumberOfSearchPoints = 16,   
     
      
    ImpactMeasures = 18,   
     
      
    PackApproximationSize = 22,   
     
      
    StrictNumericalDivision = 23,   
     
      
    FloatDisplay = 24,   
     
      
    Workers = 25,   
     
      
    BranchLimit = 28,   
     
      
    AutomaticReplay = 29,   
     
      
    SeedRandomOnSolve = 30,   
     
      
    TraceExtraction = 31,   
     
      
    DynamicProbing = 32,   
     
      
    MaxNoGoods = 33,   
     
      
    TimeDisplay = 34,   
     
      
    SolutionLimit = 35,   
     
      
    PresolveLevel = 36,   
     
      
    PrecedenceInferenceLevel = 38,   
     
      
    IntervalSequenceInferenceLevel = 39,   
     
      
    NoOverlapInferenceLevel = 40,   
     
      
    CumulFunctionInferenceLevel = 41,   
     
      
    StateFunctionInferenceLevel = 42,   
     
      
    TimeMode = 43,   
     
      
    TemporalRelaxation = 44,   
     
      
    TemporalRelaxationLevel = 45,   
     
      
    TemporalRelaxationRowLimit = 46,   
     
      
    TemporalRelaxationIterationLimit = 47,   
     
      
    SearchConfiguration = 48,   
     
      
    SequenceExpressionInferenceLevel = 50,   
     
      
    StateFunctionTriangularInequalityCheck = 51,   
     
      
    IntervalSolutionPoolCapacity = 53,   
     
      
    TTEF = 54,   
     
      
    TemporalRelaxationAlgorithm = 55,   
     
      
    RestartTimeMeasurement = 56,   
     
      
    MultiPointMaximumFailLimit = 57,   
     
      
    MultiPointDisableLamarckism = 58,   
     
      
    ParallelWorkerStackSize = 59,   
     
      
    Presolve = 60,   
     
      
    ConflictRefinerIterationLimit = 61,   
     
      
    ConflictRefinerBranchLimit = 62,   
     
      
    ConflictRefinerFailLimit = 63,   
     
      
    ConflictRefinerOnVariables = 64,   
     
      
    ConflictRefinerOnLabeledConstraints = 65,   
     
      
    ConflictRefinerAlgorithm = 66,   
     
      
    TimetablingAlgorithm = 67,   
     
      
    PresolveTransformers = 68,   
     
      
    EliminatePresolvedModel = 69,   
     
      
    ParallelMode = 70,   
     
      
    ParallelSynchronizationTicks = 71,   
     
      
    ParallelCommunicateSolutions = 72,   
     
      
    ParallelCommunicateEachSolution = 73,   
     
      
    ParallelEventQueueCapacity = 74,   
     
      
    LogDisplayWorkerIdleTime = 75,   
     
      
    StrongMaxTuples = 76,   
     
      
    ModelAnonymizer = 77,   
     
      
    FailureDirectedSearch = 78,   
     
      
    FailureDirectedSearchMaxMemory = 79,   
     
      
    SynchronizeByDeterministicTime = 80,   
     
      
    MemoryDisplay = 81,   
     
      
    MultiPointEncodeIntervalPrecedences = 82,   
     
      
    MultiPointEncodeIntervalSequences = 83,   
     
      
    MultiPointEncodeIntervalAlternatives = 84,   
     
      
    MultiPointEncodeIntervalExecutions = 85,   
     
      
    MultiPointEncodeIntervalTimes = 86,   
     
      
    MultiPointEncodeObjectives = 87,   
     
      
    ParallelOptimizeSingleWorker = 88,   
     
      
    ParallelSkipEquivalentSolutions = 89,   
     
      
    WarningLevel = 90,   
     
      
    MultiPointWeightAlleleFactories = 91,   
     
      
    MultiPointUseApproximateAssignment = 92,   
     
      
    CPOFileCompatibility = 93,   
     
      
    MultiPointUseRandomOperator = 94,   
     
      
    MultiPointUseCrossoverOperator = 95,   
     
      
    MultiPointUseMutationOperator = 96,   
     
      
    MultiPointReduceIntervalPrecedences = 97,   
     
      
    MultiPointIgnorePhases = 98,   
     
      
    MultiPointConsiderSecondaryVariables = 99,   
     
      
    UseFileLocations = 100,   
     
      
    ConflictRefinerWriteMode = 101,   
     
      
    ConflictDefinition = 102,   
     
      
    MaxPBTCaptureSize = 103,   
     
      
    CountDifferentInferenceLevel = 104,   
     
      
    MultiPointImproveDuringInit = 105,   
     
      
    PresolveIterations = 106,   
     
      
    LogSearchTags = 107,   
     
      
    MinPBTCaptureSize = 108,   
     
      
    PrintModelDetailsInMessages = 109,   
     
      
    ForbidIncludeDirective = 110,   
     
      
    FDSConfiguration = 111,   
     
      
    FDSRandomPeriod = 112,   
     
      
    FDSExtraFilteringDepth = 113,   
     
      
    FDSMinChoicesPerInterval = 114,   
     
      
    FDSMinRandomDepth = 115,   
     
      
    FDSRootStrongBranches = 116,   
     
      
    ParallelCommunicatesNoGoods = 119,   
     
      
    ParallelCommunicatesLowerBounds = 120,   
     
      
    SameMessageLimit = 121,   
     
      
    FDSStrongBranchingStartDepth = 122,   
     
      
    FDSStrongBranchingDepth = 123,   
     
      
    CPOFileStrictIdCount = 124,   
     
      
    PresolveTransformers2 = 125,   
     
      
    MaxNoGoodLength = 126,   
     
      
    PresolveConfiguration = 127,   
     
      
    RestartWithTDRSearch = 128,   
     
      
    EnforceCELimits = 129,   
     
      
    SACLimit = 130,   
     
      
    SACSkip = 131,   
     
      
    RestartBroadcastConflictMaxSize = 132,   
     
      
    IntegerPortfolioVersion = 1023,  
    
    ILC_MAX_IntParam
                   };   
  
  enum NumParam {   
     
      
    MultiPointEncodingPercentage = 17,   
     
      
    ObjectiveLimit = 37,   
     
      
    OptimalityTolerance = 1001,   
     
      
    RelativeOptimalityTolerance = 1002,   
     
      
    TimeLimit = 1003,   
     
      
    RestartGrowthFactor = 1004,   
     
      
    DynamicProbingStrength = 1005,   
     
      
    LowerBoundEffort = 1006,   
     
      
    RestartProofEmphasis = 1007,   
     
      
    MultiPointPropagationLimitFactor = 1008,   
     
      
    RestartPropagationLimitFactor = 1009,   
     
      
    ParallelRestartProp = 1010,   
     
      
    MultiPointLearningRatio = 1011,   
     
      
    ConflictRefinerTimeLimit = 1012,   
     
      
    TemporalRelaxationTimeFactor = 1013,   
     
      
    StrongMinReduction = 1014,   
     
      
    MultiPointRandomOperatorEncodingFactor = 1015,   
     
      
    MultiPointRestartProbability = 1016,   
     
      
    MultiPointRestartRatio = 1017,   
     
      
    FailureDirectedSearchEmphasis = 1018,   
     
      
    FDSDecayFactor = 1019,   
     
      
    FDSFailBonus = 1021,   
     
      
    FDSPortionForRandomDraw = 1022,  
    
    ILC_MAX_NumParam
                   };    

  
  
  enum ParameterValues {
    
    
    Auto = -1,
    
    Off = 0,
    
    On = 1,
    
    Default = 2,
    
    Low = 3,
    
    Basic = 4,
    
    Medium = 5,
    
    Extended = 6,
    
    Standard = 7,
    
    IntScientific = 8,
    
    IntFixed = 9,
    
    BasScientific = 10,
    
    BasFixed = 11,
    
    SearchHasNotFailed = 12,
    
    SearchHasFailedNormally = 13,
    
    SearchStoppedByLimit = 14,
    
    SearchStoppedByLabel = 15,
    
    SearchStoppedByExit = 16,
    
    SearchStoppedByAbort = 17,
    
    SearchStoppedByException = 18,
    
    UnknownFailureStatus = 19,
    
    Quiet = 20,
    
    Terse = 21,
    
    Normal = 22,
    
    Verbose = 23,
    
    DepthFirst = 24,
    
    Restart = 25,
    
    MultiPoint = 26,
    
    Diverse = 27,
    
    Focused = 28,
    
    Intensive = 29,
    
    Seconds = 30,
    
    HoursMinutesSeconds = 31,
    
    NoTime = 32,
    
    CPUTime = 33,
    
    ElapsedTime = 34,
    
    ConflictInfeasible = 40,
    
    ConflictHard = 41,
    
    ConflictComplementaryFeasible = 42,
    
    SearchNotStarted = 43,
    
    SearchOngoing = 44,
    
    SearchCompleted = 45,
    
    SearchStopped = 46,
    
    SearchHasNotBeenStopped = 47,
    
    SearchStoppedByUnknownCause = 48,
    
    ILC_MAX_ParameterValues
  };

  enum ParamType {  
    CallbackMapParamType,  
    LayerModelParamType,  
    LayerSolutionParamType,  
    IntParamType,  
    NumParamType,  
    ConsoleParamType, 
    MAX_PARAM_TYPES
  };

  enum SearchTypeBounds {
    FirstSearchType = DepthFirst,
    LastSearchType = MultiPoint
  };
  
 
     
  
  
  
  enum IntInfo {    
    
    
      
    NumberOfChoicePoints = 1,  
    
    
      
    NumberOfFails = 2,  
    
    
      
    NumberOfBranches = 3,  
    
    
      
    NumberOfVariables = 4,  
    
    
      
    NumberOfAuxiliaryVariables = 5,  
    
    
      
    NumberOfEngineVariables = 6,  
    
    
      
    NumberOfConstraints = 7,  
    
    
      
    MemoryUsage = 8,  
    
    
      
    NumberOfConstraintsAggregated = 9,  
    
    
      
    NumberOfConstraintsGenerated = 10,  
    
    
      
    FailStatus = 11,  
    
    
      
    NumberOfIntegerVariables = 12,  
    
    
      
    NumberOfIntervalVariables = 13,  
    
    
      
    NumberOfSequenceVariables = 14,  
    
    
      
    NumberOfSolutions = 15,  
    
    
      
    EffectiveWorkers = 16,  
    
    
      
    EffectiveDepthFirstWorkers = 17,  
    
    
      
    EffectiveMultiPointWorkers = 18,  
    
    
      
    EffectiveRestartWorkers = 19,  
    
    
      
    NumberOfPresolveTransformations = 20,  
    
    
      
    NumberOfConstraintsAdded = 21,  
    
    
      
    NumberOfConstraintsRemoved = 22,  
    
    
      
    NumberOfCriteria = 23,  
    
    
      
    NumberOfWarnings = 24,  
    
    
      
    NumberOfErrors = 25,    
    
    
      
    NumberOfStateFunctions = 28,  
    
    
      
    SearchStatus = 29,  
    
    
      
    SearchStopCause = 30,  
    
    
      
    NumberOfWorkerSynchronizations = 1008,    
    
    
    NumberOfUnboundModelVariables = 2002,    
    
    
    FailDepthCount = 2009,  
    
    
    FailDepthSum = 2010,  
    
    
    SearchDeterministicTime = 2011,  
    
    
    EngineMemoryUsage = 2012,  
    
    
    SearchMemoryUsage = 2013,    
    
    
    PeakMemoryUsage = 2017,  
    
    
      
    NumberOfEngineConstraints = 2018,  
    
    
    NumberOfUnboundModelVariablesInLogPeriod = 2020,  
    
    
    EngineIdleTimeSum = 2022,  
    
    
    EngineIdleTimeCount = 2023,  
    
    
      
    NumberOfModelVariables = 2029, 
  ILC_MAX_IntInfo
                 };    
  
  enum LayerObjectiveValueInfo {       
  ILC_MAX_LayerObjectiveValueInfo
                 };    
  
  
  
  enum NumInfo {    
    
    
      
    SolveTime = 1001,  
    
    
      
    ExtractionTime = 1002,  
    
    
      
    TotalTime = 1003,  
    
    
      
    EffectiveOptimalityTolerance = 1004,  
    
    
      
    DepthFirstIdleTime = 1005,  
    
    
      
    RestartIdleTime = 1006,  
    
    
      
    MultiPointIdleTime = 1007,  
    
    
      
    PresolveTime = 1009,  
    
    
    Gap = 2000,  
    
    
    EngineMaxIdleTime = 2014,  
    
    
    DeterministicTimePerBranch = 2016,  
    
    
    AverageFailDepth = 2021,  
    
    
    MultiPointAverageIdleTime = 2024,  
    
    
    RestartAverageIdleTime = 2025,  
    
    
    DepthFirstAverageIdleTime = 2026,  
    
    
    EngineAverageIdleTime = 2027,  
    
    
    EngineIdleTime = 2028, 
  ILC_MAX_NumInfo
                 };    
  
  enum StringInfo {     
  ILC_MAX_StringInfo
                 };    
  
  enum LayerSolutionInfo {     
  ILC_MAX_LayerSolutionInfo
                 };  

 

  
  enum ConflictStatus {
    
    ConflictPossibleMember =0,
    
    ConflictMember =1,
    
    ConflictExcluded =2
  };

  
  enum StateFunctionValue {
  
        NoState = -1
  };

 
private:
  void    _ctor(const IloModel model);
  void    _ctor(const IloEnv env);
  void    _abortSearch() const;
  void    _clearAbort() const;
  
  void    _prettyPrintState(ILOSTD(ostream)& s) const;
  
  
  IlcAllocator* _getPersistentAllocator() const;
  
  const char * _getVersion() const;
  
  void    _dumpModel(const char* filename) const;
  void    _dumpModel(ILOSTD(ostream)& s) const;
  void    _exportModel(const char* filename) const;
  void    _exportModel(ILOSTD(ostream)& s) const;
  void    _importModel(const char* filename) const;
  void    _importModel(ILOSTD(istream)& s) const;
  
  IloBool  _propagate(const IloConstraint ct) const;
  
  void    _setInferenceLevel(IloConstraint ct, IloInt level) const;
  IloInt  _getInferenceLevel(IloConstraint ct) const;
  void    _resetConstraintInferenceLevels() const;
  void    _setSearchPhases() const;
  void    _setSearchPhases(const IloSearchPhase phase) const;
  void    _setSearchPhases(const IloSearchPhaseArray phaseArray) const;
  void    _setStartingPoint(const IloSolution ws) const;
  void    _clearStartingPoint() const;
  void    _clearExplanations();
  void    _explainFailure(IloInt failIndex);
  void    _explainFailure(IloIntArray failIndexArray);
  
  IloBool _solve() const;
  
  void    _startNewSearch() const;
  IloBool _next() const;
  void    _endSearch() const;
  
  
  
  IloBool _fitsWithinCELimits() const;

public:
  static void _RegisterXML(IloEnv env);
  
  
  //
  // returns the engine used for extraction 
  // and single worker solve.
  //
  IloCPEngine getUniqueEngine() const;
  

  
private: 
  void   _setParameter(IloInt param, IloNum value) const; 
  void   _setParameter(const char * name, IloNum value) const;
  void   _setParameter(const char * name, const char * value) const;
  IloNum _getParameter(IloInt param) const; 
  IloNum _getParameter(const char * name) const;
  IloNum _getParameterDefault(IloInt param) const; 
  IloNum _getParameterDefault(const char * name) const;
  IloNum _getInfo(IloInt info) const; 
  IloNum _getInfo(const char * name) const;

public:

                
  
  IlcInt getParameter(IloCP::IntParam id) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    return (IlcInt)_getParameter(IloInt(id));
  }
  
  void setParameter(IloCP::IntParam id, IlcInt v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    _setParameter(IloInt(id), IloNum(v));
  }
  
  IlcInt getParameterDefault(IloCP::IntParam id) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    return (IlcInt)_getParameterDefault(IloInt(id));
  }
      
  
  IlcFloat getParameter(IloCP::NumParam id) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    return (IlcFloat)_getParameter(IloInt(id));
  }
  
  void setParameter(IloCP::NumParam id, IlcFloat v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    _setParameter(IloInt(id), IloNum(v));
  }
  
  IlcFloat getParameterDefault(IloCP::NumParam id) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    return (IlcFloat)_getParameterDefault(IloInt(id));
  }
         

  
  void setIntParameter(IloCP::IntParam param, IloNum value) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _setParameter(IloInt(param), value);
  }
  
  IloNum getIntParameter(IloCP::IntParam param) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _getParameter(IloInt(param));
  }
  
  IloNum getIntParameterDefault(IloCP::IntParam param) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _getParameterDefault(IloInt(param));
  }
  
  IloNum getIntInfo(IloCP::IntInfo info) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _getInfo(IloInt(info));
  }

public:
  
  
  void setParameter(const char * name, IloNum value) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(name  != 0, "IloCP::setParameter - empty name");
    _setParameter(name, value);
  }
  
  void setParameter(const char * name, const char * value) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(name  != 0, "IloCP::setParameter - empty name");
    IlcCPOAssert(value != 0, "IloCP::setParameter - empty value");
    _setParameter(name, value);
  }
  
  
  IloNum getParameter(const char * name) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(name  != 0, "IloCP::getParameter - empty name");
    return _getParameter(name);
  }
  
  IloNum getParameterDefault(const char * name) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(name  != 0, "IloCP::getParameterDefault - empty name");
    return _getParameterDefault(name);
  }

  
  IloInt getInfo(IloCP::IntInfo info) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return IloInt(_getInfo(IloInt(info)));
  }
  
  IloNum getInfo(IloCP::NumInfo info) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _getInfo(IloInt(info));
  }
  IloNum getInfo(const char * name) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(name  != 0, "IloCP::getNumInfo - empty name");
    return _getInfo(name);
  }

  ///////////////////////////////////////////////////////////////////////////
  // Constructors, extraction and related methods
  ///////////////////////////////////////////////////////////////////////////

  IloCP(const IloEnv env) {
    IlcCPOAssert(env.getImpl() != 0, "IloEnv: empty handle");
    _ctor(env);
  }

  IloCP(const IloModel model) {
    IlcCPOAssert(model.getImpl() != 0, "IloModel: empty handle");
    _ctor(model);
  }
#ifdef CPPREF_GENERATION

void extract (const IloModel model) const;

 IloBool isExtracted(const IloExtractable ext) const;

 void end();
#endif
  
  IloCPI * getImpl() const {
    return (IloCPI*)_impl;
  }
  
  IloCP(IloCPI * impl=0) : IloAlgorithm((IloAlgorithmI *)impl) { }

  ///////////////////////////////////////////////////////////////////////////
  // Solving
  ///////////////////////////////////////////////////////////////////////////
  

  void setSearchPhases(){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _setSearchPhases();
  }
  
  void setSearchPhases(const IloSearchPhase phase){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(phase.getImpl() != 0, "IloSearchPhase: empty handle");
    return _setSearchPhases(phase);
  }
  
  void setSearchPhases(const IloSearchPhaseArray phaseArray){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(phaseArray.getImpl() != 0, "IloSearchPhaseArray: empty handle");
    return _setSearchPhases(phaseArray);
  }

  
  void setStartingPoint(const IloSolution sp) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(sp.getImpl() != 0, "IloSolution: empty handle");
    _setStartingPoint(sp);
  }

  
  void clearStartingPoint() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _clearStartingPoint();
  }
  
  void clearExplanations(){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _clearExplanations();
  }
  
  void explainFailure(IloInt failIndex){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _explainFailure(failIndex);
  }
  
  void explainFailure(IloIntArray failIndexArray){
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(failIndexArray.getImpl() != 0, "IloIntArray: empty handle");
    _explainFailure(failIndexArray);
  }

  
  IloBool solve(const IloGoal goal) const;
  
  IloBool solve() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _solve();
  }

  
  void startNewSearch(const IloGoal goal) const;
   
  void startNewSearch() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _startNewSearch();
  }
  
  
  ///////////////////////////////////////////////////////////////////////////
  // Model manipulation
  ///////////////////////////////////////////////////////////////////////////

  
  void dumpModel(const char* filename) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(filename != 0, "IloCP::dumpModel: empty file name ");
    _dumpModel(filename);
  }
  
  void dumpModel(ILOSTD(ostream)& s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _dumpModel(s);
  }
  
  void exportModel(const char* filename) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(filename != 0, "IloCP::exportModel: empty file name ");
    _exportModel(filename);
  }
  
  void exportModel(ILOSTD(ostream)& s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _exportModel(s);
  }

  
  void importModel(const char* filename) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(filename != 0, "IloCP::AddModel: empty file name");
    _importModel(filename);
  }

  
  void importModel(ILOSTD(istream)& s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _importModel(s);
  }

  
  
  IloModel createDummyConcertModel() const;
  
  void setLocation(IloExtractable e, const char* fileName, int lineNumber) const;

  ///////////////////////////////////////////////////////////////////////////
  // Conflict refiner
  ///////////////////////////////////////////////////////////////////////////
  
  
  IloBool refineConflict() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _refineConflict();
  }
  
  IloBool refineConflict(IloConstraintArray csts) {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(csts.getImpl() != 0, "IloConstraintArray: empty handle");
    return _refineConflict(csts);
  }
  
  IloBool refineConflict(IloConstraintArray csts, IloNumArray prefs) {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(csts.getImpl() != 0, "IloConstraintArray: empty handle");
    IlcCPOAssert(prefs.getImpl() != 0, "IloNumArray: empty handle");
    IlcCPOAssert(csts.getSize()==prefs.getSize(), "IloCP::refineConflict: constraint and preference arrays have different size");
    return _refineConflict(csts, prefs);
  }
  
  IloBool refineConflict(IloSolution sol) {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(sol.getImpl() != 0, "IloSolution: empty handle");
    return _refineConflict(sol);
  }
  
  IloCP::ConflictStatus getConflict(IloConstraint cst) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(cst.getImpl() != 0, "IloConstraint: empty handle");
    IlcCPOAssert(_hasConflict(), "IloCP::getConflict: no available conflict");
    return _getConflict(cst);
  }
  
  IloCP::ConflictStatus getConflict(IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(_hasConflict(), "IloCP::getConflict: no available conflict");
    return _getConflict(var);
  }
  
  IloCP::ConflictStatus getConflict(IloIntervalVar var) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(_hasConflict(), "IloCP::getConflict: no available conflict");
    return _getConflict(var);
  }
  
  IloInt getIntConflict(IloConstraint cst) const {
    return (IloInt)getConflict(cst);
  }
  
  IloInt getIntConflict(IloNumVar var) const {
    return (IloInt)getConflict(var);
  }
  
  IloInt getIntConflict(IloIntervalVar var) const {
    return (IloInt)getConflict(var);
  }

  typedef IloArray<IloCP::ConflictStatus> ConflictStatusArray;

  void getConflictArray(IloConstraintArray& csts, IloCP::ConflictStatusArray& statuses) const {
    IlcCPOAssert(csts.getImpl() != 0, "IloConstraintArray: empty handle");
    IlcCPOAssert(statuses.getImpl() != 0, "IloCP::ConflictStatusArray: empty handle");
    _getConflictArray(csts, statuses);
  }

  
  void writeConflict(ILOSTD(ostream)& str) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasConflict(), "IloCP::writeConflict: no available conflict");
    return _writeConflict(str);
  }

  
  void writeConflict() const {
    writeConflict(out());
  }

  
  void exportConflict(ILOSTD(ostream)& str) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasConflict(), "IloCP::exportConflict: no available conflict");
    return _exportConflict(str);
  }

  
  void runSeeds(IloInt n = 0) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(n >= 0, "IloCP::runSeeds: number of runs should be positive");
    return _runSeeds(n);
  }

private:
  IloBool _refineConflict() const;
  IloBool _refineConflict(IloConstraintArray csts) const;
  IloBool _refineConflict(IloConstraintArray csts, IloNumArray prefs) const;
  IloBool _refineConflict(IloSolution sol) const;
  IloCP::ConflictStatus _getConflict(IloConstraint cst) const;
  IloCP::ConflictStatus _getConflict(IloNumVar var) const;
  IloCP::ConflictStatus _getConflict(IloIntervalVar var) const;
  void _getConflictArray(IloConstraintArray& csts, IloCP::ConflictStatusArray& statuses) const;
  void _writeConflict(ILOSTD(ostream)& str) const;
  void _exportConflict(ILOSTD(ostream)& str) const;
  IloBool _hasConflict() const;
  void _runSeeds(IloInt n) const;
  
public:
  
  IloBool propagate(const IloConstraint constraint = 0) {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _propagate(constraint);
  }




  
private:
  IloIntVarArray                _getAllIloIntVars() const;
  IloIntervalVarArray           _getAllIloIntervalVars() const;
  IloStateFunctionArray         _getAllIloStateFunctions() const;
  IloIntervalSequenceVarArray   _getAllIloIntervalSequenceVars() const;
  IloCumulFunctionExprArray     _getAllConstrainedIloCumulFunctionExprs() const;
  IloBool                       _uses(const IloExtractableI* x) const;
public: 
  static IlcBool IsInteger(const IloNumVarI* var) 
  { return var->getType() != ILOFLOAT; }
  static IlcBool IsInteger(const IloNumVar var)
  { return IsInteger(var.getImpl()); }

  
  
  IloIntVarArray getAllIloIntVars() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getAllIloIntVars();
  }

  
  IloIntervalVarArray getAllIloIntervalVars() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getAllIloIntervalVars();
  }

  
  IloStateFunctionArray getAllIloStateFunctions() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getAllIloStateFunctions();
  }

  
  IloIntervalSequenceVarArray getAllIloIntervalSequenceVars() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getAllIloIntervalSequenceVars();
  }

  
  IloCumulFunctionExprArray getAllConstrainedIloCumulFunctionExprs() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getAllConstrainedIloCumulFunctionExprs();
  }

  
  IloIntVar getIloIntVar(const char* name) const;

  
  IloIntervalVar getIloIntervalVar(const char* name) const;

  
  IloIntervalSequenceVar getIloIntervalSequenceVar(const char* name) const;

  
  IloStateFunction getIloStateFunction(const char* name) const;

  
  IloCumulFunctionExpr getIloCumulFunctionExpr(const char* name) const;
  

  
  class IntVarIterator
#ifndef CPPREF_GENERATION
    : public IlcIntDomainIterator 
#endif
  {
  private:
    void                        _init(const IloCP cp, const IloIntVarI* var);
  public:
    IntVarIterator() { _init(0, 0); }
    IntVarIterator(const IntVarIterator& x)
      : IlcIntDomainIterator(x)
    { }
    
    IntVarIterator(IloCP cp, IloIntVar var) {
      IlcCPOAssert(cp.getImpl() != 0, "IloCP: empty handle");
      IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
      _init(cp, var.getImpl());
    }
    
    IntVarIterator(IloCP cp, IloNumVar var) {
      IlcCPOAssert(cp.getImpl() != 0, "IloCP: empty handle");
      IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
      IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
      _init(cp, IloIntVar(var.getImpl()).getImpl());
    }
    IntVarIterator& operator = (const IntVarIterator& x) 
    { return static_cast<IntVarIterator&>(IlcIntDomainIterator::operator=(x)); }
        
    IntVarIterator& operator++() { 
      IlcCPOAssert(getImpl() != 0, "IloCP::IntVarIterator: empty handle");
      _advance();
      return *this;
    }
    
    IloInt operator*() const { 
      IlcCPOAssert(getImpl() != 0, "IloCP::IntVarIterator: empty handle");
      return _getValue();
    }
    // 2.0b1
    IloAny getAnyValue() const;
    
    IloBool ok() const { 
      IlcCPOAssert(getImpl(), "IloCP::IntVarIterator: empty handle");
      return _ok();
    }

  };
  
  IloNum getValue(const IloObjective obj) const {
    return IloAlgorithm::getValue(obj);
  }

  
  IloNum getValue(const IloNumExprArg expr) const 
  { return IloAlgorithm::getValue(expr); }

  
private:
  
  const IlcEngineState*                 _getState(IlcBool check = IlcTrue) const;
  const IlcEngineI*                     _getEngine() const;
  const IlcCPI*                         _getIlcCPI() const;
  IloCPI*                               _getIloCPI() const;
  IlcBool                               _isAllValid(const IloExtractableArray a) const;
  IlcBool                               _isAllExtracted(const IloExtractableArray a) const;
  void                                  _printDomain(ILOSTD(ostream)& s, const IloIntVarArray vars) const;
  void                                  _printDomain(ILOSTD(ostream)& s, const IloIntervalVar) const;
  void                                  _printDomain(ILOSTD(ostream)& s, const IloNumVar var) const;
  void                                  _printDomain(ILOSTD(ostream)& s, const IloNumVarArray vars) const;
  void                                  _checkExtracted(const IloCumulFunctionExprI* expr, const char* fn) const;
  void                                  _checkExtracted(const IloExtractableI* ilo, const char* fn) const;
  void                                  _checkExtracted(const IlcLaCumulFunction* e, const char* const fn, const char* what, const char* name) const;
  void                                  _checkExtracted(const IlcLaObject* layer, const char* fn, const char* what, const char* name) const;

  const IlcLaObject*                    _getLayerObject(const char* name, const char* fn) const;
  const IlcLaIntVar*                    _getLayerIntVar(const char* name, const char* fn) const;
  const IlcLaFloatVar*                  _getLayerFloatVar(const char* name, const char* fn) const;
  const IlcLaIntervalVar*               _getLayerInterval(const char* name, const char* fn) const;
  const IlcLaSequenceVar*               _getLayerSequence(const char* name, const char* fn) const;
  const IlcLaCumulFunction*             _getLayerCumulFunction(const char* name, const char* fn) const;
  const IlcLaStateFunction*             _getLayerStateFunction(const char* name, const char* fn) const;
  const IlcLaObject*                    _getLayerObject(const IloExtractableI* o, const char* fn) const;
  const IlcLaIntVar*                    _getLayerIntVar(const IloNumVarI* v, const char* fn) const;
  const IlcLaFloatVar*                  _getLayerFloatVar(const IloNumVarI* v, const char* fn) const;
  const IlcLaIntervalVar*               _getLayerInterval(const IloIntervalVarI* v, const char* fn) const;
  const IlcLaSequenceVar*               _getLayerSequence(const IloIntervalSequenceVarI* v, const char* fn) const;
  const IlcLaCumulFunction*             _getLayerCumulFunction(const IloCumulFunctionExprI* v, const char* fn) const;
  const IlcLaStateFunction*             _getLayerStateFunction(const IloStateFunctionI* v, const char* fn) const;
  const IlcLaStateFunction*             _getLayerStateFunction(const IloStateFunctionExprI* v, const char* fn) const;

  
  void          _getBounds(const IloIntVar var, IloInt& min, IloInt& max) const;
  IloInt        _getValue(const char* intVarName) const;
  IloBool       _isPresent(const char* intervalVarName) const;
  IloBool       _isAbsent(const char* intervalVarName) const;
  IloInt        _getStart(const char* intervalVarName) const;
  IloInt        _getEnd(const char* intervalVarName) const;
  IloInt        _getSize(const char* intervalVarName) const;
  IloInt        _getLength(const char* intervalVarName) const;
  IloInt        _getDomainSize(const IloNumVar var) const;
  IloBool       _isInDomain(const IloIntVar var, IloInt value) const;
  void          _store(const IloSolution solution) const;
  void          _storeInternal(IloSolutionI* solution) const;
  IlcBool       _restore(const IloSolution solution) const;
  IlcBool       _restoreInternal(const IloSolutionI* solution) const;

  IloInt        _getMax(const IloIntVar var) const;
  IloNum        _getMax(const IloNumVar var) const;
  IloInt        _getMin(const IloIntVar var) const;
  IloNum        _getMin(const IloNumVar var) const;
  IloNum        _getValue(const IloNumVar var) const;
  IloInt        _getValue(const IloIntVar var) const;
  void          _getObjValues(const IloNumArray values) const;
  IloNum        _getObjValue(IloInt index) const;
  void          _getObjBounds(const IloNumArray values) const;  
  IloNum        _getObjBound(IloInt index) const;
  void          _getObjGaps(const IloNumArray values) const;  
  IloNum        _getObjGap(IloInt index) const;
  IloNum        _getObjMin() const;
  IloNum        _getObjMin(IloInt index) const;
  IloNum        _getObjMax() const;
  IloNum        _getObjMax(IloInt index) const;
  IloInt        _getNumberOfCriteria() const;
  IloBool       _isFixed(const IloIntVar var) const;
  IloBool       _isFixed(const IloNumVar var) const;
  
  IloBool       _isFixed      (const IloIntervalVar a) const;  
  IloBool       _isPresent    (const IloIntervalVar a) const;
  IloBool       _isAbsent     (const IloIntervalVar a) const; 
  IloInt        _getStartMin  (const IloIntervalVar a) const;
  IloInt        _getStartMax  (const IloIntervalVar a) const; 
  IloInt        _getStart     (const IloIntervalVar a) const; 
  IloInt        _getEndMin    (const IloIntervalVar a) const;
  IloInt        _getEndMax    (const IloIntervalVar a) const; 
  IloInt        _getEnd       (const IloIntervalVar a) const; 
  IloInt        _getSizeMin   (const IloIntervalVar a) const;
  IloInt        _getSizeMax   (const IloIntervalVar a) const; 
  IloInt        _getSize      (const IloIntervalVar a) const; 
  IloInt        _getLengthMin (const IloIntervalVar a) const;
  IloInt        _getLengthMax (const IloIntervalVar a) const; 
  IloInt        _getLength    (const IloIntervalVar a) const; 

  
  IloBool               _isFixed      (const IloIntervalSequenceVar seq) const;  
  IloIntervalVarI*      _getIloIntervalVar(const IlcLaIntervalVar* v) const;
  IloIntervalVar        _getFirst(const IloIntervalSequenceVar seq) const;
  IloIntervalVar        _getLast (const IloIntervalSequenceVar seq) const;
  IloIntervalVar        _getNext (const IloIntervalSequenceVar seq, const IloIntervalVar a) const;
  IloIntervalVar        _getPrev (const IloIntervalSequenceVar seq, const IloIntervalVar a) const;
  IloBool               _isInSequence (const IloIntervalSequenceVar seq, const IloIntervalVar a) const;

  
  IloBool       _isFixed(const IloCumulFunctionExpr cumul) const;
  IloInt        _getNumberOfSegments(const IloCumulFunctionExpr cumul) const;
  IloBool       _isValidSegment(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloInt        _getSegmentStart(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloInt        _getSegmentEnd(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloInt        _getSegmentValue(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloBool       _isValidAbscissa(const IloCumulFunctionExpr cumul, IloInt a) const;
  IloInt        _getValue(const IloCumulFunctionExpr cumul, IloInt a) const;
  IloInt        _getHeightAtFoo(const IloIntervalVar var, const IloCumulFunctionExpr cumul, IloBool atStart, const char* fn) const;

  
  IloNum        _getNumberOfSegmentsAsNum(const IloCumulFunctionExpr cumul) const;
  IloNum        _getSegmentStartAsNum(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloNum        _getSegmentEndAsNum(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloNum        _getSegmentValueAsNum(const IloCumulFunctionExpr cumul, IloInt s) const;
  IloNum        _getValueAsNum(const IloCumulFunctionExpr cumul, IloInt a) const;
  
  
  IloBool       _isFixed(const IloStateFunction f) const;
  IloInt        _getNumberOfSegments(const IloStateFunction f) const;
  IloBool       _isValidSegment(const IloStateFunction f, IloInt s) const;
  IloInt        _getSegmentStart(const IloStateFunction f, IloInt s) const;
  IloInt        _getSegmentEnd(const IloStateFunction f, IloInt s) const;
  IloInt        _getSegmentValue(const IloStateFunction f, IloInt s) const;
  IloBool       _isValidAbscissa(const IloStateFunction f, IloInt a) const;
  IloInt        _getValue(const IloStateFunction f, IloInt a) const;

  
  IloNum        _getNumberOfSegmentsAsNum(const IloStateFunction f) const;
  IloNum        _getSegmentStartAsNum(const IloStateFunction f, IloInt s) const;
  IloNum        _getSegmentEndAsNum(const IloStateFunction f, IloInt s) const;
  IloNum        _getSegmentValueAsNum(const IloStateFunction f, IloInt s) const;
  IloNum        _getValueAsNum(const IloStateFunction f, IloInt a) const;

  
  IloInt        _getNumberOfSegments(const IloStateFunctionExpr expr) const;
  IloInt        _getSegmentStart(const IloStateFunctionExpr expr, IloInt s) const;
  IloInt        _getSegmentEnd(const IloStateFunctionExpr expr, IloInt s) const;
  IloInt        _getSegmentValue(const IloStateFunctionExpr expr, IloInt s) const;
  IloInt        _getValue(const IloStateFunctionExpr expr, IloInt a) const;

  IloBool       _hasObjective() const;
  IloBool       _isAllFixed() const;

public:
  IloBool isAllValid(const IloExtractableArray ext) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(ext.getImpl() != 0, "IloExtractableArray: empty handle");
    return _isAllValid(ext);
  }

  
  
  IloBool hasObjective() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _hasObjective();
  }

  
  IloInt getValue(const char* intVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::getValue: empty handle.");
    IlcCPOAssert(intVarName, "IloCP::getValue: null pointer argument.");
    return _getValue(intVarName);
  }

  
  IloBool isPresent(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::isPresent: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::isPresent: null pointer argument.");
    return _isPresent(intervalVarName);
  }

  
  IloBool isAbsent(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::isAbsent: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::isAbsent: null pointer argument.");
    return _isAbsent(intervalVarName);
  }

  
  IloInt getStart(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::getStart: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::getStart: null pointer argument.");
    return _getStart(intervalVarName);
  }

  
  IloInt getEnd(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::getEnd: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::getEnd: null pointer argument.");
    return _getEnd(intervalVarName);
  }

  
  IloInt getSize(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::getSize: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::getSize: null pointer argument.");
    return _getSize(intervalVarName);
  }

  
  IloInt getLength(const char* intervalVarName) const {
    IlcCPOAssert(getImpl() != 0, "IloCP::getLength: empty handle.");
    IlcCPOAssert(intervalVarName, "IloCP::getLength: null pointer argument.");
    return _getLength(intervalVarName);
  }

  
  void store(const IloSolution solution) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(solution.getImpl() != 0, "IloSolution: empty handle");
    _store(solution);
  }

  
  IloBool restore(const IloSolution solution) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(solution.getImpl() != 0, "IloSolution: empty handle");
    return _restore(solution);
  }
  
  void printDomain(const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    _printDomain(out(), var);                                           
  }
  void printDomain(ILOSTD(ostream)& s, const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    _printDomain(s, var);
  }
  void printDomain(ILOSTD(ostream)& s, const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    _printDomain(s, vars);
  }
  void printDomain(const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    _printDomain(out(), vars);
  }
  void printDomain(ILOSTD(ostream)& s, const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloIntVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloIntVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloIntVarArray: element not extracted");
    _printDomain(s, vars);
  }
  void printDomain(const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloIntVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloIntVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloIntVarArray: element not extracted");
    _printDomain(out(), vars);
  }

  class PrintDomains {
  protected:
    const IloCP& _cp;
    IloInt                  _n;
    IloBool                 _singleton;
    IloExtractableI **      _var;

    PrintDomains(const IloCP& cp, const IloExtractable ext);
    PrintDomains(const IloCP& cp, const IloExtractableArray ext);
    PrintDomains(const PrintDomains&);
    const IlcEngineState* getState() const; 
  public:
    ~PrintDomains();
  private:
    PrintDomains& operator = (PrintDomains& pd) { return *this; }
  };

  class PrintNumVarDomains : public PrintDomains {
    friend class IloCP;
  private:
    PrintNumVarDomains(const IloCP& cp, const IloNumVar var);
    PrintNumVarDomains(const IloCP& cp, const IloNumVarArray var);
    PrintNumVarDomains(const IloCP& cp, const IloIntVarArray var);
  public:
    void display(ILOSTD(ostream)& o) const;
  private:
    PrintNumVarDomains& operator = (const IloCP::PrintNumVarDomains&) { return *this; }
  };

  
  PrintNumVarDomains domain(const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    return PrintNumVarDomains(*this, var);
  }
  
  PrintNumVarDomains domain(const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    return PrintNumVarDomains(*this, vars);
  }
  
  PrintNumVarDomains domain(const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloIntVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloIntVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloIntVarArray: element not extracted");
    return PrintNumVarDomains(*this, vars);
  }

#ifdef ILC_COMPILE_DEAD_CODE
  ///////////////////////////////////////////////////////////////////////////
  // Hooks
  ///////////////////////////////////////////////////////////////////////////
  void setNodeHook(IloCPHookI * hook = 0) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _setNodeHook(hook);
  }
#endif

  
  ///////////////////////////////////////////////////////////////////////////
  // Getting solution information
  ///////////////////////////////////////////////////////////////////////////
  // Mimic IloAlgorithm as CP Optimizer has its own getValue functions
  
  void getObjValues(const IloNumArray values) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(values.getImpl() != 0, "IloNumArray: empty handle");
    return _getObjValues(values);
  }
  
  IloInt getNumberOfCriteria() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getNumberOfCriteria();
  }
  
  IloNum getObjValue(IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(i >= 0, "IloCP: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCP: Objective value index is too large");
    return _getObjValue(i);
  }

  IloNum getObjValue() const { return IloAlgorithm::getObjValue(); }
  
  void getObjBounds(const IloNumArray values) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(values.getImpl() != 0, "IloNumArray: empty handle");
    _getObjBounds(values);
  }

  
  IloNum getObjBound() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getObjBound(0);
  }

  
  IloNum getObjBound(IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(i >= 0, "IloCP: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCP: Objective bound index is too large");
    return _getObjBound(i);
  }

  
  void getObjGaps(const IloNumArray values) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(values.getImpl() != 0, "IloNumArray: empty handle");
    _getObjGaps(values);
  }
  
  IloNum getObjGap() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getObjGap(0);
  }

  
  IloNum getObjGap(IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(i >= 0, "IloCP: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCP: Objective gap index is too large");
    return _getObjGap(i);
  }
  // End of part only for IloCP
  

  IloNum getObjMin() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCP: No objective present");
    return _getObjMin();
  }
  IloNum getObjMin(IloInt i) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCP: No objective present");
    IlcCPOAssert(i >= 0, "IloCP: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCP: Objective value index is too large");
    return _getObjMin(i);
  }
  IloNum getObjMax() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCP: No objective present");
    return _getObjMax();
  }
  IloNum getObjMax(IloInt i) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCP: No objective present");
    IlcCPOAssert(i >= 0, "IloCP: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCP: Objective value index is too large");
    return _getObjMax(i);
  }

  
  IloNum getValue(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
     IlcCPOAssert(isFixed(v), "IloNumVar: not fixed");  
    return _getValue(v);
  }
  
  IloInt getValue(const IloIntVar v) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloIntVar: not extracted");
     IlcCPOAssert(isFixed(v), "IloIntVar: not fixed");  
    return _getValue(v);
  }
  // 2.0b1
  IloAny getAnyValue(const IloIntVar v) const { return (IloAny)getValue(v); }

  
  IloNum getMin(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
    return _getMin(v);
  }
  
  IloNum getMax(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
    return _getMax(v);
  }
  
  IloInt getMax(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getMax(var);
  }
  
  IloInt getMin(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getMin(var);
  }
  void getBounds(const IloIntVar var, IloInt& min, IloInt& max) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    _getBounds(var, min, max);
  }
   
  IloBool isInDomain(const IloNumVar var, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return _isInDomain(IloIntVar(var.getImpl()), value);
  }
  
  IloBool isInDomain(const IloIntVar var, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _isInDomain(var, value);
  }
  
  IloInt getDomainSize(const IloNumVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return _getDomainSize(var);
  }
  
  IloInt getDomainSize(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getDomainSize(var);
  }
  
  IloBool isFixed(const IloNumVar var) const  {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    return _isFixed(var);
  }
  
  IloBool isFixed(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _isFixed(var);
  }
  
  IloBool isAllExtracted(const IloExtractableArray ext) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(ext.getImpl() != 0, "IloExtractableArray: empty handle");
    return _isAllExtracted(ext);
  }
  
  IloBool isAllFixed() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _isAllFixed();
  }


  ///////////////////////////////////////////////////////////////////////////
  // Advanced: Ilc mapping
  //           No inlining here to avoid Ilc/Ilo world crossover
  ///////////////////////////////////////////////////////////////////////////
  
   IloBool isFixed(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isFixed(a);
  }

 
  IloBool isPresent(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isPresent(a);
  }
 
  IloBool isAbsent(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isAbsent(a);
  }

  
  IloInt getStartMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getStartMin(a);
  }
  
  IloInt getStartMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getStartMax(a);
  }

  
  IloInt getStart(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getStart(a);
  }

  
  IloInt getEndMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getEndMin(a);
  }
  
  IloInt getEndMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getEndMax(a);
  }

  
  IloInt getEnd(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getEnd(a);
  }

  
  IloInt getSizeMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getSizeMin(a);
  }
  
  IloInt getSizeMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getSizeMax(a);
  }

  
  IloInt getSize(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getSize(a);
  }

  
  IloInt getLengthMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getLengthMin(a);
  }
  
  IloInt getLengthMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getLengthMax(a);
  }

  
  IloInt getLength(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getLength(a);
  }
  void printDomain(ILOSTD(ostream)& s, const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    _printDomain(s, a);
  }
  void printDomain(const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle.");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    _printDomain(out(), a);
  }

  
  IloBool isFixed(const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
    return _isFixed(seq);
  }

  
  IloIntervalVar getFirst(const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    return _getFirst(seq);
  }
  
  IloIntervalVar getLast (const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    return _getLast(seq);
  }
  
  IloIntervalVar getNext(const IloIntervalSequenceVar seq, const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isPresent(a), "IloIntervalVar: not present.");
    IlcCPOAssert(isInSequence(seq, a), "IloIntervalVar: not in sequence variable.");
     return _getNext(seq, a);
  }
  
  IloIntervalVar getPrev (const IloIntervalSequenceVar seq, const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isPresent(a), "IloIntervalVar: not present.");
    IlcCPOAssert(isInSequence(seq, a), "IloIntervalVar: not in sequence variable.");
    return _getPrev(seq, a);
  }


  
  IloBool isFixed(const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCumulFunctionExpr: not extracted");
    return _isFixed(f);
  }

  
  IloInt getNumberOfSegments(const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    return _getNumberOfSegments(f);
  }

  
  IloNum getNumberOfSegmentsAsNum(const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    return _getNumberOfSegmentsAsNum(f);
  }

  
  IloInt getSegmentStart(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentStart(f, i);
  }
  
  IloNum getSegmentStartAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentStartAsNum(f, i);
  }

  
  IloInt getSegmentEnd(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentEnd(f, i);
  }
  
  IloNum getSegmentEndAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentEndAsNum(f, i);
  }

  
  IloInt getSegmentValue(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentValue(f, i);
  }
  
  IloNum getSegmentValueAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCP: invalid cumul function expression segment");
    return _getSegmentValueAsNum(f, i);
  }

  
  IloInt getValue(const IloCumulFunctionExpr f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCP: cumul function expression evaluated on invalid point");
    return _getValue(f, t);
  }
  
  IloInt getHeightAtStart(const IloIntervalVar var, const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(isExtracted(var), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isFixed(var), "IloIntervalVar: not fixed.");  
    return _getHeightAtFoo(var, f, IloTrue, "IloCP::getHeightAtStart");
  }
  
  IloInt getHeightAtEnd(const IloIntervalVar var, const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(isExtracted(var), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isFixed(var), "IloIntervalVar: not fixed.");  
    return _getHeightAtFoo(var, f, IloFalse, "IloCP::getHeightAtEnd");
  }
  
  IloNum getValueAsNum(const IloCumulFunctionExpr f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCP: cumul function expression not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCP: cumul function expression evaluated on invalid point");
    return _getValueAsNum(f, t);
  }

  ////////////////////////////////////////////////////////////////////////
  // Reading State Functions at solution
  ////////////////////////////////////////////////////////////////////////
  
  
  IloBool isFixed(const IloStateFunction f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloStateFunctionFunction: not extracted");
    return _isFixed(f);
  }
  
  
  IloInt getNumberOfSegments(const IloStateFunction f) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    return _getNumberOfSegments(f);
  }


  
  IloInt getSegmentStart(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentStart(f, s);
  }
  
  IloNum getSegmentStartAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentStartAsNum(f, s);
  }
  
  
  IloInt getSegmentEnd(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentEnd(f, s);
  }
  
  IloNum getSegmentEndAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentEndAsNum(f, s);
  }
 
  
  IloInt getSegmentValue(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentValue(f, s);
  }
  
  IloNum getSegmentValueAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCP: invalid state function segment");
    return _getSegmentValueAsNum(f, s);
  }
  
  IloInt getValue(const IloStateFunction f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCP: state function evaluated on invalid point");
    return _getValue(f, t);
  }
  
  IloNum getValueAsNum(const IloStateFunction f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCP: state function not extracted");
    IlcCPOAssert(_isFixed(f), "IloCP: state function not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCP: state function evaluated on invalid point");
    return _getValueAsNum(f, t);
  }
 
   
  ///////////////////////////////////////////////////////////////////////////
  // class IloStateFunctionExpr
  ///////////////////////////////////////////////////////////////////////////

  
  IloInt getNumberOfSegments(const IloStateFunctionExpr expr) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getNumberOfSegments(expr);
  }


  
  IloInt getSegmentStart(const IloStateFunctionExpr expr, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getSegmentStart(expr, i);
  }



  
  IloInt getSegmentEnd(const IloStateFunctionExpr expr, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getSegmentEnd(expr, i);
  }


  
  IloInt getSegmentValue(const IloStateFunctionExpr expr, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getSegmentValue(expr, i);
  }

  
  IloInt getValue(const IloStateFunctionExpr expr, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getValue(expr, t);
  }
  

  ///////////////////////////////////////////////////////////////////////////
  // Printing interval domains
  ///////////////////////////////////////////////////////////////////////////
  class PrintIntervalVarDomains : public PrintDomains {
    friend class IloCP;
  private:
    PrintIntervalVarDomains(const IloCP& cp, const IloIntervalVar var);
  public:
    void display(ILOSTD(ostream)& o) const;
  private:
    PrintIntervalVarDomains& operator = (const PrintIntervalVarDomains&) { return *this; }
  };

  
  PrintIntervalVarDomains domain(const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted");
    return PrintIntervalVarDomains(*this, a);
  }

  // ------------------------------------------------------------------------
  // Advanced
  // ------------------------------------------------------------------------
  IloBool isInSequence (const IloIntervalSequenceVar seq,
                        const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    IlcCPOAssert(seq.getImpl() != 0, "IloIntervalSequenceVar: empty handle");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle");
    return _isInSequence(seq, a);
  }

  
  IloCP::IntVarIterator iterator(const IloIntVar var) {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    return IloCP::IntVarIterator(*this, var);
  }
  
  IloCP::IntVarIterator iterator(const IloNumVar var) {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return IloCP::IntVarIterator(*this, var);
  }
 
  
  
private:
  IloNum _getRandomNum() const;
  IloInt   _getRandomInt(IloInt n) const;

public:
  
  IlcRandom getRandom() const;
  
  IloInt getRandomInt(IloInt n) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(n >= 1, "IloCP: getRandomInt(n): n < 1");
    return _getRandomInt(n);
  }
  
  IloNum getRandomNum() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getRandomNum();
  }


  private:
  void _printInformation() const;
  void _printInformation(ILOSTD(ostream) & o) const;
  void _printPortableInformation() const;
  void _printPortableInformation(ILOSTD(ostream) & o) const;
  void _printModelInformation() const;
  void _printModelInformation(ILOSTD(ostream) & o) const;

  public:
  ///////////////////////////////////////////////////////////////////////////
  // Solving
  ///////////////////////////////////////////////////////////////////////////
  
  void printInformation() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printInformation();
  }
  
  void printInformation(ILOSTD(ostream)& stream) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printInformation(stream);
  }

  void printPortableInformation() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printPortableInformation();
  }
  void printPortableInformation(ILOSTD(ostream)& stream) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printPortableInformation(stream);
  }

  void printModelInformation() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printModelInformation();
  }
  void printModelInformation(ILOSTD(ostream)& stream) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printModelInformation(stream);
  }
  
  IloBool next() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    return _next();
  }
  
  void endSearch() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _endSearch();
  }

 


  
  const char* getVersion() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getVersion();
  }
  static const char* GetVersion();



  void setInferenceLevel(IloConstraint ct, IloInt level) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    IlcCPOAssert(ct.getImpl() != 0, "IloConstraint: empty handle");
    _setInferenceLevel(ct, level);
  }
  IloInt getInferenceLevel(IloConstraint ct) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getInferenceLevel(ct);
  }
  void resetInferenceLevels() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _resetConstraintInferenceLevels();
  }

  
  void abortSearch() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _abortSearch();
  }
  
  void clearAbort() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _clearAbort();
  }


  
  void prettyPrintState(ILOSTD(ostream)& s) const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    _prettyPrintState(s);
  }
  void prettyPrintState() const 
  { prettyPrintState(out()); }

  


 
  ///////////////////////////////////////////////////////////////////////////
  // Exceptions
  ///////////////////////////////////////////////////////////////////////////
 
 class Exception : public IloAlgorithm::Exception {
    IloInt _status;
  public:
    Exception(int status, const char* str);
    IloInt getStatus() const { return _status; }
  };
  class NoLicense : public Exception {
  public:
    NoLicense(const char* msg)
      : IloCP::Exception(-1, msg) { }
  };

  class MultipleObjException : public Exception {
    IloObjective _obj;
  public:
    MultipleObjException(IloObjective obj)
      :IloCP::Exception(-1, "IloCP can not handle more than one objective object")
      , _obj(obj) {}
    IloObjective getObj() const { return _obj; }
  };

  class GetObjValueNotSupportedException : public Exception {
    IloObjective _obj;
  public:
    GetObjValueNotSupportedException()
      :IloCP::Exception(-1, "IloCP::getValue only supported for "
                            "simple or static lexicographic objectives") { }
  };

  class MultipleSearchException : public Exception {
  public:
    MultipleSearchException()
      : IloCP::Exception(-1, "IloCP can not handle multiple searches"){
    }
  };

  class InvalidDiffException : public Exception {
    IloDiff _diff;
  public:
    InvalidDiffException(IloDiff diff)
      : IloCP::Exception(-1, "Invalid IloDiff constraint for IloCP (can only uses integer expressions)")
      , _diff(diff) {}
    IloDiff getDiff() const { return _diff; }
  };

  class InvalidSequenceConstraintException : public Exception {
    IloIntervalSequenceVar _seq;
    IloIntervalVar _itv;
  public:
    InvalidSequenceConstraintException(IloIntervalSequenceVar seq, IloIntervalVar itv)
      : IloCP::Exception(-1, "Invalid IloFirst, IloLast, IloBefore or IloPrevious constraint for IloCP (constraint must hold on interval variables of the sequence)")
      , _seq(seq), _itv(itv) {}
    IloIntervalSequenceVar getSequenceVariable() const { return _seq; }
    IloIntervalVar getIntervalVariable() const { return _itv; }
  };

  class SolverErrorException : public Exception {
    const char* _function;
    IloInt _errorType;
  public:
    SolverErrorException(const char* function, const char* str, IloInt er)
      : IloCP::Exception(-1, str), _function(function),
      _errorType(er) {}
    virtual void print(ILOSTD(ostream)& o) const;
    const char* getFunction() const { return _function; }
    IloInt getErrorType() const { return _errorType; }
  };

  class SolverErrorExceptionInt : public SolverErrorException {
    const IloInt _value;
  public:
    SolverErrorExceptionInt(const char* function,
                            const char* str,
                            IloInt e,
                            IloInt value) :
      IloCP::SolverErrorException(function, str, e), _value(value) {}

    virtual void print(ILOSTD(ostream)& o) const;
    IloInt getValue() const { return _value; }
  };
  class SolverErrorExceptionIntInt : public SolverErrorExceptionInt {
    const IloInt _value2;
  public:
    SolverErrorExceptionIntInt(const char* function,
                               const char* str,
                               IloInt e,
                               IloInt value,
                               IloInt value2) :
      IloCP::SolverErrorExceptionInt(function, str, e, value),
      _value2(value2) {}

    virtual void print(ILOSTD(ostream)& o) const;
    IloInt getValue2() const { return _value2; }
  };

  class SolverErrorExceptionFloat : public SolverErrorException {
    const IloNum _value;
  public:
    SolverErrorExceptionFloat(const char* function,
                            const char* str,
                            IloInt e,
                            IloNum value) :
      IloCP::SolverErrorException(function, str, e), _value(value) {}

    virtual void print(ILOSTD(ostream)& o) const;
    IloNum getValue() const { return _value; }
  };

  class SolverErrorExceptionAny : public SolverErrorException {
    const IloAny _value;
  public:
    SolverErrorExceptionAny(const char* function,
                            const char* str,
                            IloInt e,
                            IloAny value) :
      IloCP::SolverErrorException(function, str, e), _value(value) {}

    virtual void print(ILOSTD(ostream)& o) const;
    IloAny getValue() const { return _value; }
  };

  class SolverErrorExceptionExprI : public SolverErrorException {
    const void* _exprI;
  public:
    SolverErrorExceptionExprI(const char* function,
                              const char* str,
                              IloInt e,
                              const IlcExprI* expr) :
      IloCP::SolverErrorException(function, str, e), _exprI(expr) {}

    virtual void print(ILOSTD(ostream)& o) const;
    const IlcExprI* getExprI() const { return (IlcExprI*)_exprI; }
  };

  class SolverErrorExceptionExprsI : public SolverErrorExceptionExprI {
    const void* _exprI2;
  public:
    SolverErrorExceptionExprsI(const char* function,
                              const char* str,
                              IloInt e,
                              const IlcExprI* expr,
                              const IlcExprI* expr2) :
      IloCP::SolverErrorExceptionExprI(function, str, e, expr),
      _exprI2(expr2){}

   virtual void print(ILOSTD(ostream)& o) const;
   const IlcExprI* getExprI2() const { return (IlcExprI*)_exprI2; }
  };

  class UnimplementedFeature : public Exception {
  public:
    UnimplementedFeature(const char* message) :
      IloCP::Exception(-1, message) {}
  };

  class ObjectNotExtracted : public Exception {
  private:
    const IloCPI * _cp;
    IloExtractable _ex;
  public:
    ObjectNotExtracted(IloCP cp, IloExtractable ex, const char* message = 0);
    ObjectNotExtracted(const IloCPI * cp, IloExtractable ex, const char* message = 0);
    const IloAlgorithmI * getAlgorithmI() const { return (IloAlgorithmI*)_cp; }
    IloExtractable getExtractable() const { return _ex; }
    IloCP getCP() const;
    virtual void print(ILOSTD(ostream)&) const;
  };

  class ModelNotExtracted : public Exception {
  public:
    ModelNotExtracted() : IloCP::Exception(-1, "Model is not loaded") {}
  };

  class BadParameterType : public Exception {
  public:
    BadParameterType(const char* message) :IloCP::Exception(-1, message) {}
  };

  class NumIsNotInteger : public Exception {
  public:
    NumIsNotInteger() :IloCP::Exception(-1, "IloNum is not integer") {}
  };

  class NumIsNotBoolean : public Exception {
  public:
    NumIsNotBoolean() :IloCP::Exception(-1, "IloNum is not boolean") {}
  };

  class IntegerOverflow : public Exception {
  public:
    IntegerOverflow() :IloCP::Exception(-1, "IloNum is out of integer range") {}
  };

  class MixedTypeVariableArray : public Exception {
  public:
    MixedTypeVariableArray(const char* message) :IloCP::Exception(-1, message) {}
  };

  class ArgumentOutOfRange : public Exception {
  public:
    ArgumentOutOfRange(const char* message) :
      IloCP::Exception(-1, message) {}
    virtual void print(ILOSTD(ostream)&) const;
  };

  class VariableShouldBeInteger : public Exception {
  public:
    VariableShouldBeInteger(const char* message) :
      IloCP::Exception(-1, message) {}
    virtual void print(ILOSTD(ostream)&) const;
  };

  class VariableShouldBeFloat : public Exception {
  public:
    VariableShouldBeFloat(const char* message) :
      IloCP::Exception(-1, message) {}
    virtual void print(ILOSTD(ostream)&) const;
  };

  class WrongContext : public Exception {
  public:
    WrongContext(const char* message) :
      IloCP::Exception(-1, message) {}
  };
  class WrongType : public Exception {
  public:
    WrongType(const char* message) :
      IloCP::Exception(-1, message) {}
  };
  class WrongUsage : public Exception {
  public:
    WrongUsage(const char* message) :
      IloCP::Exception(-1, message) {}
  };

  class EmptyHandle : public Exception {
  public:
    EmptyHandle(const char* message) :
      IloCP::Exception(-1, message) {}
  };

  class SizeMustBePositive : public Exception {
  public:
    SizeMustBePositive(const char* message) : IloCP::Exception(-1, message) {}
  };

  class ModelInconsistent : public Exception {
    IloExtractableI* _extractable;
  public:
    ModelInconsistent(IloExtractableI* ext) :IloCP::Exception(-1, "The loaded model is inconsistent"), _extractable(ext) {}
    IloExtractable getExtractable() const { return _extractable; }
    virtual void print(ILOSTD(ostream)&) const;
    virtual const char* getInconsistencyReason() const;
  };

  class IntervalInconsistent : public ModelInconsistent {
  public:
    enum Reason {
      StartRange,
      EndRange,
      SizeRange,
      LengthRange,
      Window
    };
    IntervalInconsistent(IloExtractableI* ext, Reason r) :IloCP::ModelInconsistent(ext), _reason(r) {}
    virtual const char* getInconsistencyReason() const;
  private:
    Reason _reason;
  };

  class StateFunctionNoTriangularInequality : public Exception {
    const IloStateFunctionI* _sf;
    IloInt _i;
    IloInt _j;
    IloInt _k;
  public:
    StateFunctionNoTriangularInequality(const IloStateFunctionI* sf, 
                                        IloInt i =-1, IloInt j =-1, IloInt k =-1)
      :IloCP::Exception(-1, "Transition distance matrix does not satisfy the triangular inequality")
      ,_sf(sf), _i(i), _j(j), _k(k) {}
    virtual void print(ILOSTD(ostream)& o) const;
    const IloStateFunctionI* getStateFunction() const { return _sf; }
    IloInt getI() const { return _i; }
    IloInt getJ() const { return _j; }
    IloInt getK() const { return _k; }
  };
  
  class PropagatorException : public Exception {
  public:
    PropagatorException(const char* message) :
      IloCP::Exception(-1, message) {}
  };

  class ParameterCannotBeSetHereException : public Exception {
  public:
    ParameterCannotBeSetHereException(const char* message) :
      IloCP::Exception(-1, message) {}
  };

  class MetaConstraintNotAllowed : public Exception {
  public:
    MetaConstraintNotAllowed() :
      IloCP::Exception(-1, "Global constraints (for example: constraints on arrays) cannot be used in meta-constraints.") {}
  };

  class NoSuchXException : public Exception {
    private:
      const char * _x;
      void _ctor(const char *);
    public:
      NoSuchXException(const NoSuchXException &ex)
        : Exception(ex) { _ctor(ex._x); }
      NoSuchXException& operator = (const NoSuchXException & ex) {
        *((Exception*)this) = (const Exception&)ex;
        _ctor(ex._x);
        return *this;
      }
      NoSuchXException(const char * what, const char * x)
        : Exception(-1, what) { _ctor(x); }
      ~NoSuchXException();
      void print(ILOSTD(ostream)&) const;
  };

  class NoSuchParameterException : public NoSuchXException {
    public:
      NoSuchParameterException(const char * param)
        : NoSuchXException("parameter", param) { }
  };
  class NoSuchParameterValueException : public NoSuchXException {
    public:
      NoSuchParameterValueException(const char * paramValue)
        : NoSuchXException("parameter value", paramValue) { }
  };
  class NoSuchInfoException : public NoSuchXException {
    public:
      NoSuchInfoException(const char * info)
        : NoSuchXException("info", info) { }
  };

  class ConflictRefinerException : public Exception {
  public:
    ConflictRefinerException(const char* msg) :IloCP::Exception(-1, msg) {}
  };
  
  class ConflictRefinerNotAddedCt : public ConflictRefinerException {
  public:
  ConflictRefinerNotAddedCt(IloConstraint ct)
  :ConflictRefinerException("Constraint was not added to the model: "), _ct(ct){}
  virtual void print(ILOSTD(ostream)&) const;
  private:
    IloConstraint _ct;
  };

  
  class PresolveException: public Exception {
   private:
    void* _store;
    char* _aggregatedMessage;

    // PresolveException cannot be assigned:
    PresolveException & operator = (const PresolveException &);
    void initCounter();
    void incrCounter();

   public:
    PresolveException(IlcLaMessageStore* store):
      IloCP::Exception(-1, NULL),
      _store((void*)store),
      _aggregatedMessage(NULL)
    {
      initCounter();
    }
    // All exceptions must be copyable
    PresolveException(const PresolveException &ex):
      IloCP::Exception(-1, NULL),
      _store(ex._store),
      _aggregatedMessage(NULL)
    {
      incrCounter();
    }
    ~PresolveException();
    virtual const char* getMessage() const;
    
    class Iterator;
    friend class Iterator;
    class Iterator {
     private:
      void* _store;
      IloInt _position;
      IloUInt _getMsgCode() const;
      IloExtractable _getExtractable() const;
      const char* _getMessage() const;
      const char* _getCPOModelPart() const;
     public:
      Iterator(const PresolveException* exception):
        _store((IlcLaMessageStore*)exception->_store),
        _position(0)
      {}
      IloBool ok() const;
      void operator++() {
        IlcCPOAssert(ok(), "Invalid state of the iterator");
        _position++;
      }
      IloUInt getMsgCode() const {
        IlcCPOAssert(ok(), "Invalid state of the iterator");
        return _getMsgCode();
      }
      IloExtractable getExtractable() const {
        IlcCPOAssert(ok(), "Invalid state of the iterator");
        return _getExtractable();
      }
      const char* getMessage() const {
        IlcCPOAssert(ok(), "Invalid state of the iterator");
        return _getMessage();
      }
      
      const char* getCPOModelPart() const {
        IlcCPOAssert(ok(), "Invalid state of the iterator");
        return _getCPOModelPart();
      }
    };

    Iterator getIterator() const { return Iterator(this); }
  };

  class ILMTException : public IloException {
  public:
    ILMTException(const char * message) : IloException(message, IloFalse) { }
  };
  
 

  ///////////////////////////////////////////////////////////////////////////
  // Unclassified
  ///////////////////////////////////////////////////////////////////////////

  IlcAllocator* getPersistentAllocator() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _getPersistentAllocator();
  }


  ////////////////////////////////////////////////////////////////////////
  // Run-time license
  ////////////////////////////////////////////////////////////////////////
  
  static IloBool RegisterLicense(const char *, int);

  
  IloBool fitsWithinCELimits() const {
    IlcCPOAssert(getImpl() != 0, "IloCP: empty handle");
    return _fitsWithinCELimits();
  }


 void _prettyPrintSchedule(ILOSTD(ostream)& s) const { _prettyPrintState(s); }

 
 IloNum getNativeIntMaxAsNum() const;

  
public:
  ///////////////////////////////////////////////////////////////////////////
  // Pretty-print schedule
  ///////////////////////////////////////////////////////////////////////////
  void prettyPrintSchedule(ILOSTD(ostream)& s) const {
    IlcCPOAssert(getImpl(), "IloCP: empty handle.");
    _prettyPrintSchedule(s);
  }
  void prettyPrintSchedule() const { prettyPrintSchedule(out()); }

  ///////////////////////////////////////////////////////////////////////////
  // Cursor on conflict elements
  ///////////////////////////////////////////////////////////////////////////
  
  
  class ConflictElementCursor {
  public:
    ConflictElementCursor(IloCP cp) {
      _init(cp);
      IlcCPOAssert(_hasConflict(), "No current conflict");
    }
    ~ConflictElementCursor();
    void operator++() {
       IlcCPOAssert(ok(), "Invalid state of cursor");
      _advance();
    }
    IloBool ok() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      return _ok();
    }
    IloBool isConstraint() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      return _isConstraint();
    }
    IloBool isIntVar() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      return _isIntVar();
    }
    IloBool isIntervalVar() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      return _isIntervalVar();
    }
    IloCP::ConflictStatus getStatus() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      return _getStatus();
    }
    const char* getName() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      return _getName();
    } 
    // When model is from concert:
    IloConstraint getConstraint() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      IlcCPOAssert(_isConstraint(), "Current conflict element is not a constraint");
      return _getConstraint();
    }
    IloIntVar getIntVar() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      IlcCPOAssert(_isIntVar(), "Current conflict element is not an integer variable");
      return _getIntVar();
    }
    IloIntervalVar getIntervalVar() const {
      IlcCPOAssert(_hasConflict(), "No current conflict");
      IlcCPOAssert(ok(), "Invalid state of cursor");
      IlcCPOAssert(_isIntervalVar(), "Current conflict element is not an interval variable");
      return _getIntervalVar();
    }
  private:
    IloBool _hasConflict() const;
    void _init(IloCP cp);
    void _advance();
    IloBool _ok() const;
    IloBool _isConstraint() const;
    IloBool _isIntVar() const;
    IloBool _isIntervalVar() const;
    IloCP::ConflictStatus _getStatus() const;
    const char* _getName() const; 
    IloConstraint _getConstraint() const;
    IloIntVar _getIntVar() const;
    IloIntervalVar _getIntervalVar() const;
    
  private:
    void*   _crefiner;
    IloInt  _pos;
  };
  
public:
  void setJavaVM(void* vm);

  
  void storeWarningsInternally(IloBool store = IloTrue);

   
  class WarningIterator {
   private:
    IloCPI* _cp;
    IloInt  _position;
    IloUInt _getMsgCode() const;
    IloExtractable _getExtractable() const;
    const char* _getMessage() const;
    const char* _getCPOModelPart() const;
   public:
    WarningIterator(IloCP cp);
    ~WarningIterator();
    IloBool ok() const;
    void operator++() {
      IlcCPOAssert(ok(), "Invalid state of the iterator");
      _position++;
    }
    IloUInt getMsgCode() const {
      IlcCPOAssert(ok(), "Invalid state of the iterator");
      return _getMsgCode();
    }
    IloExtractable getExtractable() const {
      IlcCPOAssert(ok(), "Invalid state of the iterator");
      return _getExtractable();
    }
    const char* getMessage() const {
      IlcCPOAssert(ok(), "Invalid state of the iterator");
      return _getMessage();
    }
    
    const char* getCPOModelPart() const {
      IlcCPOAssert(ok(), "Invalid state of the iterator");
      return _getCPOModelPart();
    }
  };
  ///////////////////////////////////////////////////////////////////////////
  // Json Serialization
  ///////////////////////////////////////////////////////////////////////////
public:
  // These functions are NOT documented
  // they have been moved on IloCP to facilitate the JAVA wrapping
  
  // generate JSON which contains
  // - the serialization format
  // - the solve/fail status
  // - the current variables' domains/values
  // - infos
  // - params
  
  void serializeCP(ILOSTD(ostream)& stream, IloInt indentStep,
                   IloBool postPropagate, IloBool status, IloBool solution,
                   IloBool info, IloBool parameters, IloInt ns=0);

  
  void serializeAConflict(ILOSTD(ostream)& stream, IloInt indentStep=0);
 
};

ILOSTD(ostream)& operator << (ILOSTD(ostream) & o,
                              const IloCP::PrintNumVarDomains& doms);
ILOSTD(ostream)& operator << (ILOSTD(ostream) & o,
                              const IloCP::PrintIntervalVarDomains& doms);



////////////////////////////////////////////////////////////////////////
//
// IloSolver compat
//
////////////////////////////////////////////////////////////////////////


enum IlcFilterLevel {
  IlcLow=0L,
  IloLowLevel=0L,
  IlcBasic=1L,
  IloBasicLevel=1L,
  IlcMedium=2L,
  IloMediumLevel=2L,
  IlcExtended=3L,
  IloExtendedLevel=3L
};

enum IlcFilterLevelConstraint {
  IlcAllDiffCt=0L,
  IloAllDiffCt=0L,
  IlcDistributeCt=1L,
  IloDistributeCt=1L,
  IlcSequenceCt=2L,
  IloSequenceCt=2L,
  IlcAllMinDistanceCt=3L,
  IloAllMinDistanceCt=3L,
  IlcPartitionCt=4L,
  IloPartitionCt=4L,
  IlcAllNullIntersectCt=5L,
  IloAllNullIntersectCt=5L,
  IlcEqUnionCt=6L,
  IloEqUnionCt=6L,
  IlcCountCt=8L,
  IloCountCt=8L
};

enum IlcFloatDisplay {
  IlcStandardDisplay = 0,
  IlcIntScientific,
  IlcIntFixed,
  IlcBasScientific,
  IlcBasFixed
};





////////////////////////////////////////////////////////////////////////////////
//
// RTTI management
//
////////////////////////////////////////////////////////////////////////////////
class IlcRtti {
public:
  typedef IloInt TypeIndex;
  typedef void (*TypeInfo)();
private:
  static TypeIndex _counter;
protected:
  static TypeIndex getNewRtti() {
    return ++_counter;
  }
  static  void InitTypeIndex(volatile TypeIndex* index);
public:
  
  static const char* GetRttiTypeName();
  
  virtual ~IlcRtti();

  
  static  TypeIndex    GetTypeIndex();
  
  virtual TypeIndex    getTypeIndex() const;

  
  static  TypeInfo    GetTypeInfo();
  
  static  TypeInfo    GetBaseTypeInfo();
  
  virtual TypeInfo    getTypeInfo() const;
  
  virtual IloBool     isType (TypeInfo typeinfo) const;
  
  virtual const char* getRttiTypeName() const;

  // returns the allocation size of 
  virtual size_t getRttiTypeSize() const = 0;
};

typedef IlcRtti::TypeIndex IlcTypeIndex;
typedef IlcRtti::TypeInfo IlcTypeInfo;

# define ILCRTTIDECL                                                    \
        static volatile IlcTypeIndex _rttiIndex;                        \
public:                                                                 \
                                                                           \
        static  IlcTypeInfo GetTypeInfo();                                    \
                                                                           \
        static  IlcTypeInfo GetBaseTypeInfo();                                \
                                                                           \
        virtual IlcTypeInfo getTypeInfo() const;                              \
                                                                           \
        virtual IlcTypeInfo getBaseTypeInfo() const;                          \
                                                                           \
        virtual IloBool     isType(IlcTypeInfo typeinfo) const;               \
        virtual IlcTypeIndex getTypeIndex() const;                            \
        virtual IlcTypeIndex getBaseTypeIndex() const;                        \
        static  IlcTypeIndex GetTypeIndex();                                  \
                                                                           \
        virtual const char* getRttiTypeName() const;                          \
                                                                           \
        static  const char* GetRttiTypeName();                                \
        virtual size_t getRttiTypeSize() const;

#define ILCRTTIN(_thisMacro, _this, _baseMacro, _base, _templateMacro, _template) \
        _templateMacro _template \
        IlcTypeInfo _thisMacro _this::GetBaseTypeInfo() { \
        const IlcRtti::TypeInfo type = _baseMacro _base::GetTypeInfo(); \
        return type; \
} \
        _templateMacro _template \
        IlcTypeInfo _thisMacro _this::GetTypeInfo() { \
          return IlcRtti::TypeInfo(_thisMacro _this::GetTypeIndex()); \
} \
        _templateMacro _template \
        IlcTypeInfo _thisMacro _this::getTypeInfo() const { \
          return _thisMacro _this::GetTypeInfo(); \
} \
        _templateMacro _template \
        IlcTypeInfo _thisMacro _this::getBaseTypeInfo() const { \
        const IlcRtti::TypeInfo type = _baseMacro _base::GetTypeInfo(); \
        return type; \
} \
        _templateMacro _template \
        IloBool _thisMacro _this::isType (IlcTypeInfo typeinfo) const { \
        return (typeinfo == GetTypeInfo()  || \
        _baseMacro _base::isType (typeinfo)  ); \
} \
        _templateMacro _template \
        volatile IlcTypeIndex _thisMacro _this::_rttiIndex = 0;  \
        _templateMacro _template \
        IlcTypeIndex _thisMacro _this::getTypeIndex() const { \
          return GetTypeIndex();                              \
} \
        _templateMacro _template \
        IlcTypeIndex _thisMacro _this::getBaseTypeIndex() const { \
        return _baseMacro _base::GetTypeIndex(); \
} \
        _templateMacro _template \
        IlcTypeIndex _thisMacro _this::GetTypeIndex() { \
          IlcRtti::InitTypeIndex(&_thisMacro _this::_rttiIndex);        \
          return _rttiIndex; \
} \
        _templateMacro _template \
        const char* _thisMacro _this::getRttiTypeName() const { \
          return _thisMacro _this::GetRttiTypeName();           \
} \
        _templateMacro _template \
        size_t _thisMacro _this::getRttiTypeSize() const { \
          return sizeof(*this);                          \
} \
        _templateMacro _template \
        const char* _thisMacro _this::GetRttiTypeName() { \
        return ILCCONCAT(_thisMacro,_STRINGIZE) _this; \
}



#define ILCEMPTYMACROARGUMENT
#define ILCSIMPLECLASS(X)       X
#define ILCTEMPLATECLASS_1(X,Y) X < Y >
#define ILCTEMPLATECLASS_2(X,Y,Z) X < Y , Z >
#define ILCTEMPLATECLASS_3(X,Y,Z,K) X < Y, Z, K >

#define ILCSIMPLECLASS_STRINGIZE(X)     #X
#define ILCTEMPLATECLASS_1_STRINGIZE(X,Y) #X "<" #Y ">"
#define ILCTEMPLATECLASS_2_STRINGIZE(X,Y,Z) #X "<" #Y "," #Z ">"
#define ILCTEMPLATECLASS_3_STRINGIZE(X,Y,Z,K) #X "<" #Y "," #Z "," #K ">"

#define ILCNOTEMPLATE()
#define ILCGENTEMPLATE_1(a)     template < a >
#define ILCGENTEMPLATE_2(a,b)   template < a , b >
#define ILCGENTEMPLATE_3(a,b,c) template < a , b , c >

#define ILCRTTI1(_this, _base, _t1)  \
        ILCRTTIN(ILCSIMPLECLASS,(_this),ILCSIMPLECLASS,(_base),ILCGENTEMPLATE_1,(_t1))

#define ILCRTTI2(_this,_base,_t1,_t2) \
        ILCRTTIN(ILCSIMPLECLASS,(_this),ILCSIMPLECLASS,(_base),ILCGENTEMPLATE_2,(_t1,_t2))

#define ILCRTTI3(_this,_base,_t1,_t2,t3) \
        ILCRTTIN(ILCSIMPLECLASS,(_this),ILCSIMPLECLASS,(_base),ILCGENTEMPLATE_3,(_t1,_t2,t3))

#define ILCRTTI( _this, _base ) \
        ILCRTTIN(ILCSIMPLECLASS,( _this ),ILCSIMPLECLASS,( _base ),ILCNOTEMPLATE,())

//----------------------------------------------------------------------
//
// this is a root class for objects 
// that are allocated using new on an IlcAllocatorPtr.
// It is there as a convenience.
//
class IlcAllocatedObject : public IlcRtti {
private:
  IlcAllocatedObject(const IlcAllocatedObject& x);
protected:
  IlcAllocatorPtr _allocator;
public:
  typedef IlcAllocatorPtr                       AllocatorType;
  typedef IlcAllocatorAllocatorTraits           AllocatorTraits;
  IlcAllocatedObject(IlcAllocatorPtr a)
    : _allocator(a)
  { } 
  virtual ~IlcAllocatedObject();
  IlcAllocator* getAllocator() const { return _allocator; }
  // IloCPI*       getCPI() const;
  void* operator new(size_t size, IlcAllocator* a);
  void operator delete(void* ptr, size_t size);
#ifdef ILODELETEOPERATOR
  void operator delete(void *, IlcAllocator*) { }
#endif
  ILCRTTIDECL
};


class IlcRttiEnvObjectI : public IlcAllocatedObject {
  ILCRTTIDECL
  protected:
  char*         _name;
  void*         _object;
public:
  IlcRttiEnvObjectI(IlcAllocator* env) 
  : IlcAllocatedObject(env)
  { }
  IlcRttiEnvObjectI(IloEnvI* env);
  virtual ~IlcRttiEnvObjectI();
  IloEnvI * getEnv() const;
  void* getObject() const { return _object; }
  const char* getName() const { return _name; }
  void setObject(void* p) { _object = p; }
  void setName(const char* n);

  void* operator new(size_t size, IloEnvI* a);
  void operator delete(void* ptr, size_t size);
  void* operator new(size_t size, IloEnv env) 
  { return operator new (size, env.getImpl()); }
#ifdef ILODELETEOPERATOR
  void operator delete(void *, IloEnvI*) { }
  void operator delete(void *, IloEnv) { }
#endif
};

////////////////////////////////////////////////////////////////////////
//
// ILOGOAL
//
////////////////////////////////////////////////////////////////////////


class IloGoalI : public IlcRttiEnvObjectI {
public:

  IloGoalI(IloEnvI*);

  virtual ~IloGoalI();

  virtual IlcGoal extract(const IloCPEngine cp) const=0;    
  IlcGoal extract(IlcCPEngineI* engine) const;                  

  virtual void display(ILOSTD(ostream&)) const;
  ILCRTTIDECL

#ifdef ILO_WINDOWS
  void operator delete(void* ptr, size_t size);
#endif
};


class IloGoal {
  ILOCPHANDLEINLINE(IloGoal, IloGoalI)
public:
  typedef IloGoalI ImplClass;

  IloGoal(IloEnv env, IloIntVarArray vars);

  IloGoal(IloEnv env, IloIntVarArray vars,
                      IloIntVarChooser varChooser,
                      IloIntValueChooser valueChooser);

  IloEnv getEnv() const;

  void end() const;
};

ILOSTD(ostream&) operator << (ILOSTD(ostream&), const IloGoal&);


typedef IloArray<IloGoal> IloGoalArray;


IloGoal IloGoalTrue(const IloEnv);


IloGoal IloGoalFail(const IloEnv);


IloGoal operator && (const IloGoal g1, const IloGoal g2);

IloGoal IloAndGoal(const IloEnv env, const IloGoal, const IloGoal);

IloGoal operator||(const IloGoal g1, const IloGoal g2);

IloGoal IloOrGoal(const IloEnv env, const IloGoal, const IloGoal);


IloConstraint IloSubCircuit(IloEnv env, IloIntExprArray next, const char * name = 0);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

ILCGCCEXPORTOFF

#endif

