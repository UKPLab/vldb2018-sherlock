
// -------------------------------------------------------------- -*- C++ -*-
// File: ./include/ilcp/cpext.h
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
//
#ifndef __CP_cpextH
#define __CP_cpextH

#ifdef _MSC_VER
#pragma pack(push,8)
#endif

#include <ilcp/cp.h>
#ifdef ILO_LINUX
# include <stdint.h> // For int64_t
#endif

ILCGCCEXPORTON

// Internal classes (not defined in this header file):
ILCGCCHIDINGON

class IlcConstIntArray;
class IlcCPODemonIList;
class IlcCPOECSetOfSharedTupleI;
class IlcCPOEventHandlerI;
class IlcCPOIntCell;
class IlcCumulElementVarArrayI;
class IlcDeltaIteratorI;
class IlcFloatArrayI;
class IlcFloatExpI;
class IlcFloatVarArrayI;
class IlcFloatVarArrayIterator;
class IlcGetVarVisitor;
class IlcIntervalSequenceVarArrayI;
class IlcIntervalVarArrayI;
class IlcIntExpI;
class IlcIntSetI;
class IlcIntSetVarI;
class IlcIntVarArrayIterator;
class IlcIntVarI;
class IlcManagerI;
class IlcQueueMonitorI;
class IlcsDemandI;
class IlcSearchI;
class IlcSearchMonitorWrapper;
class IlcsIntervalSequenceVarI;
class IlcsIntervalVarI;
class IlcCPEngineI;

class IlcCPI;
IlcManagerI* IlcGetManagerI(const IlcCPEngineI* s);

class IloInferencer;
class IlcLaModel;
typedef const IlcLaModel* IlcConstLayerModel;

ILCGCCHIDINGOFF

// Public classes (defined in cp.h or cpext.h):
class IlcConstraintI;
class IlcGoalI;

// #define IlcCPEngine IloCP

IlcCPEngineI* IlcGetCPEngineI(const IlcManagerI *);
const IlcCPI* IlcGetCPI(const IlcManagerI*);
const IlcCPI* IlcGetCPI(const IlcCPEngineI* e);


////////////////////////////////////////////////////////////////////////////
//
// BASIC TYPES
//
////////////////////////////////////////////////////////////////////////////

#define ILCSTLBEGIN ILOSTLBEGIN
#define ILCSTD(x) ILOSTD(x)



#ifdef CPPREF_GENERATION

const IlcInt IlcIntMax=9007199254740991; // (2^53 - 1)

const IlcInt IlcIntMin=-9007199254740991; // -(2^53 - 1)
#else
#ifdef ILO64
#define IlcIntMax ((IlcInt)9007199254740991) // (2^53 - 1)
#else
#define IlcIntMax ((IlcInt)LONG_MAX)
#endif
#define IlcIntMin (-IlcIntMax)

#endif


#ifdef ILO64
#  ifdef ILO_WIN64
#    define IlcIntTop _I64_MAX
#  else
#    define IlcIntTop LONG_MAX
#  endif
#else
#define IlcIntTop IlcIntMax
#endif


#define IlcIntBottom (-IlcIntTop)


extern const IlcFloat IlcFloatMax;


extern const IlcFloat IlcFloatMin;

////////////////////////////////////////////////////////////////////////////
//
// BASIC MACROS
//
////////////////////////////////////////////////////////////////////////////

class IlcExtension {
private:
  IlcAny _object;
  char* _name;
public:
  IlcExtension(IlcManagerI * m, IlcAny object, const char* name);
  IlcAny getObject() const {return _object;}
  char* getName() const {return _name;}
  void setObject(IlcManagerI *, IlcAny object);
  void setName(IlcManagerI *, const char* name);
};

#define ILCEXTENSIONMETHODSIDECL \
 \
  const char * getName() const; \
 \
  IlcAny getObject() const; \
 \
  void setName(const char * name); \
 \
  void setObject(IlcAny object);

#define ILCEXTENSIONMETHODSHDECL(Hname) \
private: \
  const char * _getName() const; \
  IlcAny       _getObject() const; \
  void         _setName(const char * name) const; \
  void         _setObject(IlcAny object) const; \
public: \
 \
  const char * getName() const { \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");\
    return _getName(); \
  } \
 \
  IlcAny getObject() const { \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");\
    return _getObject(); \
  } \
 \
  void setName(const char * name) { \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");\
    _setName(name); \
  } \
 \
  void setObject(IlcAny object) { \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");\
    _setObject(object); \
  }



#define ILCGETCPINLINEHDECL(Hname)                              \
                                                             \
  IlcCPEngine getCPEngine() const {                                 \
    IlcCPOAssert(_impl != 0, #Hname ": Empty handle");          \
    return _impl->getCPEngine();                                  \
  }                                                             \
   IlcCPEngineI* getCPEngineI() const {                             \
    IlcCPOAssert(_impl != 0, #Hname ": Empty handle");          \
    return _impl->getCPEngineI();                                 \
  }                                                             \
  const IlcCPI* getCPI() const { return IlcGetCPI(getCPEngineI()); }          \
  IlcManagerI* getManagerI() const { return IlcGetManagerI(getCPEngineI()); } \
  IlcCPEngine getManager() const { return getCPEngine(); }





#define ILCGETCPHDECL(Hname)                                            \
private:                                                                \
  IlcManagerI * _getManagerI() const;                                   \
  IlcCPEngineI * _getCPEngineI() const { return IlcGetCPEngineI(_getManagerI()); } \
  const IlcCPI*  _getCPI() const { return IlcGetCPI(_getManagerI()); }        \
public:                                                                 \
                                                                     \
  IlcCPEngine getCPEngine() const {                                     \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");   \
    return _getCPEngineI();                                             \
  }                                                                     \
  IlcCPEngineI *getCPEngineI() const {                                  \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");   \
    return _getCPEngineI();                                             \
  }                                                                     \
  const IlcCPI* getCPI() const {                                        \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(Hname) " : Empty handle");   \
    return _getCPI();                                                   \
  }                                                                     \
  IlcManagerI* getManagerI() const { return _getManagerI(); }           \
  IlcCPEngine getManager() const { return _getCPEngineI(); }



#define ILCID                                           \
  private:                                              \
    static IlcInt _classId;                             \
  public:                                               \
    virtual IlcInt getClassId() const;                  \
    static IlcInt GetClassId()                          \

#if defined(NDEBUG) && !defined(ILC_SHOW_FUNC_PARAM)
#define ILCPARAM(x)
#else
#define ILCPARAM(x) x
#endif


//----------------------------------------------------------------------
class IlcSearchInfo;
class IlcEngineI;

// used for input & output
class IlcEngineCallbackI {
public:
  virtual void execute(IlcInt infoType, IlcSearchInfo* info) = 0;
  virtual ~IlcEngineCallbackI();
};

class IlcEngineCallback {
public:
  ILOCPVISIBLEHANDLEMINI(IlcEngineCallback, IlcEngineCallbackI)
};

class IlcEngineCallbackMapI;
class IlcCPEngine;

class IlcEngineCallbackMap {
public:
  ILOCPVISIBLEHANDLEMINI(IlcEngineCallbackMap, IlcEngineCallbackMapI)
  IlcEngineCallbackMap(IlcCPEngine);
  void put(IlcInt callbackType, IlcEngineCallback);
  void get(IlcInt callbackType);
  void end();
};

////////////////////////////////////////////////////////////////////////////
//
// IlcCPEngine
//
////////////////////////////////////////////////////////////////////////////



class IlcCPEngine {
public: 
       
  
  
  
  enum IntInfo {    
    
    
      
    NumberOfChoicePoints = 1,  
    
    
      
    NumberOfFails = 2,  
    
    
      
    NumberOfBranches = 3,      
    
    
      
    NumberOfEngineVariables = 6,    
    
    
      
    MemoryUsage = 8,                  
    
    
      
    WorkerId = 26,                          
    
    
      
    NumberOfEngineConstraints = 2018,         
  ILC_MAX_IntInfo
                 };    
  
  enum LayerObjectiveValueInfo {       
  ILC_MAX_LayerObjectiveValueInfo
                 };    
  
  enum NumInfo {                                 
  ILC_MAX_NumInfo
                 };    
  
  enum StringInfo {     
  ILC_MAX_StringInfo
                 };    
  
  enum LayerSolutionInfo {     
  ILC_MAX_LayerSolutionInfo
                 };  

 

protected:
  void          _prettyPrintState(ILOSTD(ostream)& s) const;
  IlcAllocator* _getReversibleAllocator() const;
  IlcAllocator* _getSolveAllocator() const;
  IlcAllocator* _getPersistentAllocator() const;
  IlcFloat      _getInfo(const char * name) const;
  IlcFloat      _getParameter(const char* name) const;
  ILOSTD(ostream&) _out() const;
  void _dumpModel(const char * fname) const;
  void _dumpModel(ILCSTD(ostream)& out) const;
  void _exportModel(const char * fname) const;
  void _exportModel(ILCSTD(ostream)& out) const;
  void _printInformation() const;
  void _printInformation(ILOSTD(ostream) & o) const;

     
  IlcInt _getIntInfo(IlcInt id) const;           
  IlcFloat _getNumInfo(IlcInt id) const;             
                 
  IlcInt _getIntParameter(IlcInt id) const; 
  IlcInt _getIntParameterDefault(IlcInt id) const;      
  IlcFloat _getNumParameter(IlcInt id) const; 
  IlcFloat _getNumParameterDefault(IlcInt id) const;       
public:
    
  
  IlcInt getInfo(IlcCPEngine::IntInfo info) const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    return _getIntInfo((IlcInt)info);
  }
                    

  
  IlcFloat getInfo(const char * name) const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(name  != 0, "IlcCPEngine::getInfo - empty name");
    return (IlcFloat)_getInfo(name);
  } 



  
  IlcFloat getParameter(const char* name) const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    return _getParameter(name);
  }

  

private:
  void                  _saveValue(IlcAny * ptr) const;
  void                  _saveValue(IlcFloat * ptr) const;
  void                  _exitSearch() const;
  void                  _fail(IlcAny label) const;
  void                  _freeze() const;
  void                  _unfreeze() const;
  IlcBool               _isAllFixed() const;
  IlcBool               _hasObjective() const;
  IlcBool               _next() const;
  void                  _endSearch() const;
public:
  IlcAllocationStack *  _getHeap() const;

  ///////////////////////////////////////////////////////////////////////////
  // Low-level, advanced
  ///////////////////////////////////////////////////////////////////////////
  // No wrapping
  void saveValue(IlcAny * ptr) const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(ptr != 0, "IlcCPEngine: saveValue must receive non-null pointer");
    _saveValue(ptr);
  }
  // No wrapping
  void saveValue(IlcInt * ptr) const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(ptr != 0, "IlcCPEngine: saveValue must receive non-null pointer");
    _saveValue((IlcAny*)ptr);
  }
  // No wrapping
  void saveValue(IlcFloat * ptr) const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(ptr != 0, "IlcCPEngine: saveValue must receive non-null pointer");
    _saveValue(ptr);
  }
  
  void startNewSearch(const IlcGoal goal) const;
  
  IloBool next() const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    return _next();
  }
  
  
  void endSearch() const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    _endSearch();
  }

  
  void fail(IlcAny label=0) const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    _fail(label);
  }
  void freeze() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    _freeze();
  }
  void unfreeze() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    _unfreeze();
  }
  
  void exitSearch() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    _exitSearch();
  }

  
  void addReversibleAction(const IlcGoal goal) const;

  
  IlcBool solve(const IlcGoal goal, IlcBool restore = IlcFalse) const;
  
  void add(const IlcConstraint constraint) const;
  
  void add(const IlcConstraintArray constraints) const;
  
  IlcAllocationStack * getHeap() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _getHeap();
  }
  IlcBool isAllFixed() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _isAllFixed();
  }
  
  IlcBool hasObjective() const {
    IlcCPOAssert(_impl != 0, "IlcCPEngine: empty handle");
    return _hasObjective();
  }


  
  
    
  
private:
  IlcFloat _getRandomNum() const;
  IlcInt   _getRandomInt(IlcInt n) const;

public:
  
  IlcRandom getRandom() const;
  
  IlcInt getRandomInt(IlcInt n) const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(n >= 1, "IlcCPEngine: getRandomInt(n): n < 1");
    return _getRandomInt(n);
  }
  
  IlcFloat getRandomNum() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _getRandomNum();
  }
 
  

  ILOCPHIDDENHANDLEMINI(IlcCPEngine,IlcCPEngineI)
  ILCEXTENSIONMETHODSHDECL(IlcCPEngine)
  ILCGETCPHDECL(IlcCPEngine)

  IlcCPEngine(IlcManagerI* m)
    : _impl(IlcGetCPEngineI(m))
  { }

  ///////////////////////////////////////////////////////////////////////////
  // Unclassified
  ///////////////////////////////////////////////////////////////////////////
  IlcAllocator* getReversibleAllocator() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _getReversibleAllocator();
  }
  IlcAllocator* getSolveAllocator() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _getSolveAllocator();
  }
  IlcAllocator* getPersistentAllocator() const {
    IlcCPOAssert(getImpl() != 0, "IlcCPEngine: empty handle");
    return _getPersistentAllocator();
  }
  void prettyPrintState(ILOSTD(ostream)& s) const {
    IlcCPOAssert(getImpl(), "IlcCPEngine: empty handle.");
    _prettyPrintState(s);
  }
  void prettyPrintState() const 
  { prettyPrintState(ILOSTD(cout)); }

  
  ILOSTD(ostream)& out() const
  { return _out(); }

  void dumpModel(const char * fname) const {
    IlcCPOAssert(getImpl(), "IlcCPEngine: empty handle.");
    IlcCPOAssert(fname != 0, "IlcCPEngine::dumpModel - NULL filename.");
    _dumpModel(fname);
  }
  void dumpModel(ILCSTD(ostream)& out) const {
    IlcCPOAssert(getImpl(), "IlcCPEngine: empty handle.");
    _dumpModel(out);
  }
  void exportModel(const char * fname) const {
    IlcCPOAssert(getImpl(), "IlcCPEngine: empty handle.");
    IlcCPOAssert(fname != 0, "IlcCPEngine::exportModel - NULL filename.");
    _exportModel(fname);
  }
  void exportModel(ILCSTD(ostream)& out) const {
    IlcCPOAssert(getImpl(), "IlcCPEngine: empty handle.");
    _exportModel(out);
  }
  
  void printInformation() const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printInformation();
  }
  
  void printInformation(ILOSTD(ostream)& stream) const {
    IlcCPOAssert(_impl != 0, "IloCP: empty handle");
    _printInformation(stream);
  }
};




class IloCPEngine : public IlcCPEngine {
protected:
  IloCP     _iloCP;
public:
  IloCPI* getIloCPI() const { return _iloCP.getImpl(); }
  
  IloCP getMasterCP() const { return _iloCP; }

  ///////////////////////////////////////////////////////////////////////////
  // Constructors, extraction and related methods
  ///////////////////////////////////////////////////////////////////////////

  IloCPEngine(); // For SWIG only
  
  IloCPEngine(const IlcCPEngine& engine);
  IloCPEngine(IlcCPEngineI* e);

  // FIXME: This seems wrong, so commented it.
  //ILOSTD(ostream)& out() const
  //{ return _iloCP.out(); }
  
  IloCPEngine& operator = (const IloCPEngine& x) {
    _impl = x._impl;
    _iloCP = x._iloCP;
    return *this;
  }

  IloCPEngine& operator = (const IlcCPEngine& x);

  
  
  
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

  


  
  class IntVarIterator
#ifndef CPPREF_GENERATION
    : public IlcIntDomainIterator 
#endif
  {
  private:
    // disable reference counting
    void _setImpl(IlcIntDomainIteratorI* it) { _impl = it; }
    void _init(const IloCPEngine cp, const IloIntVarI* var);
  public:
    IntVarIterator() { _init(0, 0); } // Only for SWIG
    IntVarIterator(const IntVarIterator& x)
      : IlcIntDomainIterator(x)
    { }
    
    IntVarIterator(const IloCPEngine cp, const IloIntVar var) 
    {
      IlcCPOAssert(cp.getImpl() != 0, "IloCPEngine: empty handle");
      IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
      _init(cp, var.getImpl());
    }
    
    IntVarIterator(const IloCPEngine cp, const IloNumVar var) {
      IlcCPOAssert(cp.getImpl() != 0, "IloCPEngine: empty handle");
      IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
      IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
      _init(cp, IloIntVar(var.getImpl()).getImpl());
    }
    
    IntVarIterator& operator = (const IntVarIterator& x) {
      _impl = x._impl; 
      return *this; 
    }
        
    IntVarIterator& operator++() { 
      IlcCPOAssert(getImpl() != 0, "IloCPEngine::IntVarIterator: empty handle");
      _advance();
      return *this;
    }
    
    IloInt operator*() const { 
      IlcCPOAssert(getImpl() != 0, "IloCPEngine::IntVarIterator: empty handle");
      return _getValue();
    }
    // 2.0b1
    IloAny getAnyValue() const;
    
    IloBool ok() const { 
      IlcCPOAssert(getImpl(), "IloCPEngine::IntVarIterator: empty handle");
      return _ok();
    }

    
    ~IntVarIterator();
  };

  
  
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
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(ext.getImpl() != 0, "IloExtractableArray: empty handle");
    return _isAllValid(ext);
  }

  
  void printDomain(const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    _printDomain(out(), var);                                           
  }
  void printDomain(ILOSTD(ostream)& s, const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    _printDomain(s, var);
  }
  void printDomain(ILOSTD(ostream)& s, const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    _printDomain(s, vars);
  }
  void printDomain(const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    _printDomain(out(), vars);
  }
  void printDomain(ILOSTD(ostream)& s, const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloIntVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloIntVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloIntVarArray: element not extracted");
    _printDomain(s, vars);
  }
  void printDomain(const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloIntVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloIntVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloIntVarArray: element not extracted");
    _printDomain(out(), vars);
  }

  class PrintDomains {
  protected:
    const IloCPEngine& _cp;
    IloInt                  _n;
    IloBool                 _singleton;
    IloExtractableI **      _var;

    PrintDomains(const IloCPEngine& cp, const IloExtractable ext);
    PrintDomains(const IloCPEngine& cp, const IloExtractableArray ext);
    PrintDomains(const PrintDomains&);
    const IlcEngineState* getState() const; 
  public:
    ~PrintDomains();
  private:
    PrintDomains& operator = (PrintDomains& pd) { return *this; }
  };

  class PrintNumVarDomains : public PrintDomains {
    friend class IloCPEngine;
  private:
    PrintNumVarDomains(const IloCPEngine& cp, const IloNumVar var);
    PrintNumVarDomains(const IloCPEngine& cp, const IloNumVarArray var);
    PrintNumVarDomains(const IloCPEngine& cp, const IloIntVarArray var);
  public:
    void display(ILOSTD(ostream)& o) const;
  private:
    PrintNumVarDomains& operator = (const IloCPEngine::PrintNumVarDomains&) { return *this; }
  };

  
  PrintNumVarDomains domain(const IloNumVar var) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    return PrintNumVarDomains(*this, var);
  }
  
  PrintNumVarDomains domain(const IloNumVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(vars.getImpl() != 0, "IloNumVarArray: empty handle");
    IlcCPOAssert(isAllValid(vars), "IloNumVarArray: empty element handle");
    IlcCPOAssert(isAllExtracted(vars), "IloNumVarArray: element not extracted");
    return PrintNumVarDomains(*this, vars);
  }
  
  PrintNumVarDomains domain(const IloIntVarArray vars) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
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
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    _setNodeHook(hook);
  }
#endif

  

  IloNum getObjMin() const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCPEngine: No objective present");
    return _getObjMin();
  }
  IloNum getObjMin(IloInt i) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCPEngine: No objective present");
    IlcCPOAssert(i >= 0, "IloCPEngine: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCPEngine: Objective value index is too large");
    return _getObjMin(i);
  }
  IloNum getObjMax() const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCPEngine: No objective present");
    return _getObjMax();
  }
  IloNum getObjMax(IloInt i) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(_hasObjective(), "IloCPEngine: No objective present");
    IlcCPOAssert(i >= 0, "IloCPEngine: Objective value index is negative");
    IlcCPOAssert(i < _getNumberOfCriteria(),
              "IloCPEngine: Objective value index is too large");
    return _getObjMax(i);
  }

  
  IloNum getValue(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
     IlcCPOAssert(isFixed(v), "IloNumVar: not fixed");  
    return _getValue(v);
  }
  
  IloInt getValue(const IloIntVar v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloIntVar: not extracted");
     IlcCPOAssert(isFixed(v), "IloIntVar: not fixed");  
    return _getValue(v);
  }
  // 2.0b1
  IloAny getAnyValue(const IloIntVar v) const { return (IloAny)getValue(v); }

  
  IloNum getMin(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
    return _getMin(v);
  }
  
  IloNum getMax(const IloNumVar v) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(v.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(v), "IloNumVar: not extracted");
    return _getMax(v);
  }
  
  IloInt getMax(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getMax(var);
  }
  
  IloInt getMin(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getMin(var);
  }
  void getBounds(const IloIntVar var, IloInt& min, IloInt& max) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    _getBounds(var, min, max);
  }
   
  IloBool isInDomain(const IloNumVar var, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return _isInDomain(IloIntVar(var.getImpl()), value);
  }
  
  IloBool isInDomain(const IloIntVar var, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _isInDomain(var, value);
  }
  
  IloInt getDomainSize(const IloNumVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return _getDomainSize(var);
  }
  
  IloInt getDomainSize(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _getDomainSize(var);
  }
  
  IloBool isFixed(const IloNumVar var) const  {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloNumVar: not extracted");
    return _isFixed(var);
  }
  
  IloBool isFixed(const IloIntVar var) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(var), "IloIntVar: not extracted");
    return _isFixed(var);
  }
  
  IloBool isAllExtracted(const IloExtractableArray ext) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
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
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isFixed(a);
  }

 
  IloBool isPresent(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isPresent(a);
  }
 
  IloBool isAbsent(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _isAbsent(a);
  }

  
  IloInt getStartMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getStartMin(a);
  }
  
  IloInt getStartMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getStartMax(a);
  }

  
  IloInt getStart(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getStart(a);
  }

  
  IloInt getEndMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getEndMin(a);
  }
  
  IloInt getEndMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getEndMax(a);
  }

  
  IloInt getEnd(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getEnd(a);
  }

  
  IloInt getSizeMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getSizeMin(a);
  }
  
  IloInt getSizeMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getSizeMax(a);
  }

  
  IloInt getSize(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getSize(a);
  }

  
  IloInt getLengthMin(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getLengthMin(a);
  }
  
  IloInt getLengthMax(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    return _getLengthMax(a);
  }

  
  IloInt getLength(const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl(), "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
     IlcCPOAssert(isFixed(a), "IloIntervalVar: not fixed.");  
    return _getLength(a);
  }
  void printDomain(ILOSTD(ostream)& s, const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    _printDomain(s, a);
  }
  void printDomain(const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle.");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle.");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted.");
    _printDomain(out(), a);
  }

  
  IloBool isFixed(const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
    return _isFixed(seq);
  }

  
  IloIntervalVar getFirst(const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    return _getFirst(seq);
  }
  
  IloIntervalVar getLast (const IloIntervalSequenceVar seq) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
    IlcCPOAssert(seq.getImpl(), "IloIntervalSequenceVar: empty handle.");
    IlcCPOAssert(isExtracted(seq), "IloIntervalSequenceVar: not extracted.");
     IlcCPOAssert(isFixed(seq), "IloIntervalSequenceVar: not fixed.");  
    return _getLast(seq);
  }
  
  IloIntervalVar getNext(const IloIntervalSequenceVar seq, const IloIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
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
    IlcCPOAssert(getImpl(), "IloCPEngine: empty handle.");
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
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCumulFunctionExpr: not extracted");
    return _isFixed(f);
  }

  
  IloInt getNumberOfSegments(const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    return _getNumberOfSegments(f);
  }

  
  IloNum getNumberOfSegmentsAsNum(const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    return _getNumberOfSegmentsAsNum(f);
  }

  
  IloInt getSegmentStart(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentStart(f, i);
  }
  
  IloNum getSegmentStartAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentStartAsNum(f, i);
  }

  
  IloInt getSegmentEnd(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentEnd(f, i);
  }
  
  IloNum getSegmentEndAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentEndAsNum(f, i);
  }

  
  IloInt getSegmentValue(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentValue(f, i);
  }
  
  IloNum getSegmentValueAsNum(const IloCumulFunctionExpr f, IloInt i) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidSegment(f, i), "IloCPEngine: invalid cumul function expression segment");
    return _getSegmentValueAsNum(f, i);
  }

  
  IloInt getValue(const IloCumulFunctionExpr f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCPEngine: cumul function expression evaluated on invalid point");
    return _getValue(f, t);
  }
  
  IloInt getHeightAtStart(const IloIntervalVar var, const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(isExtracted(var), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isFixed(var), "IloIntervalVar: not fixed.");  
    return _getHeightAtFoo(var, f, IloTrue, "IloCPEngine::getHeightAtStart");
  }
  
  IloInt getHeightAtEnd(const IloIntervalVar var, const IloCumulFunctionExpr f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(isExtracted(var), "IloIntervalVar: not extracted.");
    IlcCPOAssert(isFixed(var), "IloIntervalVar: not fixed.");  
    return _getHeightAtFoo(var, f, IloFalse, "IloCPEngine::getHeightAtEnd");
  }
  
  IloNum getValueAsNum(const IloCumulFunctionExpr f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloCumulFunctionExpr: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: cumul function expression not extracted");
    IlcCPOAssert(_isFixed(f), "IloCPEngine: cumul function expression not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCPEngine: cumul function expression evaluated on invalid point");
    return _getValueAsNum(f, t);
  }

  ////////////////////////////////////////////////////////////////////////
  // Reading State Functions at solution
  ////////////////////////////////////////////////////////////////////////
  
  
  IloBool isFixed(const IloStateFunction f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloStateFunctionFunction: not extracted");
    return _isFixed(f);
  }
  
  
  IloInt getNumberOfSegments(const IloStateFunction f) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    return _getNumberOfSegments(f);
  }


  
  IloInt getSegmentStart(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentStart(f, s);
  }
  
  IloNum getSegmentStartAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentStartAsNum(f, s);
  }
  
  
  IloInt getSegmentEnd(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentEnd(f, s);
  }
  
  IloNum getSegmentEndAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentEndAsNum(f, s);
  }
 
  
  IloInt getSegmentValue(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentValue(f, s);
  }
  
  IloNum getSegmentValueAsNum(const IloStateFunction f, IloInt s) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidSegment(f, s), "IloCPEngine: invalid state function segment");
    return _getSegmentValueAsNum(f, s);
  }
  
  IloInt getValue(const IloStateFunction f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
     IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCPEngine: state function evaluated on invalid point");
    return _getValue(f, t);
  }
  
  IloNum getValueAsNum(const IloStateFunction f, IloInt t) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(f.getImpl() != 0, "IloStateFunction: empty handle");
    IlcCPOAssert(isExtracted(f), "IloCPEngine: state function not extracted");
    IlcCPOAssert(_isFixed(f), "IloCPEngine: state function not fixed");  
    IlcCPOAssert(_isValidAbscissa(f, t), "IloCPEngine: state function evaluated on invalid point");
    return _getValueAsNum(f, t);
  }
 
  

  ///////////////////////////////////////////////////////////////////////////
  // Printing interval domains
  ///////////////////////////////////////////////////////////////////////////
  class PrintIntervalVarDomains : public PrintDomains {
    friend class IloCPEngine;
  private:
    PrintIntervalVarDomains(const IloCPEngine& cp, const IloIntervalVar var);
  public:
    void display(ILOSTD(ostream)& o) const;
  private:
    PrintIntervalVarDomains& operator = (const PrintIntervalVarDomains&) { return *this; }
  };

  
  PrintIntervalVarDomains domain(const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle");
    IlcCPOAssert(isExtracted(a), "IloIntervalVar: not extracted");
    return PrintIntervalVarDomains(*this, a);
  }

  // ------------------------------------------------------------------------
  // Advanced
  // ------------------------------------------------------------------------
  IloBool isInSequence (const IloIntervalSequenceVar seq,
                        const IloIntervalVar a) const {
    IlcCPOAssert(_impl != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(seq.getImpl() != 0, "IloIntervalSequenceVar: empty handle");
    IlcCPOAssert(a.getImpl() != 0, "IloIntervalVar: empty handle");
    return _isInSequence(seq, a);
  }

  
  IloCPEngine::IntVarIterator iterator(const IloIntVar var) {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloIntVar: empty handle");
    return IloCPEngine::IntVarIterator(*this, var);
  }
  
  IloCPEngine::IntVarIterator iterator(const IloNumVar var) {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    IlcCPOAssert(var.getType() != ILOFLOAT, "IloNumVar: not integer");
    return IloCPEngine::IntVarIterator(*this, var);
  }

  
  

private:
  IloInt                _getReduction(const IloIntVar var) const;
  IlcIntVar             _getIntVar(const IloNumVar var) const;
  IlcFloatArray         _getFloatArray(const IloNumArray arg) const;
  IlcIntArray           _getIntArray(const IloIntArray arg) const;
  IlcIntArray           _getIntArray(const IloNumArray arg) const;
  IlcIntSet             _getIntSet(const IloNumSet arg) const;
  IlcIntSet             _getIntSet(const IloIntSet arg) const;
  IloNum                _getImpactOfLastAssignment(const IloIntVar var) const;
  IloNum                _getImpact(const IloIntVar var) const;
  IloNum                _getImpact(const IloIntVar var, IloInt value) const;
  IloNum                _getSuccessRate(const IloIntVar var) const;
  IloNum                _getSuccessRate(const IloIntVar var, IloInt value) const;
  IloNum                _getNumberOfFails(const IloIntVar var, IloInt value) const;
  IloNum                _getNumberOfInstantiations(const IloIntVar var, IloInt value) const;
  IloNum                _getLocalImpact(const IloIntVar var, IloInt value) const;
  IloNum                _getLocalVarImpact(const IloIntVar var, IloInt depth) const;
  void                  _failBuffered() const;
  void                  _removeValueBuffered(const IloNumVarI* var, IloInt value) const;
  void                  _setMinBuffered(const IloNumVarI* var, IloNum min) const;
  void                  _setMaxBuffered(const IloNumVarI* var, IloNum max) const;
public:
  ///////////////////////////////////////////////////////////////////////////
  // Search information
  ///////////////////////////////////////////////////////////////////////////
  
  IloInt getReduction(const IloIntVar x) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getReduction(x);
  }
  
  IloNum getImpactOfLastAssignment(const IloIntVar x) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getImpactOfLastAssignment(x);
  }
  
  IloNum getImpact(const IloIntVar x) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getImpact(x);
  }
  
  IloNum getImpact(const IloIntVar x, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getImpact(x, value);
  }
  
  IloNum getSuccessRate(const IloIntVar x) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getSuccessRate(x);
  }
  
  IloNum getSuccessRate(const IloIntVar x, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getSuccessRate(x, value);
  }
  IloNum getNumberOfFails(const IloIntVar x, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getNumberOfFails(x, value);
  }
  IloNum getNumberOfInstantiations(const IloIntVar x, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getNumberOfInstantiations(x, value);
  }
  IloNum getLocalImpact(const IloIntVar x, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getLocalImpact(x, value);
  }
  IloNum getLocalVarImpact(const IloIntVar x, IloInt depth) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(x.getImpl() != 0, "IloIntVar: empty handle");
    IlcCPOAssert(isExtracted(x), "IloIntVar: not extracted");
    return _getLocalVarImpact(x, depth);
  }
  ///////////////////////////////////////////////////////////////////////////
  // Advanced: Ilc mapping
  //           No inlining here to avoid Ilc/Ilo world crossover
  ///////////////////////////////////////////////////////////////////////////
  
 IlcIntVar getIntVar(const IloNumVar var) const;
 IlcIntVar getIntVar(const IloIntVar var) const;
 IlcIntVar getIntVar(const IloNumVarI* var) const;
  
  IlcIntervalVar getInterval(const IloIntervalVar var) const;
  
  IlcCumulElementVar getCumulElement(const IloCumulFunctionExpr f) const;
  
  IlcIntervalSequenceVar getIntervalSequence(const IloIntervalSequenceVar s) const;
  
  IlcIntArray getIntArray(const IloNumArray arg) const;
   
  IlcIntArray getIntArray(const IloIntArray arg) const;
  
  IlcFloatArray getFloatArray(const IloNumArray arg) const;
  
  IlcIntSet getIntSet(const IloIntSet arg) const;
  
  IlcIntSet getIntSet(const IloNumSet arg) const;
  
  IlcFloatVar getFloatVar(const IloNumVar var) const;
  
  IlcIntVarArray getIntVarArray(const IloIntVarArray vars) const;
  
  IlcIntVarArray getIntVarArray(const IloNumVarArray vars) const;
  IlcIntVarArray getIntVarArray(const IloIntExprArray exps) const;
   
  IlcFloatVarArray getFloatVarArray(const IloNumVarArray vars) const;

  
  IlcIntExp getIntExp(const IloIntExprArg expr) const;
  
  IlcFloatExp getFloatExp(const IloNumExprArg expr) const;
  // IlcFloatExp getFloatExp(const IloNumExprI* exp) const;
  
  IlcIntTupleSet getIntTupleSet(const IloIntTupleSet ts) const;

  void removeValueBuffered(const IloNumVarI* var, IloInt value) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var != 0, "IloNumVar: empty handle");
    IlcCPOAssert(var->getType() != ILOFLOAT, "IloNumVar: not integer");
    _removeValueBuffered(var, value);
  }
  void setMinBuffered(const IloNumVarI* var, IloNum min) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var != 0, "IloNumVar: empty handle");
    _setMinBuffered(var, min);
  }
  void setMaxBuffered(const IloNumVarI* var, IloNum max) const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    IlcCPOAssert(var != 0, "IloNumVar: empty handle");
    _setMaxBuffered(var, max);
  }
  // For propagator
  void failBuffered() const {
    IlcCPOAssert(getImpl() != 0, "IloCPEngine: empty handle");
    _failBuffered();
  }

  

  IloEnv getEnv() const;
   
  
  IloBool isExtracted(const IloExtractable ext) const;
  
  //
  // Java wrapping helpers: don't use enums but int to access parameters and infos.
  //
  
  IloNum getNumInfo(IlcInt info) const {
    return _getNumInfo(info);
  }

  
  IloInt getIntInfo(IlcInt info) const {
    return _getIntInfo(info);
  }

  
  IloInt getIntParameter(IlcInt param) const
  { return _getIntParameter(param); }

  
  IloInt getIntParameterDefault(IlcInt param) const
  { return _getIntParameter(param); }

  
  IloNum getNumParameter(IlcInt param) const
  { return _getNumParameter(param); }

  
  IloNum getNumParameterDefault(IlcInt param) const
  { return _getNumParameterDefault(param); }

  
  IloNum getParameter(const char* name) const
  { return _getParameter(name); }

  
  IloNum getInfo(const char* name) const
  { return _getInfo(name); }

  using IlcCPEngine::getInfo;
};


ILOSTD(ostream)& operator << (ILOSTD(ostream) & o,
                              const IloCPEngine::PrintNumVarDomains& doms);
ILOSTD(ostream)& operator << (ILOSTD(ostream) & o,
                              const IloCPEngine::PrintIntervalVarDomains& doms);


////////////////////////////////////////////////////////////////////////
//
// PROPAGATORS
//
////////////////////////////////////////////////////////////////////////



class IloPropagatorI : public IloEnvObjectI {
  friend class IloCPI;
private:
  IloNumVarArray _vars;
  IlcCPEngineI*  _cp;
  void _addVar(IloNumVar var);
public:
  
  IloPropagatorI(IloEnv env);
  
  virtual ~IloPropagatorI();

  IloNumVar getVar(IloInt i) const { return _vars[i]; }
  IloInt getNumVars() const { return _vars.getSize(); }

  
  void addVar(IloNumVar var) {
    IlcCPOAssert(var.getImpl() != 0, "IloNumVar: empty handle");
    _addVar(var);
  }

  void violate(IloCPEngine cp);

  
  void violate();

  void setMax(IloCPEngine cp, IloNumVar var, IloNum max);
  void setMax(IloCPEngine cp, IloIntVar var, IloInt max);
  
  void setMax(IloNumVar var, IloNum max);
  void setMax(IloIntVar var, IloInt max);

  void setMin(IloCPEngine cp, IloNumVar var, IloNum min);
  void setMin(IloCPEngine cp, IloIntVar var, IloInt min);
  
  void setMin(IloNumVar var, IloNum min);
  void setMin(IloIntVar var, IloInt min);

  void setRange(IloCPEngine cp, IloNumVar var, IloNum min, IloNum max);
  void setRange(IloCPEngine cp, IloIntVar var, IloInt min, IloInt max);
  
  void setRange(IloNumVar var, IloNum min, IloNum max);
  void setRange(IloIntVar var, IloInt min, IloInt max);

  void setValue(IloCPEngine cp, IloNumVar var, IloNum value);
  void setValue(IloCPEngine cp, IloIntVar var, IloInt value);
  
  void setValue(IloNumVar var, IloNum value);
  void setValue(IloIntVar var, IloInt value);

  void removeValue(IloCPEngine cp, IloIntVar var, IloInt value);
  
  void removeValue(IloIntVar var, IloInt value);

  IloNum getMin(IloCPEngine cp, IloNumVar var) const;
  IloInt getMin(IloCPEngine cp, IloIntVar var) const;
  
  IloNum getMin(IloNumVar var) const;
  IloInt getMin(IloIntVar var) const;

  IloNum getMax(IloCPEngine cp, IloNumVar var) const;
  IloInt getMax(IloCPEngine cp, IloIntVar var) const;
  
  IloNum getMax(IloNumVar var) const;
  IloInt getMax(IloIntVar var) const;

  IloNum getValue(IloCPEngine cp, IloNumVar var) const;
  IloInt getValue(IloCPEngine cp, IloIntVar var) const;
  
  IloNum getValue(IloNumVar var) const;
  IloInt getValue(IloIntVar var) const;

  IloInt getDomainSize(IloCPEngine cp, IloNumVar var) const;
  
  IloInt getDomainSize(IloNumVar var) const;
  IloBool isInDomain(IloCPEngine cp, IloNumVar var, IloInt value) const;
  
  IloBool isInDomain(IloNumVar var, IloInt value) const;
  IloBool isFixed(IloCPEngine cp, IloNumVar var) const;
  IloBool isFixed(IloCPEngine cp, IloIntVar var) const;
  
  IloBool isFixed(IloNumVar var) const;
  IloBool isFixed(IloIntVar var) const;
  IloCPEngine::IntVarIterator iterator(IloCPEngine cp, IloNumVar var);
  
  IloCPEngine::IntVarIterator iterator(IloNumVar var);

  
  virtual void execute() = 0;
    
  virtual IloPropagatorI* makeClone(IloEnv env) const=0; 
  
  IloCPEngine getCP() const { return _cp; }
  void setCP(IloCPEngine cp);
  friend class IlcPropagatorConstraintI;
  friend class IntVarIterator;
};



IloConstraint IloCustomConstraint(IloEnv env, IloPropagatorI * prop);





#define ILOCPCOMMONARRAYDECL1(T, I, E, EI)                              \
private:                                                                \
  ILOCPVISIBLEHANDLEMINI(T, I)                                          \
private:                                                                \
  ILCEXTENSIONMETHODSHDECL(T)                                           \
private:                                                                \
                                                                        \
  void _ctor(IlcCPEngine s, IlcInt size);                               \
  void _ctor(IlcCPEngine s, IlcInt size, E * items);                    \
  void _ctor(IlcCPEngine s, E v1);                                      \
  void _ctor(IlcCPEngine s, E v1, E v2);                                \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3);                          \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4);                    \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5);              \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5, E v6);        \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5, E v6, E v7);  \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5, E v6, E v7, E v8); \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5, E v6, E v7, E v8, \
             E v9);                                                     \
  void _ctor(IlcCPEngine s, E v1, E v2, E v3, E v4, E v5, E v6, E v7, E v8, \
             E v9, E v10);                                              \
                                                                        \
  EI * _baseAddr() const { return (EI *)_impl; }                        \
  IlcInt * _sizeAddr() const { return (IlcInt*)((EI*)_impl - 1); }      \
  IlcCPEngineI ** _cpAddr() const { return (IlcCPEngineI **)((EI*)_impl - 2); } \
  IlcCPEngineI ** _solverAddr() const { return (IlcCPEngineI **)((EI*)_impl - 2); } \
  const char ** _nameAddr() const { return (const char **)((EI*)_impl - 3); } \
  IlcAny * _objAddr() const { return (IlcAny *)((EI*)_impl - 4); }      \
                                                                        \
  E & _get(IlcInt index) const { return ((E*)_baseAddr())[index]; }     \
  IlcInt _getSize() const { return *_sizeAddr(); }                      \
  IlcCPEngine _getCPEngine() const { return * _solverAddr(); }          \
  IlcCPEngineI* _getCPEngineI() const { return * _solverAddr(); }       \
  IlcManagerI* _getManagerI() const { return IlcGetManagerI(_getCPEngineI()); } \
  void _display(ILOSTD(ostream)& str) const;                            \
                                                                        \
public:                                                                 \
                                                                     \
  T(IlcCPEngine s, IlcInt size) {                                       \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size >= 0, ILO_STRINGIZE(T) "::" ILO_STRINGIZE(T)      \
                         " - size must be positive");                   \
    _ctor(s, size);                                                     \
  }                                                                     \
                                                                     \
  T(IlcCPEngine s, IlcInt size, E * items) {                            \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size >= 0, ILO_STRINGIZE(T) "::" ILO_STRINGIZE(T)      \
                         " - size must be positive");                   \
    _ctor(s, size, items);                                              \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1)  {                                                     \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 1, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 1");                          \
    _ctor(s, v1);                                                       \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2)  {                                         \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 2, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 2");                          \
    _ctor(s, v1, v2);                                                   \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3) {                              \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 3, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 3");                          \
    _ctor(s, v1, v2, v3);                                               \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4) {                  \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 4, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 4");                          \
    _ctor(s, v1, v2, v3, v4);                                           \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5) {      \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 5, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 5");                          \
    _ctor(s, v1, v2, v3, v4, v5);                                       \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5,        \
     const E v6) {                                                      \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 6, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 6");                          \
    _ctor(s, v1, v2, v3, v4, v5, v6);                                   \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5,        \
     const E v6, const E v7) {                                          \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 7, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 7");                          \
    _ctor(s, v1, v2, v3, v4, v5, v6, v7);                               \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5,        \
     const E v6, const E v7, const E v8) {                              \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 8, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 8");                          \
    _ctor(s, v1, v2, v3, v4, v5, v6, v7, v8);                           \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5,        \
     const E v6, const E v7, const E v8, const E v9) {                  \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 9, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)       \
                         " - size must be 9");                          \
    _ctor(s, v1, v2, v3, v4, v5, v6, v7, v8, v9);                       \
  }                                                                     \
  T (IlcCPEngine s, IlcInt ILCPARAM(size),                              \
     const E v1, const E v2, const E v3, const E v4, const E v5,        \
     const E v6, const E v7, const E v8, const E v9, const E v10) {     \
    IlcCPOAssert(s.getImpl() != 0, "IlcCPEngine: empty handle");        \
    IlcCPOAssert(size == 10, ILO_STRINGIZE(T) ":" ILO_STRINGIZE(T)      \
                          " - size must be 10");                        \
    _ctor(s, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);                  \
  }                                                                     \
                                                                     \
  IlcInt getSize() const {                                              \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(T) " : empty handle");       \
    return _getSize();                                                  \
  }                                                                     \
                                                                     \
  E & operator[] (IlcInt i) const {                                     \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(T) ": empty handle");        \
    IlcCPOAssert(i >= 0 && i < getSize(), ILO_STRINGIZE(T)              \
              ": index out of range");                                  \
    return _get(i);                                                     \
  }                                                                     \
  EI * getArray() const { return _baseAddr(); }                         \
                                                                     \
  IlcCPEngine getCPEngine() const {                                     \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(T) " : Empty handle");       \
    return _getCPEngine();                                              \
  }                                                                     \
  IlcManagerI* getManagerI() const {                                    \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(T) " : Empty handle");       \
    return _getManagerI();                                              \
  }                                                                     \
  IlcCPEngineI* getCPEngineI() const {                                  \
    IlcCPOAssert(_impl != 0, ILO_STRINGIZE(T) " : Empty handle");       \
    return _getCPEngineI();                                             \
  }


#ifdef CPPREF_GENERATION
#define ILOCPCOMMONARRAYDECL(T, E, EI)                                      \
        ILOCPCOMMONARRAYDECL1(T, IlcAny, E, EI)                             \
                                                                         \
  T(IlcCPEngine solver, IlcInt size, const E exp ...);
#else
#define ILOCPCOMMONARRAYDECL(T, E, EI)                                      \
        ILOCPCOMMONARRAYDECL1(T, IlcAny, E, EI)
#endif


////////////////////////////////////////////////////////////////////////////
//
// Arrays and sets of basic types
//
////////////////////////////////////////////////////////////////////////////

typedef IlcInt IlcIntArrayI;

class IlcIntArray {
private:
  IlcIntArrayI * _impl;

  void _display(ILOSTD(ostream)& out) const;
  void _ctor(IlcCPEngine s, IlcInt size, IlcInt* values);
  void _ctor(IlcCPEngine s, IlcInt size, IlcInt prototype);
public:
 
 IlcIntArray(IlcInt* impl = 0) : _impl(impl){}
 
 IlcIntArrayI * getImpl() const { return _impl; }
 IlcInt * getArray() const { return _impl; }
 ILCGETCPHDECL(IlcIntArray)

  IlcIntArray(IlcCPEngine solver, IlcInt size, IlcInt* values) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntArray: size must be non-negative");
    _ctor(solver, size, values);
  }

  IlcIntArray(IlcCPEngine solver, IlcInt size, IlcInt prototype = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntArray: size must be non-negative");
    _ctor(solver, size, prototype);
  }

  IlcIntArray(IlcCPEngine solver, IlcInt size, IlcInt exp0, IlcInt exp ...);
#if defined(ILO_HP11) || defined(ILO64)
  IlcIntArray(IlcCPEngine solver, IlcInt size, int exp0, int exp ...);
#endif


  IlcInt& operator[] (IlcInt i) const{
    IlcCPOAssert(_impl != 0, "IlcIntArray: empty handle");
    IlcCPOAssert(i >= 0 && i < getSize(), "IlcIntArray: index out of range");
    return _impl[i];
  }

  IlcIntExp operator[] (const IlcIntExp rank) const;

  IlcInt getSize() const {
    IlcCPOAssert(_impl != 0, "IlcIntArray: empty handle");
    return *(_impl - 1);
  }
  void display(ILOSTD(ostream)& str) const {
    IlcCPOAssert(_impl != 0, "IlcIntArray: empty handle");
    _display(str);
  }
  IlcInt getNumberOfRepetitions() const;
  IlcInt isIn(IlcInt value) const;
};

ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntArray& exp);


class IlcFloatArray {
  ILOCPHIDDENHANDLEMINI(IlcFloatArray, IlcFloatArrayI)
private:
  IloInt _getSize() const;
  void _display(ILOSTD(ostream)& str) const;
  IlcFloat& _get(IloInt i) const;
  void _ctor(IlcCPEngine solver, IlcInt size, IlcFloat prototype);
  void _ctor(IlcCPEngine solver, IlcInt size, IlcFloat * values);
public:

  IlcFloatArray(IlcCPEngine solver, IlcInt size, IlcFloat* values) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcFloatArray: size must be non-negative");
    _ctor(solver, size, values);
  }

  IlcFloatArray(IlcCPEngine solver, IlcInt size, IlcFloat prototype = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcFloatArray: size must be non-negative");
    _ctor(solver, size, prototype);
  }
#if defined(ILO_HP11) && defined(ILO64)
  IlcFloatArray(IlcCPEngine solver, IlcInt size,
                int exp0,
                IlcVarArgsNum exp ...);
  IlcFloatArray(IlcCPEngine solver, IlcInt size,
                IlcInt exp0,
                IlcVarArgsNum exp ...);
  IlcFloatArray(IlcCPEngine solver, IlcInt size,
                IlcFloat exp0,
                IlcVarArgsNum exp ...);
# else

  IlcFloatArray(IlcCPEngine solver, IlcInt size, IlcFloat exp0, IlcFloat exp1,...);

  IlcFloatArray(IlcCPEngine solver, IlcInt size, IlcInt exp0, IlcInt exp1,...);
#if !defined(ILOINTASINT)
  IlcFloatArray(IlcCPEngine solver, IlcInt size, int exp0, int exp ...);
#endif
#endif

  IlcFloat& operator[] (IlcInt i) const {
    IlcCPOAssert(_impl != 0, "IlcFloatArray: empty handle");
    IlcCPOAssert(i >= 0 && i < getSize(), "IlcFloatArray: index out of range");
    return _get(i);
  }


  IlcFloatExp operator[] (const IlcIntExp rank) const;


  IlcInt getSize() const {
    return (_impl == 0) ? 0 : _getSize();
  }
  void display(ILOSTD(ostream)& str) const {
    IlcCPOAssert(_impl != 0, "IlcFloatArray: empty handle");
    _display(str);
  }
  ILCGETCPHDECL(IlcFloatArray)
};


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcFloatArray& exp);


class IlcIntSet {
private:
  ILOCPHIDDENHANDLEMINI(IlcIntSet, IlcIntSetI)

  void _ctor(IlcCPEngine solver, IlcInt min, IlcInt max, IlcBool fullSet);
  void _ctor(IlcCPEngine solver, const IlcIntArray array, IlcBool fullSet);

  IlcInt   _getSize() const;
  IlcBool  _isIn(IlcInt elt) const;
  IlcBool  _add(IlcInt elt) const;
  IlcBool  _remove(IlcInt elt) const;
  void     _display(ILOSTD(ostream)& str) const;
  IlcIntSet _copy() const;
  void     _empty() const;
  void     _fill() const;
  IlcInt   _getMin() const;
  IlcInt   _getMax() const;
  IlcBool  _same(IlcIntSet set) const;
  IlcBool  _includes(IlcIntSet set) const;
  IlcBool  _intersects(IlcIntSet set) const;
public:

  IlcIntSet(IlcCPEngine solver, IlcInt min, IlcInt max, IlcBool fullSet = IlcTrue) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, min, max, fullSet);
  }

  IlcIntSet(IlcCPEngine solver, const IlcIntArray array, IlcBool fullSet = IlcTrue) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(array.getImpl() != 0, "IlcIntArray: empty handle");
    _ctor(solver, array, fullSet);
  }

  IlcInt getSize () const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _getSize();
  }

  IlcBool isIn(IlcInt elt) const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _isIn(elt);
  }

  IlcBool add(IlcInt elt){
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _add(elt);
  }

  IlcBool remove(IlcInt elt){
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _remove(elt);
  }
  void display(ILOSTD(ostream) &str) const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    _display(str);
  }

  IlcIntSet copy() const{
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _copy();
  }
  void empty() {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    _empty();
  }
  void fill(){
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    _fill();
  }
  IlcInt getMin() const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _getMin();
  }
  IlcInt getMax() const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    return _getMax();
  }
  IlcBool same(IlcIntSet set) const{
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    IlcCPOAssert(set._impl != 0, "IlcIntSet: empty handle");
    return _same(set);
  }
  IlcBool includes(IlcIntSet set) const{
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    IlcCPOAssert(set._impl != 0, "IlcIntSet: empty handle");
    return _includes(set);
  }
  IlcBool intersects(IlcIntSet set) const {
    IlcCPOAssert(_impl != 0, "IlcIntSet: empty handle");
    IlcCPOAssert(set._impl != 0, "IlcIntSet: empty handle");
    return _intersects(set);
  }
  ILCGETCPHDECL(IlcIntSet)
};


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntSet& exp);


class IlcIntSetIterator {
private:
  void*        _set;
  IlcInt       _curr;
  IlcBool      _ok;

  void _ctor(IlcIntSet set);
public:

  IlcIntSetIterator(IlcIntSet set) {
    IlcCPOAssert(set.getImpl() != 0, "IlcIntSet: empty handle");
    _ctor(set);
  }

  IlcBool ok() const { return _ok; }

  IlcInt operator*() const;

  IlcIntSetIterator& operator++();
};

////////////////////////////////////////////////////////////////////////////
//
// Reversible data structures
//
////////////////////////////////////////////////////////////////////////////

#ifndef ILCNOINT64

typedef IlcInt64 IlcStamp;

#else 

# if defined(ILO64)
class IlcStamp {
  friend class IlcManagerI;
private:
  IlcInt _stamp;
public:
  IlcStamp(IlcInt s=0) : _stamp(s) {}
  void operator ++(int) { _stamp++; }
  void operator++()     { _stamp++; }
  void operator --(int) { _stamp--; }
  void operator--()     { _stamp--; }
  IlcBool operator == (const IlcStamp & s) const {
    return _stamp == s._stamp;
  }
  void setMax()          { _stamp = 0x7FFFFFFFFFFFFFFF; }
  IlcBool isMax() const  { return _stamp == 0x7FFFFFFFFFFFFFFF; }
  void setMin()          { _stamp = (IlcInt)0x8000000000000000; }
  IlcBool isMin() const  { return _stamp == (IlcInt)0x8000000000000000; }
  void setZero()         { _stamp = 0; }
  IlcBool isZero() const { return _stamp == 0; }
  void save(IlcManagerI *);

  operator IlcInt() const { return _stamp; }
  friend IlcInt operator<(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator<=(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator>(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator>=(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator!=(const IlcStamp& s1, const IlcStamp& s2);
  friend class IlcIntExpI;

  friend ILOSTD(ostream)& operator << (ILOSTD(ostream) & out, const IlcStamp& s);
  operator IlcFloat() const { return (IlcFloat)_stamp; }
  friend IlcStamp operator-(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcStamp operator+(const IlcStamp& s1, const IlcStamp& s2);
};
inline IlcInt operator<(const IlcStamp& s1, const IlcStamp& s2) {
  return s1._stamp < s2._stamp;
}
inline IlcInt operator<=(const IlcStamp& s1, const IlcStamp& s2) {
  return s1._stamp <= s2._stamp;
}
inline IlcInt operator>(const IlcStamp& s1, const IlcStamp& s2) {
  return s1._stamp > s2._stamp;
}
inline IlcInt operator>=(const IlcStamp& s1, const IlcStamp& s2) {
  return s1._stamp >= s2._stamp;
}
inline IlcInt operator!=(const IlcStamp& s1, const IlcStamp& s2) {
  return s1._stamp != s2._stamp;
}
inline IlcStamp operator-(const IlcStamp& s1, const IlcStamp& s2) {
  return IlcStamp(s1._stamp - s2._stamp);
}
inline IlcStamp operator+(const IlcStamp& s1, const IlcStamp& s2) {
  return IlcStamp(s1._stamp + s2._stamp);
}
# else
class IlcStamp {
  friend class IlcManagerI;
private:
  IlcUInt _low;
  IlcInt  _high;
public:
  IlcStamp(IlcInt s=0) : _low(s), _high((s < 0) ? -1 : 0) { }
  IlcBool operator == (const IlcStamp & s) const {
    return !((_low ^ s._low) | (_high ^ s._high));
  }
  IlcBool operator != (const IlcStamp & s) const {
    return ((_low ^ s._low) | (_high ^ s._high));
  }
  void operator ++() {
    if (++_low == 0)
      ++_high;
  }
  void operator ++(int) {
    if (++_low == 0)
      ++_high;
  }
  void operator --() {
    if (_low-- == 0)
      --_high;
  }
  void operator --(int) {
    if (_low-- == 0)
      --_high;
  }
  void setMax() {
    _low  = 0xFFFFFFFF;
    _high = 0x7FFFFFFF;
  }
  IlcBool isMax() const { return _low == 0xFFFFFFFF && _high == 0x7FFFFFFF; }
  void setMin() {
    _low  = 0;
    _high = (IlcInt)0x80000000;
  }
  IlcBool isMin() const  { return _low == 0 && _high == (IlcInt)0x80000000; }
  void setZero()         { _low = _high = 0; }
  IlcBool isZero() const { return (_low | _high) == 0; }
  operator IlcInt() const {
    IlcCPOAssert(_high == 0 || _high == -1, "");
    return _low;
  }
  friend IlcInt operator<(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator<=(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator>(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcInt operator>=(const IlcStamp& s1, const IlcStamp& s2);
  friend ILOSTD(ostream)& operator<<(ILOSTD(ostream) & out, const IlcStamp& s);
  operator IlcFloat() const {
    return (IlcFloat)_low + (IlcFloat)_high*(IlcFloat)0x80000000;
  }
  friend IlcStamp operator-(const IlcStamp& s1, const IlcStamp& s2);
  friend IlcStamp operator+(const IlcStamp& s1, const IlcStamp& s2);
};
inline IlcInt operator<(const IlcStamp& s1, const IlcStamp& s2) {
  return (s1._high < s2._high) || (s1._high == s2._high && s1._low < s2._low);
}
inline IlcInt operator<=(const IlcStamp& s1, const IlcStamp& s2) {
  return (s1._high < s2._high) || (s1._high == s2._high && s1._low <= s2._low);
}
inline IlcInt operator>(const IlcStamp& s1, const IlcStamp& s2) {
  return (s1._high > s2._high) || (s1._high == s2._high && s1._low > s2._low);
}
inline IlcInt operator>=(const IlcStamp& s1, const IlcStamp& s2) {
  return (s1._low >= s2._low && s1._high == s2._high) || s1._high > s2._high;
}
#endif

#endif 


class IlcRevBool{
  IlcBool   _value;
  IlcStamp  _stamp;
  IlcRevBool(const IlcRevBool&) {}
  void operator= (const IlcRevBool&){}

  void _ctor(IlcCPEngine solver, IlcBool initValue);
  void _setValue(IlcCPEngine solver, IlcBool value);
public:
  IlcRevBool();

  IlcRevBool(IlcCPEngine solver, IlcBool initValue=IlcFalse) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, initValue);
  }
  IlcRevBool(IlcManagerI* solver, IlcBool initValue=IlcFalse);

  operator IlcBool() const { return _value; }

  IlcBool getValue() const { return _value; }

  void setValue(IlcCPEngine solver, IlcBool value) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _setValue(solver, value);
  }
  void setValue(IlcManagerI * manager, IlcBool value);
};

//---------------------------------------------------------------------


class IlcRevInt {
 protected:
  IlcInt   _value;
  IlcStamp _stamp;
  IlcRevInt(const IlcRevInt&){}
  void operator= (const IlcRevInt&){}

  void _ctor(IlcCPEngine solver, IlcInt initValue);
  void _setValue(IlcCPEngine solver, IlcInt value);
public:
  IlcRevInt();

  IlcRevInt(IlcCPEngine solver, IlcInt initValue=0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, initValue);
  }
  IlcRevInt(IlcManagerI * manager, IlcInt initValue=0);

  operator IlcInt() const { return _value; }

  IlcInt getValue() const { return _value;}

  void setValue(IlcCPEngine solver, IlcInt value) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _setValue(solver, value);
  }
  void setValue(IlcManagerI * manager, IlcInt value);
};

//---------------------------------------------------------------------


class IlcRevAny{
private:
  IlcAny  _value;
  IlcStamp _stamp;
  IlcRevAny(const IlcRevAny&){}
  void operator= (const IlcRevAny&) {}
  void _ctor(IlcCPEngine solver, IlcAny initValue);
  void _setValue(IlcCPEngine solver, IlcAny value);
public:
  IlcRevAny();

  IlcRevAny(IlcCPEngine solver, IlcAny initValue=0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, initValue);
  }
  IlcRevAny(IlcManagerI * manager, IlcAny initValue=0);

  operator IlcAny() const { return _value; }


  IlcAny getValue() const { return _value; }

  void setValue(IlcCPEngine solver, IlcAny value) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _setValue(solver, value);
  }
  void setValue(IlcManagerI * manager, IlcAny value);
};

//---------------------------------------------------------------------


class IlcRevFloat{
private:
  IlcFloat  _value;
  IlcStamp   _stamp;
  IlcRevFloat(const IlcRevFloat&){}
  void operator= (const IlcRevFloat&){}

  void _ctor(IlcCPEngine solver, IlcFloat initValue);
  void _setValue(IlcCPEngine solver, IlcFloat value);

public:
  IlcRevFloat();

  IlcRevFloat(IlcCPEngine solver, IlcFloat initValue = 0.) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, initValue);
  }
  IlcRevFloat(IlcManagerI * manager, IlcFloat initValue = 0.);

  operator IlcFloat() const { return _value; }

  IlcFloat getValue() const { return _value; }

  void setValue(IlcCPEngine solver, IlcFloat value) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _setValue(solver, value);
  }
  void setValue(IlcManagerI * manager, IlcFloat value);
};

////////////////////////////////////////////////////////////////////////////
//
// Demons
//
////////////////////////////////////////////////////////////////////////////

class IlcIDemonI  : public IlcRtti  {
  friend class IlcCPODemonIList;
  friend class IlcManagerI;
private:
  IlcStamp         _lastPropStamp;
public:
  IlcIDemonI():
    _lastPropStamp(0)
  {}
  IlcIDemonI(IlcManagerI*):
    _lastPropStamp(0)
  {}
  virtual ~IlcIDemonI() {}
  virtual void propagate() = 0;
  virtual void display(ILOSTD(ostream) &) const;
#ifndef ILCNOINT64
  IlcBool isInhibited() const { return _lastPropStamp == IlcInt64Max; }
#else
  IlcBool isInhibited() const { return _lastPropStamp.isMax(); }
#endif
  void setPropagationStamp(IlcStamp s) {
    IlcCPOAssert(!isInhibited(), "");
    _lastPropStamp = s;
  }
  IlcStamp getPropagationStamp() const { return _lastPropStamp; }
  void callPropagateDemon(IlcStamp, IlcStamp);
  void callPropagateDemon(IlcManagerI* m);
  void propagateOn(class IlcCPOEventHandlerI *);
  const char* getName() const { return 0; }
  virtual IlcRecomputeExprI* getExpr(); 
  ILCRTTIDECL 
};


class IlcDemonI 
#ifndef CPPREF_GENERATION
: public IlcIDemonI
#endif
{
  friend class IlcManagerI;
private:
  IlcManagerI*     _manager;
protected:
  IlcExtension   * _ext;
private:
  IlcConstraintI * _owner;
public:

  IlcDemonI(IlcCPEngine solver, IlcConstraintI * owner = 0);
  IlcDemonI(IlcManagerI * manager, IlcConstraintI * owner = 0);
  virtual ~IlcDemonI();
  void setOwner(IlcConstraintI * ct) { _owner = ct; }
  IlcConstraintI * getOwner() const { return _owner; }


  IlcCPEngineI* getCPEngineI() const { return IlcGetCPEngineI(_manager); }
  const IlcCPI* getCPI() const { return IlcGetCPI(_manager); } // for compat
  IlcManagerI * getManagerI() const { return _manager; }
  IlcCPEngine getManager() const { return getCPEngineI(); }

  IlcCPEngine getCPEngine() const { return _manager; }
  // undocumented


  IlcConstraintI * getConstraintI() const { return getOwner(); }


  virtual void propagate() = 0;
  virtual void display(ILOSTD(ostream) &) const;
  IlcGoalI * toGoal();
#ifdef ILC_HAS_PROPAGATION_TRACE
  void callPropagateDemonWithMonitor(IlcStamp, IlcStamp, IlcQueueMonitorI*);
  void callPropagateDemonWithMonitor(IlcQueueMonitorI*);
#endif 

#ifdef ILC_HAS_PROPAGATION_TRACE
  virtual IlcBool isTraceDemon() const;
#endif

  virtual IlcBool isAConstraint() const { return IlcFalse; }
  ILCEXTENSIONMETHODSIDECL 
  ILCRTTIDECL 
};


class IlcDemon {
  ILOCPVISIBLEHANDLEMINI(IlcDemon, IlcDemonI)
public:
  void propagate() const {
    IlcCPOAssert(_impl != 0, "IlcDemon: empty handle");
    _impl->propagate();
  }
  operator IlcIDemonI*() const {
    return getImpl();
  }
  
  inline IlcConstraint getConstraint() const;
  ILCEXTENSIONMETHODSHDECL(IlcDemon)
  ILCGETCPINLINEHDECL(IlcDemon)
};


inline ILOSTD(ostream) & operator << (ILOSTD(ostream) & s, IlcDemon d) {
  d.getImpl()->display(s);
  return s;
}

////////////////////////////////////////////////////////////////////////////
//
// Goals
//
////////////////////////////////////////////////////////////////////////////


class IlcGoalI  : public IlcRtti  {
private:
  IlcManagerI* _manager;
protected:
  IlcExtension * _ext;
public:
  
  IlcGoalI(IlcManagerI * manager) : _manager(manager), _ext(0) { }

  IlcGoalI(IlcCPEngine solver) : _manager(solver.getManagerI()), _ext(0) { }
  virtual ~IlcGoalI();


  IlcCPEngineI*  getCPEngineI() const { return IlcGetCPEngineI(_manager); }
  const IlcCPI*      getCPI() const { return IlcGetCPI(_manager); }
  IlcManagerI*  getManagerI() const { return _manager; }



  IlcCPEngine getCPEngine() const { return getCPEngineI(); }
  IlcCPEngine getManager() const { return getCPEngineI(); }


  void fail();

  void fail(IlcAny label);




  virtual IlcGoal execute() = 0;
  virtual void display(ILOSTD(ostream) &) const;

  class IlcIDemonI * toDemon();

  ILCEXTENSIONMETHODSIDECL 
  ILCRTTIDECL 
};


class IlcGoal {
  ILOCPHANDLEINLINE(IlcGoal,IlcGoalI)
  ILCGETCPINLINEHDECL(IlcGoal)
public:
  IlcGoal execute() const {
    IlcCPOAssert(_impl != 0, "IlcGoal::execute - empty handle");
    return _impl->execute();
  }
  
   IlcGoal(class IlcConstraint);
};


inline ILOSTD(ostream) & operator << (ILOSTD(ostream) & s, IlcGoal g) {
  g.getImpl()->display(s);
  return s;
}


////////////////////////////////////////////////////////////////////////////
//
// Constraints
//
////////////////////////////////////////////////////////////////////////////


class IlcConstraintI : public IlcDemonI {
friend class IlcManagerI;
protected:
  IlcConstraintI *       _opposite;
private:
  IlcUInt                _flags;

  void *                 _boolvar;
protected:
  IlcIntVarI* getBoolVar() const { return (IlcIntVarI*) _boolvar; }
  void setBoolVar(IlcIntVarI* var) { _boolvar = var; }
private:

  IlcBool getFlag(IlcInt bit) const {
    return (_flags >> bit) & 1;
  }
  void setFlag(IlcInt bit) {
    _flags |= (IlcInt(1) << bit);
  }
  void clearFlag(IlcInt bit) {
    _flags = _flags & ~(IlcInt(1) << bit);
  }
protected:
  void setHasBeenAdded();
  void setHasBeenPosted();
  void setHasBeenPropagated() {
    IlcCPOAssert(hasBeenPosted(), "Function post must be called before propagate.");
    setFlag(2);
  }
public:
  IlcBool hasBeenAdded() const { return getFlag(0); }
  IlcBool hasBeenPosted() const { return getFlag(1); }
  IlcBool hasBeenPropagated() const { return getFlag(2); }

  void queue() { setFlag(3); }
  void dequeue() { clearFlag(3); }
  void restore() { dequeue(); }
  IlcBool isInQueue() const { return getFlag(3); }

  void setSilent() { setFlag(4); }
  void setNonSilent() { clearFlag(4); }
  IlcBool isSilent() const { return getFlag(4); }

public:

  IlcConstraintI(IlcCPEngine cp);
  IlcConstraintI(IlcManagerI * manager);

  IlcConstraintI* getOpposite();
  void add(IlcConstraintI *);
  inline void add(class IlcConstraint ct);

  void propagateTrue();
  virtual void postAndPropagate();
  virtual void prepropagate();
  void prepropagateWrapper();
  IlcBool isFalse() const;
  IlcBool isTrue() const;

  void push();

  void push(IlcInt priority);
  void whenFalse(IlcIDemonI* d);
  void whenFalse(IlcDemon d);
  void whenTrue(IlcDemon d);
  void whenTrue(IlcIDemonI* d);


  void fail(IlcAny label = 0);
  virtual IlcBool isPreprocessed() const;
  IlcBool hasOpposite() const { return _opposite != 0; }
#ifdef NDEBUG
  void setParentDemonI(IlcDemonI *) { }
#else
  void setParentDemonI(IlcDemonI * d) { IlcCPOAssert(d == this, ""); }
#endif
  IlcDemonI * getParentDemonI() const { return (IlcConstraintI *)this; }

  virtual void post() = 0;

  virtual void propagate() = 0;

  virtual void metaPostDemon(IlcDemonI * demon);
  virtual void metaPost(IlcGoalI * goal); // deprecated

  virtual IlcConstraintI * makeOpposite() const;

  virtual IlcBool isViolated() const;
  virtual IlcBool setFilterLevel(const IlcFilterLevel level);
  virtual IlcInt getAddPriority();
  virtual IlcBool isFreezable() const { return IlcFalse; }

  virtual void display(ILOSTD(ostream) &) const;

  virtual IlcBool isAConstraint() const { return IlcTrue; }
  IlcBool isPosted() const { return hasBeenPosted(); }
  void setTrue();
  void setFalse();
  virtual void linkBoolIntExp(IlcIntVarI*);
  IlcIntVarI* getBoolIntExp() const { return (IlcIntVarI*)_boolvar; }
  void setBoolIntExp(IlcIntVarI* b);
  virtual IlcIntVarI * createBoolIntExp() const;
  ILCID;
public:
  // fill last branch
  virtual void fillLastBranch() const;
};


class IlcConstraint {
  ILOCPHANDLEINLINE(IlcConstraint,IlcConstraintI)
  ILCGETCPINLINEHDECL(IlcConstraint)
public:
  void post() {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    _impl->post();
  }
  void metaPostDemon(IlcDemonI * demon) {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    IlcCPOAssert(demon != 0, "IlcDemonI: null pointer");
    _impl->metaPostDemon(demon);
  }
  void whenFalse(IlcDemon d) const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    IlcCPOAssert(d.getImpl() != 0, "IlcDemon: empty handle");
    _impl->whenFalse(d);
  }
  void whenTrue(IlcDemon d) const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    IlcCPOAssert(d.getImpl() != 0, "IlcDemon: empty handle");
    _impl->whenTrue(d);
  }

  IlcBool isFalse() const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    return _impl->isFalse();
  }

  IlcBool isTrue() const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    return _impl->isTrue();
  }
  void setFilterLevel(IlcFilterLevel level) const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    _impl->setFilterLevel(level);
  }
  IlcBool isPosted() const {
    IlcCPOAssert(_impl != 0, "IlcConstraint: empty handle");
    return _impl->isPosted();
  }
};

void IlcConstraintI::add(IlcConstraint ct) { add(ct.getImpl()); }

IlcConstraint IlcDemon::getConstraint() const {
  IlcCPOAssert(_impl != 0, "IlcDemon: empty handle");
  return _impl->getConstraintI();
}


IlcConstraint operator ! (const IlcConstraint ct);


IlcConstraint operator || (const IlcConstraint ct1, const IlcConstraint ct2);


IlcConstraint operator && (const IlcConstraint ct1, const IlcConstraint ct2);


IlcConstraint operator == (const IlcConstraint ct1, const IlcConstraint ct2);


IlcConstraint operator != (const IlcConstraint ct1, const IlcConstraint ct2);

IlcConstraint operator <= (const IlcConstraint ct1, const IlcConstraint ct2);
inline IlcConstraint operator >= (const IlcConstraint ct1, const IlcConstraint ct2) {
  return ct2 <= ct1;
}


void IlcIfThen(const IlcConstraint ct1, const IlcConstraint ct2);

IlcGoal IlcGoalFail(IlcCPEngine solver, IlcAny label=0);
IlcGoal IlcGoalTrue(IlcCPEngine solver);
IlcGoal IlcOnce(IlcGoal goal);


IlcGoal IlcAnd(const IlcGoal g1, const IlcGoal g2);


IlcGoal IlcAnd(const IlcGoal g1,
          const IlcGoal g2,
          const IlcGoal g3);


IlcGoal IlcAnd(const IlcGoal g1,
          const IlcGoal g2,
          const IlcGoal g3,
          const IlcGoal g4);


IlcGoal IlcAnd(const IlcGoal g1,
          const IlcGoal g2,
          const IlcGoal g3,
          const IlcGoal g4,
          const IlcGoal g5);


IlcGoal IlcOr(const IlcGoal g1, const IlcGoal g2, IlcAny label =0);


IlcGoal IlcOr(const IlcGoal g1,
         const IlcGoal g2,
         const IlcGoal g3,
         IlcAny label =0);


IlcGoal IlcOr(const IlcGoal g1,
         const IlcGoal g2,
         const IlcGoal g3,
         const IlcGoal g4,
         IlcAny label =0);


IlcGoal IlcOr(const IlcGoal g1,
         const IlcGoal g2,
         const IlcGoal g3,
         const IlcGoal g4,
         const IlcGoal g5,
         IlcAny label =0);



ILOSTD(ostream)& operator<<(ILOSTD(ostream)& str,  const IlcConstraint& f);

//////////////////////////////////////////////////////////////////////////////
//
// ILCGOAL and ILCDEMON Macros
//
//////////////////////////////////////////////////////////////////////////////
//


#define ILCGOALRTTIDECL ILCRTTIDECL
#define ILCGOALRTTI(x,y) ILCRTTI(x, y)
#define ILCGOALRTTI1(x,y,t) ILCRTTI1(x,y,t)

// ILCGOAL0
#define ILCGOALSTART0(name, returnType)
#define ILCGOALAUX0(name, envName, returnType)

#define ILCGOALNAME0(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute,returnType)\
ILCGOALSTART0(name,returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    envName(IlcCPEngine solver):IlcGoalClass(solver){}\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcGoalClass) \
ILCGOALAUX0(name, envName,returnType)\
returnType name(IlcCPEngine solver){\
 return new (solver.getHeap()) envName(solver);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName(); \
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL0(name)\
ILCGOALNAME0(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, IlcGoal)

#define ILCDEMON0(name)\
ILCGOALNAME0(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, IlcDemon)

// ILCGOAL1


#define ILCGOALSTART1(name, t1, returnType)
#define ILCGOALAUX1(name, envName, t1, returnType)


#define ILCGOALNAME1(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, returnType)\
ILCGOALSTART1(name, t1, returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    envName(IlcCPEngine solver, t1 IlcArg1);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1)\
    :IlcGoalClass(solver),nA1(IlcArg1){}\
ILCGOALRTTI(envName,IlcGoalClass) \
ILCGOALAUX1(name, envName, t1, returnType)\
returnType name(IlcCPEngine solver, t1 arg1){\
 return new (solver.getHeap()) envName(solver, arg1);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL1(name, t1, nA1)\
ILCGOALNAME1(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, IlcGoal)

#define ILCDEMON1(name, t1, nA1)\
ILCGOALNAME1(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, IlcDemon)

// ILCGOAL2


#define ILCGOALSTART2(name, t1, t2, returnType)
#define ILCGOALAUX2(name, envName, t1, t2, returnType)


#define ILCGOALNAME2(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, t2, nA2,returnType)\
ILCGOALSTART2(name, t1, t2,returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2)\
   :IlcGoalClass(solver),nA1(IlcArg1), nA2(IlcArg2){}\
ILCGOALAUX2(name, envName, t1, t2,returnType)\
ILCGOALRTTI(envName,IlcGoalClass) \
returnType name(IlcCPEngine solver, t1 arg1, t2 arg2){\
 return new (solver.getHeap()) envName(solver, arg1, arg2);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL2(name, t1, nA1, t2, nA2)\
ILCGOALNAME2(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, t2, nA2, IlcGoal)

#define ILCDEMON2(name, t1, nA1, t2, nA2)\
ILCGOALNAME2(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, t2, nA2, IlcDemon)

// ILCGOAL3


#define ILCGOALSTART3(name, t1, t2, t3, returnType)
#define ILCGOALAUX3(name, envName, t1, t2, t3, returnType)


#define ILCGOALNAME3(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, t2, nA2, t3, nA3, returnType)\
ILCGOALSTART3(name, t1, t2, t3, returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3)\
   :IlcGoalClass(solver),nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3){}\
ILCGOALAUX3(name, envName, t1, t2, t3, returnType)\
ILCGOALRTTI(envName,IlcGoalClass) \
returnType name(IlcCPEngine solver, t1 arg1, t2 arg2, t3 arg3){\
 return new (solver.getHeap()) envName(solver, arg1, arg2, arg3);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL3(name, t1, nA1, t2, nA2, t3, nA3)\
ILCGOALNAME3(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, t2, nA2, t3, nA3, IlcGoal)

#define ILCDEMON3(name, t1, nA1, t2, nA2, t3, nA3)\
ILCGOALNAME3(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, t2, nA2, t3, nA3, IlcDemon)

// ILCGOAL4


#define ILCGOALSTART4(name, t1, t2, t3, t4, returnType)
#define ILCGOALAUX4(name, envName, t1, t2, t3, t4, returnType)


#define ILCGOALNAME4(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, returnType)\
ILCGOALSTART4(name, t1, t2, t3, t4, returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4)\
   :IlcGoalClass(solver),nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4){}\
ILCGOALAUX4(name, envName, t1, t2, t3, t4, returnType)\
ILCGOALRTTI(envName,IlcGoalClass) \
returnType name(IlcCPEngine solver, t1 arg1, t2 arg2, t3 arg3, t4 arg4){\
 return new (solver.getHeap()) envName(solver, arg1, arg2, arg3, arg4);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL4(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4)\
ILCGOALNAME4(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, IlcGoal)

#define ILCDEMON4(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4)\
ILCGOALNAME4(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, t2, nA2, t3, nA3, t4, nA4, IlcDemon)

// ILCGOAL5


#define ILCGOALSTART5(name, t1, t2, t3, t4, t5, returnType)
#define ILCGOALAUX5(name, envName, t1, t2, t3, t4, t5, returnType)


#define ILCGOALNAME5(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, returnType)\
ILCGOALSTART5(name, t1, t2, t3, t4, t5, returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    t5 nA5; \
    envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5)\
   :IlcGoalClass(solver),nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4), nA5(IlcArg5){}\
ILCGOALAUX5(name, envName, t1, t2, t3, t4, t5, returnType)\
ILCGOALRTTI(envName,IlcGoalClass) \
returnType name(IlcCPEngine solver, t1 arg1, t2 arg2, t3 arg3, t4 arg4, t5 arg5){\
 return new (solver.getHeap()) envName(solver, arg1, arg2, arg3, arg4, arg5);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL5(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5)\
ILCGOALNAME5(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, IlcGoal)

#define ILCDEMON5(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5)\
ILCGOALNAME5(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, IlcDemon)

// ILCGOAL6


#define ILCGOALSTART6(name, t1, t2, t3, t4, t5, t6, returnType)
#define ILCGOALAUX6(name, envName, t1, t2, t3, t4, t5, t6, returnType)


#define ILCGOALNAME6(name, envName, IlcGoalClass, IlcGoalRet, IlcExecute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, t6, nA6, returnType)\
ILCGOALSTART6(name, t1, t2, t3, t4, t5, t6, returnType)\
class envName : public IlcGoalClass { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    t5 nA5; \
    t6 nA6; \
    envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5, t6 IlcArg6);\
    IlcGoalRet IlcExecute();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
envName::envName(IlcCPEngine solver, t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5, t6 IlcArg6)\
   :IlcGoalClass(solver),nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4), nA5(IlcArg5), nA6(IlcArg6){}\
ILCGOALAUX6(name, envName, t1, t2, t3, t4, t5, t6, returnType)\
ILCGOALRTTI(envName,IlcGoalClass) \
returnType name(IlcCPEngine solver, t1 arg1, t2 arg2, t3 arg3, t4 arg4, t5 arg5, t6 arg6){\
 return new (solver.getHeap()) envName(solver, arg1, arg2, arg3, arg4, arg5, arg6);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
IlcGoalRet envName :: IlcExecute()


#define ILCGOAL6(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, t6, nA6)\
ILCGOALNAME6(name, ILCCONCAT(name,I), IlcGoalI, IlcGoal, execute, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, t6, nA6, IlcGoal)

#define ILCDEMON6(name, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, t6, nA6)\
ILCGOALNAME6(name, ILCCONCAT(name,I), IlcDemonI, void, propagate, t1, nA1, t2, nA2, t3, nA3, t4, nA4, t5, nA5, t6, nA6, IlcDemon)


//---------------------------------------------------------------------
// ILCCTDEMON MACROS
//---------------------------------------------------------------------

// ILCCTDEMON0

#define ILCCTDEMONNAME0(name, envName, IlcCtClass, IlcFnName)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    envName(IlcCPEngine solver,IlcConstraintI* ct);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct) \
:IlcDemonI(solver, ct){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct){\
 return new (solver.getHeap()) envName(solver, ct);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName();\
}


#define ILCCTDEMON0(name,IlcCtClass,IlcFnName)\
ILCCTDEMONNAME0(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName)

// ILCCTDEMON1

#define ILCCTDEMONNAME1(name, envName, IlcCtClass, IlcFnName, t1,nA1)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1)\
:IlcDemonI(solver, ct), nA1(IlcArg1){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1){\
 return new (solver.getHeap()) envName(solver,ct,arg1);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1);\
}


#define ILCCTDEMON1(name,IlcCtClass,IlcFnName,t1,nA1)\
ILCCTDEMONNAME1(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1)

// ILCCTDEMON2

#define ILCCTDEMONNAME2(name, envName, IlcCtClass, IlcFnName, t1,nA1,t2,nA2)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1, t2 IlcArg2);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1,t2 IlcArg2)\
:IlcDemonI(solver, ct), nA1(IlcArg1), nA2(IlcArg2){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1, t2 arg2){\
 return new (solver.getHeap()) envName(solver,ct,arg1,arg2);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1,nA2);\
}


#define ILCCTDEMON2(name,IlcCtClass,IlcFnName,t1,nA1,t2,nA2)\
ILCCTDEMONNAME2(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1,t2,nA2)

// ILCCTDEMON3

#define ILCCTDEMONNAME3(name, envName, IlcCtClass, IlcFnName, t1,nA1,t2,nA2,t3,nA3)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1, t2 IlcArg2, t3 IlcArg3);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1,t2 IlcArg2, t3 IlcArg3):\
IlcDemonI(solver, ct), nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1, t2 arg2, t3 arg3){\
 return new (solver.getHeap()) envName(solver,ct,arg1,arg2,arg3);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1,nA2,nA3);\
}


#define ILCCTDEMON3(name,IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3)\
ILCCTDEMONNAME3(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3)

// ILCCTDEMON4

#define ILCCTDEMONNAME4(name, envName, IlcCtClass, IlcFnName, t1,nA1,t2,nA2,t3,nA3,t4,nA4)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1,t2 IlcArg2, t3 IlcArg3, t4 IlcArg4)\
:IlcDemonI(solver, ct), nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1, t2 arg2, t3 arg3, t4 arg4){\
 return new (solver.getHeap()) envName(solver,ct,arg1,arg2,arg3,arg4);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1,nA2,nA3,nA4);\
}


#define ILCCTDEMON4(name,IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4)\
ILCCTDEMONNAME4(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4)

// ILCCTDEMON5

#define ILCCTDEMONNAME5(name, envName, IlcCtClass, IlcFnName, t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    t5 nA5; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1,t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5)\
:IlcDemonI(solver, ct), nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4), nA5(IlcArg5){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1, t2 arg2, t3 arg3, t4 arg4, t5 arg5){\
 return new (solver.getHeap()) envName(solver,ct,arg1,arg2,arg3,arg4,arg5);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1,nA2,nA3,nA4,nA5);\
}


#define ILCCTDEMON5(name,IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5)\
ILCCTDEMONNAME5(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5)

// ILCCTDEMON6

#define ILCCTDEMONNAME6(name, envName, IlcCtClass, IlcFnName, t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5,t6,nA6)\
class envName : public IlcDemonI { \
  ILCGOALRTTIDECL \
  public:\
    t1 nA1; \
    t2 nA2; \
    t3 nA3; \
    t4 nA4; \
    t5 nA5; \
    t6 nA6; \
    envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1, t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5, t6 IlcArg6);\
    void propagate();\
    void display (ILOSTD(ostream) &ilc_stream) const;\
};\
ILCGOALRTTI(envName,IlcDemonI) \
envName::envName(IlcCPEngine solver,IlcConstraintI* ct,t1 IlcArg1,t2 IlcArg2, t3 IlcArg3, t4 IlcArg4, t5 IlcArg5, t6 IlcArg6)\
:IlcDemonI(solver, ct), nA1(IlcArg1), nA2(IlcArg2), nA3(IlcArg3), nA4(IlcArg4), nA5(IlcArg5), nA6(IlcArg6){ }\
IlcDemon name(IlcCPEngine solver, IlcConstraintI* ct, t1 arg1, t2 arg2, t3 arg3, t4 arg4, t5 arg5, t6 arg6){\
 return new (solver.getHeap()) envName(solver,ct,arg1,arg2,arg3,arg4,arg5,arg6);\
}\
void envName ::display (ILOSTD(ostream) &ilc_stream) const {\
  if (getName() != 0) ilc_stream << getName();\
  else ilc_stream<< #name ;\
}\
void envName ::propagate(){\
    ((IlcCtClass*)getConstraintI())->IlcFnName(nA1,nA2,nA3,nA4,nA5,nA6);\
}


#define ILCCTDEMON6(name,IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5,t6,nA6)\
ILCCTDEMONNAME6(name, ILCCONCAT(name,I),IlcCtClass,IlcFnName,t1,nA1,t2,nA2,t3,nA3,t4,nA4,t5,nA5,t6,nA6)


#define ILOCPCONSTRAINTWRAPPERDECL ILORTTIDECL
#define ILOCPCONSTRAINTWRAPPERIMPL(x) ILORTTI(x, IloCPConstraintI)
#define ILOCPCONSTRAINTWRAPPER(x) ILORTTI(x, IloCPConstraintI)

#define ILOCPCONSTRAINTWRAPPERMEMBERS0DECL(_this)\
  ILOCPCONSTRAINTWRAPPERDECL \
public:\
  ILOEXTRCONSTRUCTOR0DECL(_this,IloCPConstraintI) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;

#define ILOCPCONSTRAINTWRAPPERMEMBERS0(_this)\
  ILOCPCONSTRAINTWRAPPERIMPL(_this) \
  ILOEXTRCONSTRUCTOR0(_this,IloCPConstraintI) \
  ILOEXTRMAKECLONE0(_this) \
  ILOEXTRDISPLAY0(_this)


#define ILOCPCONSTRAINTWRAPPERMEMBERS1DECL(_this, t1, a1) \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
public:\
  ILOEXTRCONSTRUCTOR1DECL(_this,IloCPConstraintI,t1,a1) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;


#define ILOCPCONSTRAINTWRAPPERMEMBERS1(_this, t1, a1) \
  ILOCPCONSTRAINTWRAPPERIMPL(_this) \
  ILOEXTRCONSTRUCTOR1(_this,IloCPConstraintI,t1,a1) \
  ILOEXTRMAKECLONE1(_this,t1,a1) \
  ILOEXTRDISPLAY1(_this,t1,a1)

#define ILOCPCONSTRAINTWRAPPERMEMBERS2DECL(_this, t1, a1, t2, a2) \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
  t2 a2;\
public:\
  ILOEXTRCONSTRUCTOR2DECL(_this,IloCPConstraintI,t1,a1,t2,a2) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;

#define ILOCPCONSTRAINTWRAPPERMEMBERS2(_this, t1, a1,  t2, a2) \
  ILOCPCONSTRAINTWRAPPERIMPL(_this) \
  ILOEXTRCONSTRUCTOR2(_this,IloCPConstraintI,t1,a1,t2,a2) \
  ILOEXTRMAKECLONE2(_this,t1,a1,t2,a2) \
  ILOEXTRDISPLAY2(_this,t1,a1,t2,a2)

#define ILOCPCONSTRAINTWRAPPERMEMBERS3DECL(_this, \
                            t1, a1, \
                            t2, a2, \
                            t3, a3) \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
public:\
  ILOEXTRCONSTRUCTOR3DECL(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;

#define ILOCPCONSTRAINTWRAPPERMEMBERS3(_this, \
                        t1, a1, \
                        t2, a2, \
                        t3, a3) \
  ILOCPCONSTRAINTWRAPPERIMPL(_this) \
  ILOEXTRCONSTRUCTOR3(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3) \
  ILOEXTRMAKECLONE3(_this,t1,a1,t2,a2,t3,a3) \
  ILOEXTRDISPLAY3(_this,t1,a1,t2,a2,t3,a3)

#define ILOCPCONSTRAINTWRAPPERMEMBERS4DECL(_this, \
                            t1, a1, \
                            t2, a2, \
                            t3, a3, \
                            t4, a4) \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
public:\
  ILOEXTRCONSTRUCTOR4DECL(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;

#define ILOCPCONSTRAINTWRAPPERMEMBERS4(_this, \
                        t1, a1, \
                        t2, a2, \
                        t3, a3, \
                        t4, a4) \
  ILOCPCONSTRAINTWRAPPERIMPL(_this) \
  ILOEXTRCONSTRUCTOR4(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4) \
  ILOEXTRMAKECLONE4(_this,t1,a1,t2,a2,t3,a3,t4,a4) \
  ILOEXTRDISPLAY4(_this,t1,a1,t2,a2,t3,a3,t4,a4)

#define ILOCPCONSTRAINTWRAPPERMEMBERS5DECL(_this, \
                                           t1, a1,      \
                                           t2, a2,      \
                                           t3, a3,      \
                                           t4, a4, \
                                           t5, a5)         \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
  t5 a5;\
public:\
  ILOEXTRCONSTRUCTOR5DECL(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;


#define ILOCPCONSTRAINTWRAPPERMEMBERS5(_this, \
                                       t1, a1,  \
                                       t2, a2,  \
                                       t3, a3,  \
                                       t4, a4,  \
                                       t5, a5)  \
  ILOCPCONSTRAINTWRAPPERIMPL(_this)                                     \
    ILOEXTRCONSTRUCTOR5(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5) \
    ILOEXTRMAKECLONE5(_this,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5)              \
    ILOEXTRDISPLAY5(_this,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5)

#define ILOCPCONSTRAINTWRAPPERMEMBERS6DECL(_this, \
                                           t1, a1,      \
                                           t2, a2,      \
                                           t3, a3,      \
                                           t4, a4, \
                                           t5, a5, \
                                           t6, a6)         \
  ILOCPCONSTRAINTWRAPPERDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
  t5 a5;\
  t6 a6;\
public:\
  ILOEXTRCONSTRUCTOR6DECL(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6) \
  ILOEXTRMAKECLONEDECL \
  ILOEXTRDISPLAYDECL \
  IlcConstraint extract(const IloCPEngine) const;


#define ILOCPCONSTRAINTWRAPPERMEMBERS6(_this, \
                                       t1, a1,  \
                                       t2, a2,  \
                                       t3, a3,  \
                                       t4, a4,  \
                                       t5, a5,\
                                       t6, a6)                          \
  ILOCPCONSTRAINTWRAPPERIMPL(_this)                                     \
    ILOEXTRCONSTRUCTOR6(_this,IloCPConstraintI,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6) \
    ILOEXTRMAKECLONE6(_this,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6)                \
    ILOEXTRDISPLAY6(_this,t1,a1,t2,a2,t3,a3,t4,a4,t5,a5,t6,a6)


#define ILOCPCONSTRAINTWRAPPER0(_this, solver) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS0DECL(ILCCONCAT(_this, I))\
};\
ILOCPCONSTRAINTWRAPPERMEMBERS0(ILCCONCAT(_this, I))\
IloConstraint _this(IloEnv env, const char* name=0) {\
  ILCCONCAT(_this, I)::InitTypeIndex();\
  return new (env) ILCCONCAT(_this, I)(env.getImpl(), name);\
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER1(_this, solver, t1, a1) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS1DECL(ILCCONCAT(_this, I), t1, a1)\
};\
ILOCPCONSTRAINTWRAPPERMEMBERS1(ILCCONCAT(_this, I), t1, a1)\
IloConstraint _this(IloEnv env, t1 a1, const char* name=0) {\
  ILCCONCAT(_this, I)::InitTypeIndex();\
  return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, name);\
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER2(_this, solver, t1, a1, t2, a2) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS2DECL(ILCCONCAT(_this, I), t1, a1, t2, a2)\
};\
ILOCPCONSTRAINTWRAPPERMEMBERS2(ILCCONCAT(_this, I), t1, a1, t2, a2)\
IloConstraint _this(IloEnv env, t1 a1, t2 a2, const char* name=0) {\
  ILCCONCAT(_this, I)::InitTypeIndex();\
  return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, a2, name);\
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER3(_this, solver, t1, a1, t2, a2, t3, a3) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS3DECL(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3)\
};\
ILOCPCONSTRAINTWRAPPERMEMBERS3(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3)\
IloConstraint _this(IloEnv env, t1 a1, t2 a2, t3 a3, const char* name=0) {\
  ILCCONCAT(_this, I)::InitTypeIndex();\
  return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, a2, a3, name);\
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER4(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS4DECL(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4)\
};\
ILOCPCONSTRAINTWRAPPERMEMBERS4(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4)\
IloConstraint _this(IloEnv env, t1 a1, t2 a2, t3 a3, t4 a4, const char* name=0) {\
  ILCCONCAT(_this, I)::InitTypeIndex();\
  return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, a2, a3, a4, name);\
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER5(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS5DECL(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
};\
  ILOCPCONSTRAINTWRAPPERMEMBERS5(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
    IloConstraint _this(IloEnv env, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, const char* name=0) { \
    ILCCONCAT(_this, I)::InitTypeIndex();                                   \
    return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, a2, a3, a4, a5, name); \
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const


#define ILOCPCONSTRAINTWRAPPER6(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
class ILCCONCAT(_this, I) : public IloCPConstraintI {\
  ILOCPCONSTRAINTWRAPPERMEMBERS6DECL(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
};\
  ILOCPCONSTRAINTWRAPPERMEMBERS6(ILCCONCAT(_this, I), t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
    IloConstraint _this(IloEnv env, t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, const char* name=0) { \
    ILCCONCAT(_this, I)::InitTypeIndex();                                   \
    return new (env) ILCCONCAT(_this, I)(env.getImpl(), a1, a2, a3, a4, a5, a6, name); \
}\
IlcConstraint ILCCONCAT(_this, I)::extract(const IloCPEngine solver) const

//////////////////////////////////////////////////////////////////////////////
//
// ILOCPGOALWRAPPER
//
//////////////////////////////////////////////////////////////////////////////

#define ILOCPGOALWRAPPERRTTIDECL  ILCRTTIDECL
#define ILOCPGOALWRAPPERRTTI(A,B) ILCRTTI(A,B)


#define ILOCPGOALWRAPPER0(_this, solver) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)(IloEnvI* e) : IloGoalI(e) {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env) {\
  return new (env) ILCCONCAT(_this,ConcertI)(env.getImpl());\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const


#define ILOCPGOALWRAPPER1(_this, solver, t1, a1) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)(IloEnvI* e, t1 ILCCONCAT(a1,a1)) : IloGoalI(e), \
  a1(ILCCONCAT(a1, a1)) {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, t1 ILCCONCAT(a1, a1)) {\
  return new (env) ILCCONCAT(_this,ConcertI)(env.getImpl(), ILCCONCAT(a1, a1));\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const


#define ILOCPGOALWRAPPER2(_this, solver, t1, a1, t2, a2) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
  t2 a2;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1,t2);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)(IloEnvI* e, t1 ILCCONCAT(a1,a1), t2 ILCCONCAT(a2, a2)) : IloGoalI(e), \
  a1(ILCCONCAT(a1, a1)), a2(ILCCONCAT(a2, a2))  {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, t1 ILCCONCAT(a1, a1), t2 ILCCONCAT(a2, a2)) {\
  return new (env) ILCCONCAT(_this,ConcertI)(env.getImpl(), \
                                         ILCCONCAT(a1, a1), \
                                         ILCCONCAT(a2, a2));\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const


#define ILOCPGOALWRAPPER3(_this, solver, t1, a1, t2, a2, t3, a3) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1, t2, t3);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)( \
      IloEnvI* e, \
      t1 ILCCONCAT(a1, a1), \
      t2 ILCCONCAT(a2, a2), \
      t3 ILCCONCAT(a3, a3) \
  ) : IloGoalI(e), \
      a1(ILCCONCAT(a1, a1)), \
      a2(ILCCONCAT(a2, a2)), \
      a3(ILCCONCAT(a3, a3)) \
      {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, \
              t1 ILCCONCAT(a1, a1), \
              t2 ILCCONCAT(a2, a2), \
              t3 ILCCONCAT(a3, a3) \
              ) {\
  return new (env) ILCCONCAT(_this,ConcertI)( \
                         env.getImpl(), \
                         ILCCONCAT(a1, a1), \
                         ILCCONCAT(a2, a2), \
                         ILCCONCAT(a3, a3) \
             );\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const


#define ILOCPGOALWRAPPER4(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1, t2, t3, t4);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine ) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)( \
      IloEnvI* env, \
      t1 ILCCONCAT(a1, a1), \
      t2 ILCCONCAT(a2, a2), \
      t3 ILCCONCAT(a3, a3), \
      t4 ILCCONCAT(a4, a4) \
  ) : IloGoalI(env), \
      a1(ILCCONCAT(a1, a1)), \
      a2(ILCCONCAT(a2, a2)), \
      a3(ILCCONCAT(a3, a3)), \
      a4(ILCCONCAT(a4, a4)) \
      {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, \
              t1 ILCCONCAT(a1, a1), \
              t2 ILCCONCAT(a2, a2), \
              t3 ILCCONCAT(a3, a3), \
              t4 ILCCONCAT(a4, a4) \
              ) {\
  return new (env) ILCCONCAT(_this,ConcertI)( \
                         env.getImpl(), \
                         ILCCONCAT(a1, a1), \
                         ILCCONCAT(a2, a2), \
                         ILCCONCAT(a3, a3), \
                         ILCCONCAT(a4, a4) \
             );\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const


#define ILOCPGOALWRAPPER5(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
  t5 a5;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1, t2, t3, t4, t5);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)( \
      IloEnvI* env, \
      t1 ILCCONCAT(a1, a1), \
      t2 ILCCONCAT(a2, a2), \
      t3 ILCCONCAT(a3, a3), \
      t4 ILCCONCAT(a4, a4), \
      t5 ILCCONCAT(a5,a5)\
  ) : IloGoalI(env), \
      a1(ILCCONCAT(a1, a1)), \
      a2(ILCCONCAT(a2, a2)), \
      a3(ILCCONCAT(a3, a3)), \
      a4(ILCCONCAT(a4, a4)), \
      a5(ILCCONCAT(a5, a5)) \
      {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, \
              t1 ILCCONCAT(a1, a1), \
              t2 ILCCONCAT(a2, a2), \
              t3 ILCCONCAT(a3, a3), \
              t4 ILCCONCAT(a4, a4), \
              t5 ILCCONCAT(a5, a5) \
              ) {\
  return new (env) ILCCONCAT(_this,ConcertI)( \
                         env.getImpl(), \
                         ILCCONCAT(a1, a1), \
                         ILCCONCAT(a2, a2), \
                         ILCCONCAT(a3, a3), \
                         ILCCONCAT(a4, a4), \
                         ILCCONCAT(a5, a5) \
             );\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const

#define ILOCPGOALWRAPPER6(_this, solver, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) \
class ILCCONCAT(_this,ConcertI) : public IloGoalI { \
ILOCPGOALWRAPPERRTTIDECL \
  t1 a1;\
  t2 a2;\
  t3 a3;\
  t4 a4;\
  t5 a5;\
  t6 a6;\
public:\
  ILCCONCAT(_this,ConcertI)(IloEnvI*, t1, t2, t3, t4, t5, t6);\
  ~ILCCONCAT(_this,ConcertI)();\
  virtual IlcGoal extract(const IloCPEngine) const;\
  const char* getDisplayName() const;\
};\
ILOCPGOALWRAPPERRTTI(ILCCONCAT(_this,ConcertI), IloGoalI) \
const char* ILCCONCAT(_this,ConcertI)::getDisplayName() const\
{ return #_this; }\
ILCCONCAT(_this,ConcertI)::ILCCONCAT(_this,ConcertI)( \
      IloEnvI* env, \
      t1 ILCCONCAT(a1, a1), \
      t2 ILCCONCAT(a2, a2), \
      t3 ILCCONCAT(a3, a3), \
      t4 ILCCONCAT(a4, a4), \
      t5 ILCCONCAT(a5, a5), \
      t6 ILCCONCAT(a6, a6) \
  ) : IloGoalI(env), \
      a1(ILCCONCAT(a1, a1)), \
      a2(ILCCONCAT(a2, a2)), \
      a3(ILCCONCAT(a3, a3)), \
      a4(ILCCONCAT(a4, a4)), \
      a5(ILCCONCAT(a5, a5)), \
      a6(ILCCONCAT(a6, a6)) \
      {}\
ILCCONCAT(_this,ConcertI)::~ILCCONCAT(_this,ConcertI)() {}\
IloGoal _this(IloEnv env, \
              t1 ILCCONCAT(a1, a1), \
              t2 ILCCONCAT(a2, a2), \
              t3 ILCCONCAT(a3, a3), \
              t4 ILCCONCAT(a4, a4), \
              t5 ILCCONCAT(a5, a5), \
              t6 ILCCONCAT(a6, a6) \
              ) {\
  return new (env) ILCCONCAT(_this,ConcertI)( \
                         env.getImpl(), \
                         ILCCONCAT(a1, a1), \
                         ILCCONCAT(a2, a2), \
                         ILCCONCAT(a3, a3), \
                         ILCCONCAT(a4, a4), \
                         ILCCONCAT(a5, a5), \
                         ILCCONCAT(a6, a6) \
             );\
}\
IlcGoal ILCCONCAT(_this,ConcertI)::extract(const IloCPEngine solver) const



////////////////////////////////////////////////////////////////////////////
//
// BOOLEAN AND INTEGER EXPRESSIONS & VARIABLES
//
////////////////////////////////////////////////////////////////////////////


class IlcIntExp {
  ILOCPHIDDENHANDLEMINI(IlcIntExp,IlcIntExpI)
  ILCEXTENSIONMETHODSHDECL(IlcIntExp)
private:
  void _ctor(IlcConstraint bexp);

  IlcBool _isFixed() const;
  IlcInt  _getValue() const;
  IlcInt  _getSize() const;
  IlcInt  _getMin() const;
  IlcInt  _getMax() const;
  void    _setValue(IlcInt value) const;
  void    _setRange(IlcInt min, IlcInt max) const;
  void    _setMin(IlcInt min) const;
  void    _setMax(IlcInt max) const;
  void    _display(ILOSTD(ostream)& str) const;
public:

  IlcIntExp(IlcConstraint bexp) {
    IlcCPOAssert(bexp.getImpl() != 0, "IlcConstraint: empty handle");
    _ctor(bexp);
  }
  IlcBool isBound() const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    return _isFixed();
  }

  IlcBool isFixed() const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    return _isFixed();
  }

  IlcInt getValue() const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    IlcCPOAssert(_isFixed(), "IlcIntExp: not fixed");
    return _getValue();
  }

  IlcInt getSize () const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    return _getSize();
  }

  IlcInt getMin() const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    return _getMin();
  }

  IlcInt getMax() const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    return _getMax();
  }

  void setValue(IlcInt value) const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    _setValue(value);
  }

  void setRange(IlcInt min, IlcInt max) const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    _setRange(min, max);
  }

  void setMin(IlcInt min) const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    _setMin(min);
  }

  void setMax(IlcInt max) const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    _setMax(max);
  }
  void display(ILOSTD(ostream)& str) const {
    IlcCPOAssert(_impl != 0, "IlcIntExp: empty handle");
    _display(str);
  }
  ILCGETCPHDECL(IlcIntExp)
};


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntExp& exp);


class IlcIntVar : public IlcIntExp {
friend class IlcIntVarArrayIterator;
friend IlcIntExp IlcScalProd(const IlcIntVarArray vars,
                             const IlcConstIntArray coeffs);
friend IlcIntExp IlcScalProd(const IlcIntVarArray vars,
                             const IlcIntArray coeffs);
private:
  void _ctor(IlcCPEngine solver, IlcInt min, IlcInt max, const char * name);
  void _ctor(IlcCPEngine solver, const IlcIntArray values, const char * name);

  IlcBool _isInProcess() const;
  IlcInt  _getOldMin() const;
  IlcInt  _getOldMax() const;
  IlcInt  _getMinDelta() const;
  IlcInt  _getMaxDelta() const;
  IlcBool _isInDelta(IlcInt value) const;
  IlcBool _isInDomain(IlcInt value) const;
  IlcInt  _getNextHigher(IlcInt value) const;
  IlcInt  _getNextLower(IlcInt value) const;
  void    _removeValue(IlcInt value) const;
  void    _removeRange(IlcInt min, IlcInt max) const;
  void    _setDomain(IlcIntVar var) const;
  void    _setDomain(IlcIntSet set) const;
  void    _setDomain(IlcIntArray array) const;
  void    _removeDomain(IlcIntSet set) const;
  void    _removeDomain(IlcIntArray array) const;
  void    _whenValue(const IlcDemon demon) const;
  void    _whenRange(const IlcDemon demon) const;
  void    _whenDomain(const IlcDemon demon) const;
  IlcInt  _getSafeOldMin() const;
  IlcInt  _getSafeOldMax() const;

public:

  IlcIntVar() {}
  IlcIntVar(IlcIntExpI* exp);
  IlcIntVar(IlcIntVarI* exp) : IlcIntExp((IlcIntExpI*)exp) {}
  IlcIntVarI* getImpl() const { return (IlcIntVarI*)_impl; }
  
  IlcIntVar(IlcCPEngine solver, IlcInt min, IlcInt max, const char* name = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, min, max, name);
  }

  IlcIntVar(IlcCPEngine solver, const IlcIntArray values, const char* name = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(values.getImpl() != 0, "IlcIntArray: empty handle");
    _ctor(solver, values, name);
  }
#ifdef CPPREF_GENERATION

 IlcIntVar(IlcIntVarI* impl);
#endif
 
  IlcIntVar(const IlcIntExp exp);
  void operator=(const IlcIntExp& exp);
private:
  void operator = (IlcIntExpI* impl) { _impl = impl; }
public:

  IlcBool isInProcess() const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _isInProcess();
  }

  IlcInt getOldMin() const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getOldMin();
  }

  IlcInt getOldMax() const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getOldMax();
  }

  ILCDEPRECATED IlcInt getMinDelta () const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getMinDelta();
  }

  ILCDEPRECATED IlcInt getMaxDelta () const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getMaxDelta();
  }

  IlcBool isInDelta(IlcInt value) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _isInDelta(value);
  }

  IlcBool isInDomain (IlcInt value) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _isInDomain(value);
  }

  IlcInt getNextHigher (IlcInt threshold) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getNextHigher(threshold);
  }

  IlcInt getNextLower (IlcInt threshold) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getNextLower(threshold);
  }

  void removeValue(IlcInt value) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    _removeValue(value);
  }

  void removeRange(IlcInt min, IlcInt max) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    _removeRange(min, max);
  }

  void setDomain(IlcIntVar var) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IlcIntVar: empty handle");
    _setDomain(var);
  }

  void setDomain(IlcIntSet set) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(set.getImpl() != 0, "IlcIntSet: empty handle");
    _setDomain(set);
  }

  void setDomain(IlcIntArray array) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(array.getImpl() != 0, "IlcIntArray: empty handle");
    _setDomain(array);
  }

  void removeDomain(IlcIntArray array) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(array.getImpl() != 0, "IlcIntArray: empty handle");
    _removeDomain(array);
  }

  void removeDomain(IlcIntSet set) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(set.getImpl() != 0, "IlcIntSet: empty handle");
    _removeDomain(set);
  }
  // Deprecated
  ILCDEPRECATED void whenValue(const IlcGoal goal) const;
  ILCDEPRECATED void whenRange(const IlcGoal goal) const;
  ILCDEPRECATED void whenDomain(const IlcGoal goal) const;

  void whenValue(const IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(demon.getImpl() != 0, "IlcDemon: empty handle");
    _whenValue(demon);
  }

  void whenRange(const IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(demon.getImpl() != 0, "IlcDemon: empty handle");
    _whenRange(demon);
  }

  void whenDomain(const IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    IlcCPOAssert(demon.getImpl() != 0, "IlcDemon: empty handle");
    _whenDomain(demon);
  }
  IlcInt getSafeOldMin() const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getSafeOldMin();
  }
  IlcInt getSafeOldMax() const {
    IlcCPOAssert(_impl != 0, "IlcIntVar: empty handle");
    return _getSafeOldMax();
  }
};


class IlcIntVarDeltaIterator {
  IlcInt              _curr;
  IlcBool             _ok;
  void *              _delta;

  void _ctor(const IlcIntVar var);
  void _advance(IlcInt& curr);
public:

  IlcIntVarDeltaIterator(const IlcIntVar var) {
    IlcCPOAssert(var.getImpl() != 0, "IlcIntVar: empty handle");
    _ctor(var);
  }

  IlcIntVarDeltaIterator& operator++() {
    _advance(_curr);
    return *this;
  }

  IlcInt operator*() const { return _curr; }

  IlcBool ok() const { return _ok; }
};


class IlcIntExpIterator {
  void*     _var;
  IlcInt    _curr;
  IlcBool   _ok;

  void _next();
public:

  IlcIntExpIterator(IlcIntVar var) {
    IlcCPOAssert(var.getImpl() != 0, "IlcIntVar: empty handle");
    _var = var.getImpl();
    _curr = var.getMin();
    _ok = IloTrue;
  }

  IlcIntExpIterator& operator++() {
    IlcInt oldCurrent = _curr;
    _next();
    _ok = (_curr != oldCurrent);
    return *this;
  }

  IlcInt operator*() const { return _curr; }

  IlcBool ok()const { return _ok; }
};

typedef void * IlcIntVarArrayI;

class IlcIntVarArray {
private:
  ILOCPVISIBLEHANDLEMINI(IlcIntVarArray, IlcIntVarArrayI)
  ILCEXTENSIONMETHODSHDECL(IlcIntVarArray)

  void _ctor(IlcCPEngine solver, IlcInt size);
  void _ctor(IlcCPEngine solver, IlcInt size, IlcInt min, IlcInt max);
  void _ctor(IlcCPEngine solver, IlcInt size, const IlcIntVar prototype);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5, const IlcIntVar v6);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5, const IlcIntVar v6,
             const IlcIntVar v7);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5, const IlcIntVar v6,
             const IlcIntVar v7, const IlcIntVar v8);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5, const IlcIntVar v6,
             const IlcIntVar v7, const IlcIntVar v8,
             const IlcIntVar v9);
  void _ctor(IlcCPEngine solver, const IlcIntVar v1, const IlcIntVar v2,
             const IlcIntVar v3, const IlcIntVar v4,
             const IlcIntVar v5, const IlcIntVar v6,
             const IlcIntVar v7, const IlcIntVar v8,
             const IlcIntVar v9, const IlcIntVar v10);
  IlcIntVar& _get(IlcInt index) const { return ((IlcIntVar*)_impl)[index]; }
  IlcIntVarArray _getCopy() const;
  IlcInt    _getSize() const { return *((IlcInt*)_impl - 1); }
  IlcIntExp _element(const IlcIntExp) const;
  IlcInt    _getMinMin(IlcInt l, IlcInt r) const;
  IlcInt    _getMinMax(IlcInt l, IlcInt r) const;
  IlcInt    _getMaxMin(IlcInt l, IlcInt r) const;
  IlcInt    _getMaxMax(IlcInt l, IlcInt r) const;
  void      _display(ILOSTD(ostream)& str) const;
  IlcBool   _equals(IlcIntVarArray a) const;
public:
  ILCGETCPHDECL(IlcIntVarArray)

  IlcIntVarArray(IlcCPEngine solver, IlcInt size) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntVarArray::IlcIntVarArray - "
                            "size must be positive");
    _ctor(solver, size);
  }
#ifdef CPPREF_GENERATION
 
  IlcIntVarArray(IlcCPEngine solver, IlcInt size, const IlcIntVar exp ...);
#endif
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 1, "IlcIntVarArray::IlcIntVarArray - size must be 1");
    _ctor(solver, v1);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 2, "IlcIntVarArray::IlcIntVarArray - size must be 2");
    _ctor(solver, v1, v2);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 3, "IlcIntVarArray::IlcIntVarArray - size must be 3");
    _ctor(solver, v1, v2, v3);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 4, "IlcIntVarArray::IlcIntVarArray - size must be 4");
    _ctor(solver, v1, v2, v3, v4);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 5, "IlcIntVarArray::IlcIntVarArray - size must be 5");
    _ctor(solver, v1, v2, v3, v4, v5);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5, const IlcIntVar v6) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 6, "IlcIntVarArray::IlcIntVarArray - size must be 6");
    _ctor(solver, v1, v2, v3, v4, v5, v6);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5, const IlcIntVar v6,
                 const IlcIntVar v7) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 7, "IlcIntVarArray::IlcIntVarArray - size must be 7");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5, const IlcIntVar v6,
                 const IlcIntVar v7, const IlcIntVar v8) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 8, "IlcIntVarArray::IlcIntVarArray - size must be 8");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5, const IlcIntVar v6,
                 const IlcIntVar v7, const IlcIntVar v8,
                 const IlcIntVar v9)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 9, "IlcIntVarArray::IlcIntVarArray - size must be 9");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8, v9);
  }
  IlcIntVarArray(IlcCPEngine solver, IlcInt ILCPARAM(size),
                 const IlcIntVar v1, const IlcIntVar v2,
                 const IlcIntVar v3, const IlcIntVar v4,
                 const IlcIntVar v5, const IlcIntVar v6,
                 const IlcIntVar v7, const IlcIntVar v8,
                 const IlcIntVar v9, const IlcIntVar v10)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 10, "IlcIntVarArray::IlcIntVarArray - size must be 10");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
  }


  IlcIntVarArray(IlcCPEngine solver, IlcInt size, IlcInt min, IlcInt max) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntVarArray::IlcIntVarArray - "
                         "size must be positive");
    _ctor(solver, size, min, max);
  }

  IlcIntVarArray getCopy() const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    return _getCopy();
  }

  IlcIntVar& operator[] (IlcInt index) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < getSize(),
              "IlcIntVarArray: index out of range");
    return _get(index);
  }

  IlcIntExp operator[] (const IlcIntExp exp) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(exp.getImpl() != 0, "IlcIntExp: empty handle");
    return _element(exp);
  }

  IlcInt getSize() const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    return _getSize();
  }

  IlcInt getMinMin(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(indexMin < indexMax,
              "IlcIntVarArray: indexMin must be less than indexMax");
    return _getMinMin(indexMin, indexMax);
  }

  IlcInt getMinMax(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(indexMin < indexMax,
              "IlcIntVarArray: indexMin must be less than indexMax");
    return _getMinMax(indexMin, indexMax);
  }

  IlcInt getMaxMin(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(indexMin < indexMax,
              "IlcIntVarArray: indexMin must be less than indexMax");
    return _getMaxMin(indexMin, indexMax);
  }

  IlcInt getMaxMax(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcIntVarArray: empty handle");
    IlcCPOAssert(indexMin < indexMax,
              "IlcIntVarArray: indexMin must be less than indexMax");
    return _getMaxMax(indexMin, indexMax);
  }

  IlcInt getMinMin() const { return getMinMin(0, getSize()); }

  IlcInt getMinMax() const { return getMinMax(0, getSize()); }

  IlcInt getMaxMin() const { return getMaxMin(0, getSize()); }

  IlcInt getMaxMax() const { return getMaxMax(0, getSize()); }
  IlcIntVarI ** getArray() const { return (IlcIntVarI**)_impl; }
};


ILCSTD(ostream)& operator<< (ILCSTD(ostream)& str, const IlcIntVarArray& exp);

////////////////////////////////////////////////////////////////////////////
//
// FLOATING POINT VARIABLE
//
////////////////////////////////////////////////////////////////////////////


class IlcFloatExp {
  ILOCPHIDDENHANDLEMINI(IlcFloatExp,IlcFloatExpI)
  ILCEXTENSIONMETHODSHDECL(IlcFloatExp)
  friend class IlcFloatVar;

  void _ctor(IlcIntExp exp);

  void     _createEventHandler() const;
  void     _display(ILOSTD(ostream)& str) const;
  IlcBool  _isFixed() const;
  IlcBool  _isInDomain(IlcFloat value) const;
  IlcFloat _getValue() const;
  IlcFloat _getSize() const;
  IlcFloat _getMin() const;
  IlcFloat _getMax() const;
  IlcFloat _getPrecision() const;
  void     _setPrecision(IloNum precision) const;
  void     _setValue(IloNum value) const;
  void     _setRange(IloNum min, IloNum max) const;
  void     _setMin(IloNum min) const;
  void     _setMax(IloNum max) const;
protected:
  void createEventHandler() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _createEventHandler();
  }
public:

  IlcFloatExp(IlcIntExp exp) {
    IlcCPOAssert(exp.getImpl() != 0, "IlcIntExp: empty handle");
    _ctor(exp);
  }

  void display(ILOSTD(ostream) &str) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _display(str);
  }
  IlcBool isBound() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _isFixed();
  }

  IlcBool isFixed() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _isFixed();
  }

  IlcBool isInDomain(IlcFloat value) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _isInDomain(value);
  }
  IlcFloat getValue() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _getValue();
  }

  IlcFloat getSize() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _getSize();
  }

  IlcFloat getMin() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _getMin();
  }

  IlcFloat getMax() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _getMax();
  }

  IlcFloat getPrecision() const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    return _getPrecision();
  }

  void setPrecision(IlcFloat p) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    IlcCPOAssert(p >= 0, "IlcFloatExp::setPrecision, precision must be >= 0");
    _setPrecision(p);
  }
  void setValue(IlcFloat value) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _setValue(value);
  }

  void setRange(IlcFloat min, IlcFloat max) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _setRange(min, max);
  }

  void setMin(IlcFloat min) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _setMin(min);
  }

  void setMax(IlcFloat max) const {
    IlcCPOAssert(_impl != 0, "IlcFloatExp: empty handle");
    _setMax(max);
  }
  ILCGETCPHDECL(IlcFloatExp)
};



ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcFloatExp& exp);

//------------------------------------------------------------
// IlcFloatVar
//------------------------------------------------------------


class IlcFloatVar : public IlcFloatExp {
  friend class IlcCPEngineI;
  friend class IlcFloatExpI;
  friend class IlcFloatVarArray;
  friend class IlcFloatVarArrayIterator;
  friend IlcFloatExp IlcScalProd(const IlcFloatVarArray,const IlcFloatArray);
  friend IlcFloatExp IlcScalProd(const IlcIntVarArray,const IlcFloatArray);
  friend class IlcGetVarVisitor;

  void _ctor(IlcCPEngine solver, IlcFloat min, IlcFloat max, const char * name);
  IlcFloat _getVarMin() const;
  IlcFloat _getVarMax() const;
  IlcFloat _getOldMin() const;
  IlcFloat _getOldMax() const;
  IlcFloat _getMinDelta() const;
  IlcFloat _getMaxDelta() const;
  IlcBool  _isInDelta(IlcFloat value) const;
  IlcBool  _isInProcess() const;
  IlcFloat _getSafeOldMin() const;
  IlcFloat _getSafeOldMax() const;
  void     _whenRange(const IlcDemon demon) const;
  void     _whenValue(const IlcDemon demon) const;
public:

  IlcFloatVar(){}

  IlcFloatVar(IlcCPEngine solver,
              IlcFloat min,
              IlcFloat max,
              const char * name = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, min, max, name);
  }

  IlcFloatVar(IlcCPEngine solver,
              IlcFloat min,
              IlcFloat max,
              IlcFloat precision,
              const char* name = 0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, min, max, name);
    setPrecision(precision);
  }
#ifdef CPPREF_GENERATION

 IlcFloatVar(IlcFloatVarI* impl);

 IlcFloatVar(IlcIntVar var);
#endif
  IlcFloatVar(IlcIntExp exp);

  IlcFloatVar(const IlcFloatExp exp);

  void operator=(IlcFloatExp exp);
private:
  IlcFloatVar(IlcFloatExpI* exp) : IlcFloatExp(exp) { }
  void operator = (IlcFloatExpI* impl) { _impl = impl; }
public:

  IlcFloat getMin() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getVarMin();
  }

  IlcFloat getMax() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getVarMax();
  }

  IlcBool isInProcess() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _isInProcess();
  }

  IlcFloat getOldMin() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getOldMin();
  }

  IlcFloat getOldMax() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getOldMax();
  }
  IlcFloat getSafeOldMin() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getSafeOldMin();
  }
  IlcFloat getSafeOldMax() const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getSafeOldMax();
  }


  ILCDEPRECATED IlcFloat getMinDelta () const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getVarMin() - _getOldMin();
  }

  ILCDEPRECATED IlcFloat getMaxDelta () const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _getVarMax() - _getOldMax();
  }

  IlcBool isInDelta(IlcFloat value) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    return _isInDelta(value);
  }

  void whenRange(const IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    _whenRange(demon);
  }

  void whenValue(const IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVar: empty handle");
    _whenValue(demon);
  }
};


class IlcFloatVarArray {
  ILOCPHIDDENHANDLEMINI(IlcFloatVarArray,IlcFloatVarArrayI)
  ILCEXTENSIONMETHODSHDECL(IlcFloatVarArray)
  ILCGETCPHDECL(IlcFloatVarArray)

  void _ctor(IlcCPEngine solver, IlcInt size);
  void _ctor(IlcCPEngine solver, IlcInt size, IlcFloat min, IlcFloat max);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5, const IlcFloatVar v6);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5, const IlcFloatVar v6,
             const IlcFloatVar v7);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5, const IlcFloatVar v6,
             const IlcFloatVar v7, const IlcFloatVar v8);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5, const IlcFloatVar v6,
             const IlcFloatVar v7, const IlcFloatVar v8,
             const IlcFloatVar v9);
  void _ctor(IlcCPEngine solver, const IlcFloatVar v1, const IlcFloatVar v2,
             const IlcFloatVar v3, const IlcFloatVar v4,
             const IlcFloatVar v5, const IlcFloatVar v6,
             const IlcFloatVar v7, const IlcFloatVar v8,
             const IlcFloatVar v9, const IlcFloatVar v10);
  IlcFloatVar& _get(IlcInt index) const;
  IlcFloatVarArray _getCopy() const;
  IlcInt    _getSize() const;
  IlcFloat  _getMinMin(IlcInt l, IlcInt r) const;
  IlcFloat  _getMinMax(IlcInt l, IlcInt r) const;
  IlcFloat  _getMaxMin(IlcInt l, IlcInt r) const;
  IlcFloat  _getMaxMax(IlcInt l, IlcInt r) const;
public:

  IlcFloatVarArray(IlcCPEngine solver, IlcInt size) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcFloatVarArray::IlcFloatVarArray - "
                         "size must be positive");
    _ctor(solver, size);
  }
 
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 1, "IlcFloatVarArray::IlcFloatVarArray - size must be 1");
    _ctor(solver, v1);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 2, "IlcFloatVarArray::IlcFloatVarArray - size must be 2");
    _ctor(solver, v1, v2);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 3, "IlcFloatVarArray::IlcFloatVarArray - size must be 3");
    _ctor(solver, v1, v2, v3);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 4, "IlcFloatVarArray::IlcFloatVarArray - size must be 4");
    _ctor(solver, v1, v2, v3, v4);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 5, "IlcFloatVarArray::IlcFloatVarArray - size must be 5");
    _ctor(solver, v1, v2, v3, v4, v5);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5, const IlcFloatVar v6)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 6, "IlcFloatVarArray::IlcFloatVarArray - size must be 6");
    _ctor(solver, v1, v2, v3, v4, v5, v6);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5, const IlcFloatVar v6,
                    const IlcFloatVar v7)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 7, "IlcFloatVarArray::IlcFloatVarArray - size must be 7");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5, const IlcFloatVar v6,
                    const IlcFloatVar v7, const IlcFloatVar v8)  {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 8, "IlcFloatVarArray::IlcFloatVarArray - size must be 8");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5, const IlcFloatVar v6,
                    const IlcFloatVar v7, const IlcFloatVar v8,
                    const IlcFloatVar v9) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 9, "IlcFloatVarArray::IlcFloatVarArray - size must be 9");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8, v9);
  }
  IlcFloatVarArray (IlcCPEngine solver, IlcInt ILCPARAM(size),
                    const IlcFloatVar v1, const IlcFloatVar v2,
                    const IlcFloatVar v3, const IlcFloatVar v4,
                    const IlcFloatVar v5, const IlcFloatVar v6,
                    const IlcFloatVar v7, const IlcFloatVar v8,
                    const IlcFloatVar v9, const IlcFloatVar v10) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size == 10, "IlcFloatVarArray::IlcFloatVarArray - size must be 10");
    _ctor(solver, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
  }

  IlcFloatVarArray(IlcCPEngine solver, IlcInt size, IlcFloat min, IlcFloat max) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(size >= 0, "IlcFloatVarArray::IlcFloatVarArray - "
                         "size must be positive");
    _ctor(solver, size, min, max);
  }

  IlcFloatVarArray getCopy() const {
    return _getCopy();
  }

#if !defined(ILOINTASINT)
  IlcFloatVar& operator[] (IlcInt index) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < getSize(),
              "IlcFloatVarArray: index out of range");
    return _get(index);
  }
#else
  IlcFloatVar& operator[] (int index) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < getSize(),
              "IlcFloatVarArray: index out of range");
    return _get((IlcInt)index);
  }
#endif

  IlcInt getSize() const {
    return (_impl == 0) ? 0 : _getSize();
  }
public:

  IlcFloat getMinMin(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    return _getMinMin(indexMin, indexMax);
  }

  IlcFloat getMinMax(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    return _getMinMax(indexMin, indexMax);
  }

  IlcFloat getMaxMin(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    return _getMaxMin(indexMin, indexMax);
  }

  IlcFloat getMaxMax(IlcInt indexMin, IlcInt indexMax) const {
    IlcCPOAssert(_impl != 0, "IlcFloatVarArray: empty handle");
    return _getMaxMax(indexMin, indexMax);
  }

  IlcFloat getMinMin() const { return getMinMin(0, getSize()); }

  IlcFloat getMinMax() const { return getMinMax(0, getSize()); }

  IlcFloat getMaxMin() const { return getMaxMin(0, getSize()); }

  IlcFloat getMaxMax() const { return getMaxMax(0, getSize()); }
};


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcFloatVarArray& exp);

////////////////////////////////////////////////////////////////////////////
//
// INTEGER SET VARIABLE
//
////////////////////////////////////////////////////////////////////////////


class IlcIntSetVar {
  ILOCPHIDDENHANDLEMINI(IlcIntSetVar,IlcIntSetVarI)
  ILCEXTENSIONMETHODSHDECL(IlcIntSetVar)

  void     _ctor(IlcCPEngine solver, IlcInt min, IlcInt max, const char * name);
  void     _ctor(IlcCPEngine solver, const IlcIntArray domain, const char * name);
  void     _ctor(IlcCPEngine solver, const IlcIntSet domain, const char * name);

  void      _display(ILOSTD(ostream)& str) const;
  IlcInt    _chooseValue() const;
  IlcBool   _isFixed() const;
  IlcInt    _getSize() const;
  IlcIntSet _getValue() const;
  IlcIntSet _getPossibleSet() const;
  IlcIntSet _getRequiredSet() const;
  IlcInt    _getRequiredSize() const;
  IlcInt    _getPossibleSize() const;
  IlcIntVar _getCardinality() const;
  IlcInt    _isRequired(IlcInt value) const;
  IlcInt    _isPossible(IlcInt value) const;
  IlcBool   _isInDomain(IlcIntSet value) const;
  void      _addRequired(IlcInt value) const;
  void      _removePossible(IlcInt value) const;
  void      _setDomain(IlcIntSetVar var) const;
  void      _setDomain(IlcIntVar var) const;
  void      _whenValue(IlcDemon demon) const;
  void      _whenDomain(IlcDemon demon) const;
  IlcBool   _isInProcess() const;
public:

  IlcIntSetVar(IlcCPEngine solver, IlcInt min, IlcInt max, const char* name=0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(min <= max, "IlcIntSetVar: min > max");
    _ctor(solver, min, max, name);
  }

  IlcIntSetVar(IlcCPEngine solver, const IlcIntArray array, const char* name=0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(array.getImpl(), "IlcIntArray: empty handle");
    _ctor(solver, array, name);
  }
  IlcIntSetVar(IlcCPEngine solver, const IlcIntSet set, const char* name=0) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(set.getImpl(), "IlcIntSet: empty handle");
    _ctor(solver, set, name);
  }

  void display(ILOSTD(ostream) &str) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    _display(str);
  }
  IlcInt chooseValue() const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _chooseValue();
  }
  IlcBool isBound() const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _isFixed();
  }

  IlcBool isFixed() const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _isFixed();
  }

  IlcInt getSize () const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getSize();
  }

  IlcIntSet getValue()const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getValue();
  }

  IlcIntSet getPossibleSet()const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getPossibleSet();
  }

  IlcIntSet getRequiredSet()const{
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getRequiredSet();
  }
  IlcInt getRequiredSize() const{
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getRequiredSize();
  }
  IlcInt getPossibleSize() const{
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getPossibleSize();
  }
  IlcIntVar getCardinality() const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _getCardinality();
  }

  IlcBool isRequired(IlcInt elt) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _isRequired(elt);
  }

  IlcBool isPossible(IlcInt elt) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _isPossible(elt);
  }

  IlcBool isInDomain(IlcIntSet set) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    IlcCPOAssert(set.getImpl() != 0, "IlcIntSet: empty handle");
    return _isInDomain(set);
  }

  void addRequired(IlcInt elt)const{
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    _addRequired(elt);
  }

  void removePossible(IlcInt elt)const{
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    _removePossible(elt);
  }

  void setDomain(IlcIntSetVar var) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    IlcCPOAssert(var.getImpl() != 0, "IlcIntSetVar: empty handle");
    _setDomain(var);
  }

  void whenValue(IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    IlcCPOAssert(demon.getImpl() != 0, "IlcDemon: empty handle");
    _whenValue(demon);
  }

  void whenDomain(IlcDemon demon) const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    IlcCPOAssert(demon.getImpl() != 0, "IlcDemon: empty handle");
    _whenDomain(demon);
  }

  IlcBool isInProcess() const {
    IlcCPOAssert(_impl != 0, "IlcIntSetVar: empty handle");
    return _isInProcess();
  }
  ILCGETCPHDECL(IlcIntSetVar)
};



inline ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str,
                                    const IlcIntSetVar& exp) {
  exp.display(str);
  return str;
}

typedef enum {
  IlcPossibleIteration,
  IlcRequiredIteration
} IlcDomainIteration;

class IlcIntSetVarIterator {
private:
  IlcIntSetIterator _it;
public:
  IlcIntSetVarIterator(IlcIntSetVar var, IlcDomainIteration mode);
  ~IlcIntSetVarIterator() {}
  IlcBool ok() const {
    return _it.ok();
  }
  IlcInt operator*() const {
    return *_it;
  }
  IlcIntSetVarIterator& operator++() {
    ++_it;
    return *this;
  }
};

class IlcIntSetVarDeltaIterator {
private:
  void*          _var;
  void*          _cell;
  IlcInt         _curr;
  IlcBool        _ok;
  void*          _intervalCell;
  void*          _set;
  IlcBool        _startNewInterval;
  IlcBool next(IlcInt& val);

  IlcIntSetVarI* getVar() const { return (IlcIntSetVarI*)_var; }
  IlcCPOIntCell* getCell() const { return (IlcCPOIntCell*)_cell; }
  void setCell(IlcCPOIntCell* c) { _cell = c; }
  IlcCPOIntCell* getIntervalCell() const { return (IlcCPOIntCell*)_intervalCell; }
  void setIntervalCell(IlcCPOIntCell* ic) { _intervalCell = ic; }
  IlcIntSetI* getSet() const { return (IlcIntSetI*)_set; }

public:
  IlcIntSetVarDeltaIterator(IlcIntSetVar var, IlcDomainIteration mode);
  IlcBool ok() const {return _ok;}
  IlcInt operator*() const {return _curr;}
  IlcIntSetVarDeltaIterator& operator++();
};

typedef void * IlcIntSetArrayI;
typedef void * IlcIntSetVarArrayI;


class IlcIntSetArray {
ILOCPCOMMONARRAYDECL(IlcIntSetArray, IlcIntSet, IlcIntSetI*)
public:

  IlcIntSetArray(IlcCPEngine solver, IlcInt size, IlcIntArray array);
  friend ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcIntSetArray& a);
};

ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcIntSetArray& a);


class IlcConstraintArray {
  ILOCPCOMMONARRAYDECL(IlcConstraintArray, IlcConstraint, IlcConstraintI*)
  friend ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcConstraintArray& a);
};

ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcConstraintArray& a);


class IlcIntSetVarArray {
  ILOCPCOMMONARRAYDECL(IlcIntSetVarArray, IlcIntSetVar, IlcIntSetVarI*)
  friend ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcIntSetVarArray& a);
private:
  void _ctor(IlcCPEngine solver, IlcInt size, IlcInt min, IlcInt max);
  void _ctor(IlcCPEngine solver, IlcInt size, IlcIntArray values);
public:

  IlcIntSetVarArray(IlcCPEngine solver, IlcInt size, IlcInt min, IlcInt max) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine:: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntSetVarArray::IlcIntSetVarArray - "
                         "size must be non-negative");
    IlcCPOAssert(min <= max, "IlcIntSetVarArray::IlcIntSetVarArray - "
                          "minimum value cannot be greater than the maximum value");
    _ctor(solver, size, min, max);
  }

  IlcIntSetVarArray(IlcCPEngine solver, IlcInt size, IlcIntArray array) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine:: empty handle");
    IlcCPOAssert(size >= 0, "IlcIntSetVarArray::IlcIntSetVarArray - "
                         "size must be non-negative");
    IlcCPOAssert(array.getImpl() != 0, "IlcIntArray: empty handle");
    _ctor(solver, size, array);
  }
};

ILOSTD(ostream) & operator << (ILOSTD(ostream) & out, const IlcIntSetVarArray& a);

//
// Expressions and constraints
//


IlcIntExp operator+(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp operator+(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp operator+(const IlcIntExp exp1, const IlcIntExp exp2);

IlcIntExp operator-(const IlcIntExp exp1, const IlcIntExp exp2);

IlcIntExp operator-(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp operator-(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp operator-(const IlcIntExp exp);

IlcIntExp operator*(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp operator*(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp operator*(const IlcIntExp exp1, const IlcIntExp exp2);

IlcIntExp operator/(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp operator/(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp operator/(const IlcIntExp exp1, const IlcIntExp exp2);

#if !defined(ILOINTASINT)
inline IlcIntExp operator+(const IlcIntExp var, int offset){
  return var +(IlcInt)offset;
}
inline IlcIntExp operator+(int offset, const IlcIntExp var){
  return (IlcInt)offset + var;
}

inline IlcIntExp operator-(const IlcIntExp var, int offset){
  return var - (IlcInt)offset;
}
inline IlcIntExp operator-(int offset, const IlcIntExp var){
  return (IlcInt)offset - var;
}

inline IlcIntExp operator*(const IlcIntExp var, int offset){
  return var * (IlcInt)offset;
}
inline IlcIntExp operator*(int offset, const IlcIntExp var){
  return (IlcInt)offset * var;
}

inline IlcIntExp operator/(const IlcIntExp var, int offset){
  return var / (IlcInt)offset;
}
inline IlcIntExp operator/(int offset, const IlcIntExp var){
  return (IlcInt)offset / var;
}
#endif


IlcIntExp IlcSquare(const IlcIntExp exp);

IlcIntExp IlcSum(const IlcIntVarArray array);

IlcIntExp IlcScalProd(const IlcIntVarArray array1, const IlcIntVarArray array2);

IlcIntExp IlcScalProd(const IlcIntVarArray array1, const IlcIntArray array2);

IlcIntExp IlcScalProd(const IlcIntArray array1, const IlcIntVarArray array2);


IlcConstraint IlcCardIntEqCst(const IlcIntExp var, IlcInt val, const IlcIntVarArray vars);

IlcConstraint IlcCardIntEqCst(const IlcIntExp var, IlcInt val, const IlcIntArray vars);

IlcConstraint IlcCardIntEqCst(IlcInt var, IlcInt val, const IlcIntVarArray vars);


IlcConstraint
IlcDistribute(IlcIntVarArray cards,
              IlcIntArray values,
              IlcIntVarArray vars,
              IlcFilterLevel level);


IlcConstraint
IlcDistribute(IlcIntVarArray cards,
              IlcIntVarArray vars,
              IlcFilterLevel level);


IlcConstraint
IlcDistribute(IlcIntVarArray cards,
              IlcIntArray values,
              IlcIntVarArray vars);


IlcConstraint
IlcDistribute(IlcIntVarArray cards,
              IlcIntVarArray vars);


IlcConstraint IlcAllDiff(const IlcIntVarArray array, IlcFilterLevel level);


IlcConstraint IlcAllDiff(const IlcIntVarArray array);


IlcIntExp IlcCountDifferent(const IlcIntVarArray array, IlcFilterLevel level);


IlcIntExp IlcCountDifferent(const IlcIntVarArray array);


IlcConstraint IlcEqAbstraction(IlcIntVarArray ys, IlcIntVarArray xs,
                               IlcIntArray vals, IlcInt abstractValue);
IlcConstraint IlcEqIntAbstraction(IlcIntVarArray ys, IlcIntVarArray xs,
                                  IlcIntArray val);



IlcConstraint IlcInverse(IlcIntVarArray f, IlcIntVarArray invf);




IlcConstraint IlcSequence(IlcInt nbMin,
                          IlcInt nbMax,
                          IlcInt seqWidth,
                          IlcIntVarArray vars,
                          IlcIntArray values,
                          IlcIntVarArray cards,
                          IlcFilterLevel level);


IlcConstraint IlcSequence(IlcInt nbMin,
                          IlcInt nbMax,
                          IlcInt seqWidth,
                          IlcIntVarArray vars,
                          IlcIntArray values,
                          IlcIntVarArray cards);

IlcConstraint IlcMinDistance(IlcIntExp x, IlcIntExp y,IlcInt k);


IlcConstraint IlcAllMinDistance(IlcIntVarArray vars, IlcInt k);


IlcConstraint IlcAllMinDistance(IlcIntVarArray vars, IlcInt k,
                             IlcFilterLevel level);


IlcConstraint operator == (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator == (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator == (IlcInt exp1, const IlcIntExp exp2);

#if !defined(ILOINTASINT)
inline IlcConstraint operator == (const IlcIntExp exp, int cst) {
  return exp == (IlcInt)cst;
}
inline IlcConstraint operator == (int cst, const IlcIntExp exp) {
  return (IlcInt)cst == exp;
}
#endif


IlcConstraint operator != (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator != (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator != (IlcInt exp1, const IlcIntExp exp2);

#if !defined(ILOINTASINT)
inline IlcConstraint operator != (const IlcIntExp exp, int cst) {
  return exp != (IlcInt)cst;
}

inline IlcConstraint operator != (int cst, const IlcIntExp exp) {
  return (IlcInt)cst != exp;
}
#endif



IlcConstraint IlcLeOffset(const IlcIntExp x, const IlcIntExp y, IlcInt c);


IlcConstraint operator <= (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator <= (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator <= (IlcInt exp1, const IlcIntExp exp2);

#if !defined(ILOINTASINT)
inline IlcConstraint operator <= (const IlcIntExp exp, int cst) {
  return exp <= (IlcInt)cst;
}

inline IlcConstraint operator <= (int cst, const IlcIntExp exp) {
  return (IlcInt)cst <= exp;
}
#endif


IlcConstraint operator >= (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator >= (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator >= (IlcInt exp1, const IlcIntExp exp2);

#if !defined(ILOINTASINT)
inline IlcConstraint operator >= (const IlcIntExp exp, int cst) {
  return exp >= (IlcInt)cst;
}

inline IlcConstraint operator >= (int cst, const IlcIntExp exp) {
  return (IlcInt)cst >= exp;
}
#endif


IlcConstraint operator < (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator < (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator < (IlcInt exp1, const IlcIntExp exp2);


IlcConstraint operator > (const IlcIntExp exp1, const IlcIntExp exp2);

IlcConstraint operator > (const IlcIntExp exp1, IlcInt exp2);

IlcConstraint operator > (IlcInt exp1, const IlcIntExp exp2);


IlcIntExp IlcAbs(const IlcIntExp exp);

inline IlcInt IlcAbs(IlcInt exp){
  return exp >= 0 ? exp : -exp;
}
#ifndef ILO64
inline IlcBigInt IlcAbs(IlcBigInt exp) {
  return exp >= 0 ? exp : -exp;
}
#endif


IlcIntExp IlcMax(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp IlcMax(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp IlcMax(const IlcIntExp exp1, const IlcIntExp exp2);

IlcIntExp IlcMax(const IlcIntVarArray array);

IlcInt    IlcMax(const IlcIntArray array);

inline IlcInt IlcMax(IlcInt exp1, IlcInt exp2){
  return exp1 > exp2 ? exp1 : exp2;
}

#ifndef ILO64
inline IlcBigInt IlcMax(const IlcBigInt exp1, const IlcBigInt exp2) {
  return exp1 > exp2 ? exp1 : exp2;
}
#endif


IlcIntExp IlcMin(const IlcIntExp exp1, IlcInt exp2);

IlcIntExp IlcMin(IlcInt exp1, const IlcIntExp exp2);

IlcIntExp IlcMin(const IlcIntExp exp1, const IlcIntExp exp2);

IlcIntExp IlcMin(const IlcIntVarArray array);

IlcInt    IlcMin(const IlcIntArray array);

inline IlcInt IlcMin(const IlcInt exp1, const IlcInt exp2){
  return exp1 < exp2 ? exp1 : exp2;
}

#ifndef ILO64
inline IlcBigInt IlcMin(const IlcBigInt exp1, const IlcBigInt exp2){
  return exp1 < exp2 ? exp1 : exp2;
}
#endif

////////////////////////////////////////////////////////////////////////////
// Floating point constraints and expressions
////////////////////////////////////////////////////////////////////////////


IlcFloatExp IlcPiecewiseLinear(IlcFloatVar x,
                               IlcFloatArray point,
                               IlcFloatArray slope,
                               IlcFloat a,
                               IlcFloat fa);



IlcConstraint operator == (const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcConstraint operator == (const IlcFloatExp exp1, IlcFloat exp2);

IlcConstraint operator == (IlcFloat exp1, const IlcFloatExp exp2);

IlcConstraint operator <= (const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcConstraint operator <= (const IlcFloatExp exp1, IlcFloat exp2);

IlcConstraint operator <= (IlcFloat exp1, const IlcFloatExp exp2);

IlcConstraint operator >= (const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcConstraint operator >= (const IlcFloatExp exp1, IlcFloat exp2);

IlcConstraint operator >= (IlcFloat exp1, const IlcFloatExp exp2);

IlcConstraint operator == (const IlcIntExp, IlcFloat);
IlcConstraint operator == (IlcFloat, const IlcIntExp);
IlcConstraint operator <= (const IlcIntExp, IlcFloat);
IlcConstraint operator <= (IlcFloat, const IlcIntExp);
IlcConstraint operator >= (const IlcIntExp, IlcFloat);
IlcConstraint operator >= (IlcFloat, const IlcIntExp);


IlcFloatExp operator+(const IlcFloatExp exp1, IlcFloat exp2);

IlcFloatExp operator+(IlcFloat exp1, const IlcFloatExp exp2);

IlcFloatExp operator+(const IlcFloatExp exp1, const IlcFloatExp exp2);


IlcFloatExp operator-(const IlcFloatExp exp1, IlcFloat exp2);

IlcFloatExp operator-(IlcFloat exp1, const IlcFloatExp exp2);

IlcFloatExp operator-(const IlcFloatExp exp);

IlcFloatExp operator-(const IlcFloatExp exp1, const IlcFloatExp exp2);



IlcFloatExp operator*(const IlcFloatExp exp1, IlcFloat exp2);

IlcFloatExp operator*(IlcFloat exp1, const IlcFloatExp exp2);
IlcFloatExp IlcDivide(const IlcFloatExp var, IlcFloat offset);

inline IlcFloatExp operator/(const IlcFloatExp exp1, IlcFloat exp2) {
  return IlcDivide(exp1,exp2);
}

// mixed operators
IlcFloatExp operator+(IlcIntExp x, IlcFloatExp y);
IlcFloatExp operator+(IlcFloatExp x, IlcIntExp y);
IlcFloatExp operator+(IlcIntExp x, IlcFloat y);
IlcFloatExp operator+(IlcFloat x, IlcIntExp y);

IlcFloatExp operator-(IlcIntExp x, IlcFloatExp y);
IlcFloatExp operator-(IlcFloatExp x, IlcIntExp y);
IlcFloatExp operator-(IlcIntExp x, IlcFloat y);
IlcFloatExp operator-(IlcFloat x, IlcIntExp y);

IlcFloatExp operator*(IlcIntExp x, IlcFloat y);
IlcFloatExp operator*(IlcFloat x, IlcIntExp y);

IlcFloatExp operator/(IlcIntExp x, IlcFloat y);

IlcConstraint operator==(IlcIntExp x, IlcFloatExp y);
IlcConstraint operator==(IlcFloatExp x, IlcIntExp y);

IlcConstraint operator>=(IlcIntExp x, IlcFloatExp y);
IlcConstraint operator>=(IlcFloatExp x, IlcIntExp y);

IlcConstraint operator<=(IlcIntExp x, IlcFloatExp y);
IlcConstraint operator<=(IlcFloatExp x, IlcIntExp y);


IlcFloatExp IlcSum(const IlcFloatVarArray vars);

IlcFloat    IlcSum(const IlcFloatArray array);


IlcFloatExp IlcScalProd(const IlcFloatVarArray array1,
                        const IlcFloatArray array2);


IlcFloatExp IlcScalProd(const IlcFloatArray array1,
                        const IlcFloatVarArray array2);


IlcFloat    IlcScalProd(const IlcFloatArray array1,
                        const IlcFloatArray array2);

IlcFloat    IlcScalProd(const IlcFloatArray,
                        const IlcIntArray);

IlcFloat    IlcScalProd(const IlcIntArray intArray,
                        const IlcFloatArray floatArray);


IlcFloatExp IlcAbs(const IlcFloatExp exp);

inline IlcFloat IlcAbs(IlcFloat exp) { return exp >= 0 ? exp : -exp; }

IlcFloatExp IlcMax(const IlcFloatExp exp1, IlcFloat exp2);

IlcFloatExp IlcMax(IlcFloat exp1, const IlcFloatExp exp2);

IlcFloatExp IlcMax(const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcFloatExp IlcMax(const IlcFloatVarArray array);

IlcFloat    IlcMax(const IlcFloatArray array);

inline IlcFloat IlcMax(IlcFloat a, IlcFloat b) { return (a > b) ? a : b; }


IlcFloatExp IlcMin(const IlcFloatExp exp1, IlcFloat exp2);

IlcFloatExp IlcMin(IlcFloat exp1, const IlcFloatExp exp2);

IlcFloatExp IlcMin(const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcFloatExp IlcMin(const IlcFloatVarArray array);

IlcFloat    IlcMin(const IlcFloatArray array);

inline IlcFloat IlcMin(IlcFloat a, IlcFloat b) { return (a < b) ? a : b; }


IlcFloatExp operator*(const IlcFloatExp exp1, const IlcFloatExp exp2);


IlcFloatExp operator/(IlcFloat exp1, const IlcFloatExp exp2);


IlcFloatExp operator/(const IlcFloatExp exp1, const IlcFloatExp exp2);

IlcFloatExp operator*(IlcIntExp x, IlcFloatExp y);
IlcFloatExp operator*(IlcFloatExp x, IlcIntExp y);

IlcFloatExp operator/(IlcIntExp x, IlcFloatExp y);
IlcFloatExp operator/(IlcFloatExp x, IlcIntExp y);

IlcFloatExp operator/(IlcFloat x, IlcIntExp y);


IlcFloatExp IlcScalProd(const IlcFloatVarArray array1,
                        const IlcFloatVarArray array2);

IlcFloatExp IlcScalProd(const IlcIntVarArray array,
                        const IlcFloatArray coeffs);

IlcFloatExp IlcScalProd(const IlcFloatArray coeffs,
                        const IlcIntVarArray array);



IlcFloatExp   IlcSquare(const IlcFloatExp exp);


IlcFloatExp   IlcPower(const IlcFloatExp x, const IlcFloat p);

IlcFloatExp   IlcPower(const IlcFloatExp x, const IlcInt p);
#ifdef ILO_WIN64
IlcFloatExp   IlcPower(const IlcFloatExp var, const long exponent);
#endif

IlcConstraint IlcNull (const IlcFloatExp x);



IlcFloatExp IlcPower(IlcFloatExp x, IlcFloatExp p);

IlcFloatExp IlcPower(IlcFloat x, IlcFloatExp p);


IlcFloatExp   IlcExponent(const IlcFloatExp x);



IlcFloatExp   IlcLog(const IlcFloatExp x);


IlcGoal IlcInstantiate(const IlcFloatVar var,
                       IlcBool increaseMinFirst=IlcTrue,
                       IlcFloat prec=0);

IlcGoal IlcBestInstantiate(const IlcFloatVar var,
                           IlcBool increaseMinFirst=IlcTrue,
                           IlcFloat prec = 0);

IlcGoal IlcSplit(const IlcFloatVarArray vars,
                 IlcBool increaseMinFirst,
                 IlcFloat precSolveBounds);

IlcGoal IlcSplit(const IlcFloatVarArray vars,
                 IlcBool increaseMinFirst=IlcTrue);

void IlcSolveBounds(IlcFloatVar var, IlcFloat prec=.1);
void IlcSolveBounds(IlcFloatVarArray array, IlcFloat prec=.1);
IlcGoal IlcGenerateBounds(IlcFloatVarArray array, IlcFloat prec=0.1);
IlcGoal IlcGenerateBounds(IlcFloatVar var, IlcFloat prec=.1);
IlcFloat IlcPower(IlcFloat x, IlcFloat p);
IlcFloat IlcExponent(IlcFloat x);
IlcFloat IlcLog(IlcFloat x);

IlcIntExp IlcSgn(IlcFloatExp exp);
IlcIntExp IlcFloor(const IlcFloatExp var);
IlcIntExp IlcCeil(const IlcFloatExp var);
IlcIntExp IlcRound(const IlcFloatExp var);
IlcIntExp IlcFloatToInt(const IlcFloatExp var);
IlcConstraint IlcSpread(IlcIntVarArray vars, IlcFloatVar mean, IlcFloatVar sd);

//
// Set variable operators and constraints
//
inline IlcIntVar IlcCard(IlcIntSetVar set){ return set.getCardinality(); }
IlcInt IlcCard(IlcIntSet aSet);
IlcConstraint operator==(IlcIntSetVar set1, IlcIntSetVar set2);
IlcConstraint operator==(IlcIntSet set1, IlcIntSetVar set2);
IlcConstraint operator==(IlcIntSetVar set1, IlcIntSet set2);
IlcConstraint operator!=(IlcIntSetVar set1, IlcIntSetVar set2);
IlcConstraint operator!=(IlcIntSet set1, IlcIntSetVar set2);
IlcConstraint operator!=(IlcIntSetVar set1, IlcIntSet set2);
IlcConstraint IlcSubsetEq(IlcIntSetVar a, IlcIntSetVar b);
IlcConstraint IlcSubsetEq(IlcIntSet a, IlcIntSetVar b);
IlcConstraint IlcSubsetEq(IlcIntSetVar a, IlcIntSet b);
IlcConstraint IlcSubset(IlcIntSetVar a, IlcIntSetVar b);
IlcConstraint IlcSubset(IlcIntSet a, IlcIntSetVar b);
IlcConstraint IlcSubset(IlcIntSetVar a, IlcIntSet b);
IlcConstraint IlcMember(IlcIntExp element, IlcIntSetVar setVar);
IlcConstraint IlcMember(IlcInt element, IlcIntSetVar setVar);
IlcConstraint IlcNotMember(IlcIntExp element, IlcIntSetVar var);
IlcConstraint IlcNotMember(IlcInt element, IlcIntSetVar var);
IlcConstraint IlcOverlap(IlcIntSetVar a, IlcIntSetVar b);
IlcConstraint IlcNullIntersect(IlcIntSetVar a, IlcIntSetVar b);
IlcConstraint IlcNullIntersect(IlcIntSet a, IlcIntSetVar b);
IlcConstraint IlcNullIntersect(IlcIntSetVar a, IlcIntSet b);
IlcConstraint IlcEqIntersection(IlcIntSetVar intersection,
                                IlcIntSetVar var1, IlcIntSetVar var2);
IlcConstraint IlcEqUnion(IlcIntSetVar unionset,
                         IlcIntSetVar var1,
                         IlcIntSetVar var2,
                         IlcFilterLevel level);
IlcConstraint IlcEqUnion(IlcIntSetVar unionset,
                         IlcIntSetVar var1,
                         IlcIntSetVar var2);
IlcConstraint IlcEqUnion(IlcIntSetVar unionset,
                         IlcIntSetVar var1,
                         IlcIntSetVar var2,
                         IlcIntSetVar intersection);
IlcIntSetVar IlcUnion(IlcIntSetVar var1, IlcIntSetVar var2);
IlcIntSetVar IlcIntersection(IlcIntSetVar var1, IlcIntSetVar var2);



#define IlcChooseIndex1 IlcChooseIntIndex1

#define IlcChooseIntIndex1(name, criterion, type) \
IlcInt name(const ILCCONCAT(type,Array) vars) { \
    IlcInt indexBest=-1; \
    IlcInt min = IlcIntTop; \
    IlcInt size = vars.getSize(); \
    for (IlcInt varIndex=0; varIndex < size; varIndex++) {\
        type var = vars[varIndex]; \
        if (!var.isFixed()) { \
            IlcInt value = criterion; \
            if (min > value) { \
                indexBest = varIndex; \
                min = value; \
            } \
        }\
    } \
    return indexBest; \
}


#define IlcChooseIndex2 IlcChooseIntIndex2

#define IlcChooseIntIndex2(name, criterion1, criterion2, type) \
IlcInt name(const ILCCONCAT(type,Array) vars) { \
    IlcInt indexBest = -1; \
    IlcInt min1 = IlcIntTop; \
    IlcInt min2 = IlcIntTop; \
    IlcInt size = vars.getSize(); \
    for (IlcInt varIndex=0; varIndex < size; varIndex++) {\
        type var = vars[varIndex]; \
        if (!var.isFixed()) { \
            IlcInt value1 = criterion1; \
            if (value1 < min1) { \
                min1 = value1; \
                indexBest = varIndex; \
                min2 = criterion2; \
            } \
            else { \
                if (value1 == min1) { \
                    IlcInt value2 = criterion2; \
                    if (value2 < min2) { \
                        min2 = value2; \
                        indexBest = varIndex; \
                    } \
                } \
            } \
        } \
    } \
    return indexBest; \
}


typedef IlcInt (*IlcChooseIntIndex)(const IlcIntVarArray);

IlcInt IlcChooseFirstUnboundInt(const IlcIntVarArray vars);

IlcInt IlcChooseFirstNonFixedInt(const IlcIntVarArray vars);

IlcInt IlcChooseMinSizeInt(const IlcIntVarArray vars);

IlcInt IlcChooseMaxSizeInt(const IlcIntVarArray vars);

IlcInt IlcChooseMinMinInt(const IlcIntVarArray vars);

IlcInt IlcChooseMinMaxInt(const IlcIntVarArray vars);

IlcInt IlcChooseMaxMinInt(const IlcIntVarArray vars);

IlcInt IlcChooseMaxMaxInt(const IlcIntVarArray vars);

IlcInt IlcChooseMinRegretMin(const IlcIntVarArray vars);

IlcInt IlcChooseMinRegretMax(const IlcIntVarArray vars);

IlcInt IlcChooseMaxRegretMin(const IlcIntVarArray vars);

IlcInt IlcChooseMaxRegretMax(const IlcIntVarArray vars);


class IlcIntSelectI {
public:

  IlcIntSelectI() { }

  virtual ~IlcIntSelectI();

  virtual IlcInt select(IlcIntVar var);
};


typedef IlcInt (*IlcEvalInt) (IlcInt val, IlcIntVar var);


class IlcIntSelectEvalI :public IlcIntSelectI {
  IlcEvalInt _function;
public:

  IlcIntSelectEvalI(IlcEvalInt function):_function(function){};

  virtual IlcInt select(IlcIntVar var);
};


class IlcIntSelect {
  ILOCPVISIBLEHANDLEMINI(IlcIntSelect,IlcIntSelectI)
  void _ctor(IlcCPEngine solver, IlcEvalInt function);
public:

  IlcIntSelect(IlcCPEngine solver, IlcEvalInt function) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    _ctor(solver, function);
  }
  IlcInt select (IlcIntVar var) const {
    IlcCPOAssert(_impl != 0, "");
    return _impl->select(var);
  }
};

class IlcChooseIntVarI {
public:
  IlcChooseIntVarI() {}
  virtual ~IlcChooseIntVarI();
  virtual int getVarIndex(IlcIntVarArray array)=0;
};

class IlcChooseIntVar {
  ILOCPVISIBLEHANDLEMINI(IlcChooseIntVar, IlcChooseIntVarI)
};


IlcGoal IlcInstantiate(const IlcIntVar var);

IlcGoal IlcInstantiate(const IlcIntVar var, IlcIntSelect select);


IlcGoal IlcBestInstantiate(const IlcIntVar var);

IlcGoal IlcBestInstantiate(const IlcIntVar var, IlcIntSelect select);


IlcGoal IlcDichotomize(const IlcIntVar var, IlcBool increaseMin = IlcTrue);


IlcGoal IlcGenerate(const IlcIntVarArray array,
            IlcChooseIntIndex chooseVariable = IlcChooseFirstNonFixedInt);

IlcGoal IlcGenerate(const IlcIntVarArray array,
            IlcChooseIntIndex chooseVariable,
            IlcIntSelect select);

IlcGoal IlcGenerate(const IlcIntVarArray,
            IlcChooseIntVar chooseVariable);
IlcGoal IlcGenerate(const IlcIntVarArray,
            IlcChooseIntVar chooseVariable,
            IlcIntSelect);


IlcGoal IlcBestGenerate(const IlcIntVarArray,
                IlcChooseIntIndex chooseVariable = IlcChooseFirstNonFixedInt);


IlcGoal IlcBestGenerate(const IlcIntVarArray,
                IlcChooseIntIndex chooseVariable,
                IlcIntSelect select);


IlcGoal IlcSetValue(const IlcIntVar var, const IlcInt val);


IlcGoal IlcRemoveValue(const IlcIntVar var, const IlcInt val);


IlcGoal IlcSetMin(const IlcIntVar var, const IlcInt val);


IlcGoal IlcSetMax(const IlcIntVar var, const IlcInt val);

IlcGoal IlcMinimizeGoal(IlcGoal g, IlcIntVar v, IlcInt s = 1);


IloGoal IloGenerate(const IloEnv env,
                    const IloIntVarArray vars,
                    IlcChooseIntIndex sel = IlcChooseFirstNonFixedInt);

IloGoal IloGenerate(const IloEnv env,
                    const IloNumVarArray vars,
                    IlcChooseIntIndex sel = IlcChooseFirstNonFixedInt);


IlcGoal IlcSimpleCompletionGoal(IlcCPEngine solver);


IloGoal IloSimpleCompletionGoal(IloEnv env);

IlcGoal IlcFailureDirectedSearchGoal(IlcCPEngine solver);
IloGoal IloFailureDirectedSearchGoal(IloEnv env);

// --------------------------------------------------------------------------
// Member
// --------------------------------------------------------------------------


IlcConstraint IlcMember(const IlcIntVar exp, const IlcIntArray elements);

IlcConstraint IlcNotMember(const IlcIntVar exp, const IlcIntArray elements);

// --------------------------------------------------------------------------
// TableConstraint
// --------------------------------------------------------------------------


class IlcIntPredicateI {
public:

  IlcIntPredicateI(){}

  virtual ~IlcIntPredicateI();

  virtual IlcBool isTrue(IlcIntArray val)=0;
};


class IlcIntPredicate {
  ILOCPVISIBLEHANDLEMINI(IlcIntPredicate, IlcIntPredicateI)
public:

  IlcBool isTrue(IlcIntArray val){
    IlcCPOAssert(_impl != 0, "");
    return _impl->isTrue(val);
  }
};


//---------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------
//
// ILCINTPREDICATE0

#define ILCINTPREDICATENAME0(name, envName)\
class envName : public IlcIntPredicateI { \
  public:\
    envName():IlcIntPredicateI(){}\
    IlcBool isTrue(IlcIntArray val);\
};\
IlcIntPredicate name(IlcCPEngine solver){\
  return new (solver.getHeap()) envName();\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE0(name)\
ILCINTPREDICATENAME0(name, ILCCONCAT(name,I))

// ILCINTPREDICATE1

#define ILCINTPREDICATENAME1(name, envName, type1, nameArg1)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    envName(type1 IlcArg1);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1)\
    :IlcIntPredicateI(),nameArg1(IlcArg1){}\
IlcIntPredicate name(IlcCPEngine solver,type1 arg1){\
 return new (solver.getHeap()) envName(arg1);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE1(name, type1, nameArg1)\
ILCINTPREDICATENAME1(name, ILCCONCAT(name,I), type1, nameArg1)

// ILCINTPREDICATE2

#define ILCINTPREDICATENAME2(name, envName, type1, nameArg1, type2, nameArg2)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    type2 nameArg2; \
    envName(type1 IlcArg1, type2 IlcArg2);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1, type2 IlcArg2)\
   :IlcIntPredicateI(),nameArg1(IlcArg1), nameArg2(IlcArg2){}\
IlcIntPredicate name(IlcCPEngine solver, type1 arg1, type2 arg2){\
 return new (solver.getHeap()) envName(arg1, arg2);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE2(name, type1, nameArg1, type2, nameArg2)\
ILCINTPREDICATENAME2(name, ILCCONCAT(name,I), type1, nameArg1, type2, nameArg2)

// ILCINTPREDICATE3

#define ILCINTPREDICATENAME3(name, envName, type1, nameArg1, type2, nameArg2, type3, nameArg3)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    type2 nameArg2; \
    type3 nameArg3; \
    envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3)\
   :IlcIntPredicateI(),nameArg1(IlcArg1), nameArg2(IlcArg2), nameArg3(IlcArg3){}\
IlcIntPredicate name(IlcCPEngine solver, type1 arg1, type2 arg2, type3 arg3){\
 return new (solver.getHeap()) envName(arg1, arg2, arg3);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE3(name, type1, nameArg1, type2, nameArg2, type3, nameArg3)\
ILCINTPREDICATENAME3(name, ILCCONCAT(name,I), type1, nameArg1, type2, nameArg2, type3, nameArg3)

// ILCINTPREDICATE4

#define ILCINTPREDICATENAME4(name, envName, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    type2 nameArg2; \
    type3 nameArg3; \
    type4 nameArg4; \
    envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4)\
   :IlcIntPredicateI(),nameArg1(IlcArg1), nameArg2(IlcArg2), nameArg3(IlcArg3), nameArg4(IlcArg4){}\
IlcIntPredicate name(IlcCPEngine solver, type1 arg1, type2 arg2, type3 arg3, type4 arg4){\
 return new (solver.getHeap()) envName(arg1, arg2, arg3, arg4);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE4(name, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4)\
ILCINTPREDICATENAME4(name, ILCCONCAT(name,I), type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4)


// ILCINTPREDICATE5

#define ILCINTPREDICATENAME5(name, envName, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    type2 nameArg2; \
    type3 nameArg3; \
    type4 nameArg4; \
    type5 nameArg5; \
    envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4, type5 IlcArg5);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4, type5 IlcArg5)\
   :IlcIntPredicateI(),nameArg1(IlcArg1), nameArg2(IlcArg2), nameArg3(IlcArg3), nameArg4(IlcArg4), nameArg5(IlcArg5){}\
IlcIntPredicate name(IlcCPEngine solver, type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5){\
 return new (solver.getHeap()) envName(arg1, arg2, arg3, arg4, arg5);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE5(name, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5)\
ILCINTPREDICATENAME5(name, ILCCONCAT(name,I), type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5)

// ILCINTPREDICATE6

#define ILCINTPREDICATENAME6(name, envName, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5, type6, nameArg6)\
class envName : public IlcIntPredicateI { \
  public:\
    type1 nameArg1; \
    type2 nameArg2; \
    type3 nameArg3; \
    type4 nameArg4; \
    type5 nameArg5; \
    type6 nameArg6; \
    envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4, type5 IlcArg5, type6 IlcArg6);\
    IlcBool isTrue(IlcIntArray val);\
};\
envName::envName(type1 IlcArg1, type2 IlcArg2, type3 IlcArg3, type4 IlcArg4, type5 IlcArg5, type6 IlcArg6)\
   :IlcIntPredicateI(),nameArg1(IlcArg1), nameArg2(IlcArg2), nameArg3(IlcArg3), nameArg4(IlcArg4), nameArg5(IlcArg5), nameArg6(IlcArg6){}\
IlcIntPredicate name(IlcCPEngine solver, type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6){\
 return new (solver.getHeap()) envName(arg1, arg2, arg3, arg4, arg5, arg6);\
}\
IlcBool envName ::isTrue(IlcIntArray val)


#define ILCINTPREDICATE6(name, type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5, type6, nameArg6)\
ILCINTPREDICATENAME6(name, ILCCONCAT(name,I), type1, nameArg1, type2, nameArg2, type3, nameArg3, type4, nameArg4, type5, nameArg5, type6, nameArg6)

//
// API : Handle : IlcIntTupleSet
//


class IlcIntTupleSet {
  ILOCPHIDDENHANDLEMINI(IlcIntTupleSet,IlcCPOECSetOfSharedTupleI)
  void _ctor(IlcCPEngine solver, IlcInt arity);
public:

  IlcIntTupleSet(IlcCPEngine solver, IlcInt arity) {
    IlcCPOAssert(solver.getImpl() != 0, "IlcCPEngine: empty handle");
    IlcCPOAssert(arity >= 1, "IlcIntTupleSet: arity must be at least 1");
    _ctor(solver, arity);
  }

  void add(IlcIntArray tuple)const;

  IlcBool isClosed()const;

  void close()const;

  IlcBool isIn(IlcIntArray tuple)const;
  void setHoloTuple()const;
  void setBigTuple()const;
  void setSimpleTuple()const;
  IlcInt getArity() const;
  IlcInt getSize() const;
};

class IlcIntTupleSetIterator {
  void* _current;
  IlcBool _ok;
  IlcBool nnext();
public:
        IlcIntTupleSetIterator(const IlcIntTupleSet tset);
        ~IlcIntTupleSetIterator(){}
        IlcIntTupleSetIterator& operator++(){
                _ok=nnext();
                return *this;
        }
        IlcIntArray operator*() const;
        IlcBool ok() const{return _ok;}
};



IlcConstraint IlcTableConstraint(IlcIntVarArray vars,
                                 IlcIntPredicate predicate);

IlcConstraint IlcTableConstraint(IlcIntVarArray vars, IlcIntTupleSet set,
                                 IlcBool compatible);

IlcConstraint IlcTableConstraint(IlcIntVar y, IlcIntArray a, IlcIntVar x);


IlcConstraint IlcElementEq(IlcIntVar var, const IlcIntArray array, IlcIntVar index, const char* name=0);

IlcConstraint IlcElementNEq(IlcIntVar var, const IlcIntArray array, IlcIntVar index, const char* name=0);

IlcConstraint IlcElementEq(IlcInt val, const IlcIntArray array, IlcIntVar index, const char* name=0);

IlcConstraint IlcElementNEq(IlcInt val, const IlcIntArray array, IlcIntVar index, const char* name=0);

IlcConstraint IlcPack(IlcIntVarArray load,
                      IlcIntVarArray where,
                      IlcIntArray weight);


IlcConstraint IlcPack(IlcIntVarArray load,
                      IlcIntVarArray where,
                      IlcIntArray weight,
                      IlcIntVar used);


IlcConstraint IlcPack(IlcIntVarArray load,
                      IlcIntVarArray where,
                      IlcIntArray weight,
                      IlcIntSetVar used);


IlcConstraint IlcLexicographic(IlcIntVarArray x, IlcIntVarArray y);
IlcConstraint IlcGeLex(IlcIntVarArray x, IlcIntVarArray y);
IlcConstraint IlcLeLex(IlcIntVarArray x, IlcIntVarArray y);

//
// used in wrappers
//


IlcConstraint IlcCustomConstraint(IloCPEngine solver, IloPropagatorI * prop);

// Signs


IlcIntExp IlcSgn(IlcIntExp exp);


///////////////////////////////////////////////////////////////////////////
// IlcBranchSelectorI
///////////////////////////////////////////////////////////////////////////




class IlcSearchMonitorI : public IlcRtti {
public:
  IlcSearchMonitorI() {}
  virtual ~IlcSearchMonitorI();
private:
  virtual void whenSuccess();
  virtual void whenChoicePoint();
  virtual void whenBranch(IlcBool dir); // 0 = left, 1 = right
  virtual void whenBeforeFail();
  virtual void whenAfterFail();
  virtual void whenRestartSearch();
  virtual void whenResumeSearch();
  virtual void whenStartSearch(IlcBool initial);
  virtual void whenEndSearch();
  virtual void whenBeforeInitialPropagation();
  virtual void whenAfterInitialPropagation();
  virtual void whenBacktrackOneLevel(IlcCPEngine solver);
  virtual void whenBeginGoal(IlcGoal g);
  virtual void whenEndGoal(IlcGoal g);
public:
  virtual IlcBool isRecursive() const;
  friend class IlcSearchMonitorWrapper;
  ILCRTTIDECL
};




class IlcSearchLimitI : public IlcSearchMonitorI {
private:
  void *        _manager;
  IlcBool       _state;
  IlcBool       _activate;
public:

  IlcSearchLimitI(IlcCPEngine cp);
  IlcSearchLimitI(IlcManagerI * manager);

  virtual ~IlcSearchLimitI();

  virtual IlcBool check() const; 
  virtual IlcBool checkTimeLimit() const;
  virtual IlcBool checkTimeLimitWrapper(); 

  virtual void init();
  virtual void display(ILOSTD(ostream&)) const;
  virtual void whenChoicePoint();
  virtual void whenAfterFail();
  virtual void whenSuccess();
  void activate();
  void deactivate();
  void stopIfNeeded();
  virtual IlcBool checkWrapper();
  IlcBool limitCrossed() const { return _state; }
  IlcBool isActive() const { return _activate; }
  IlcManagerI * getManagerI() const { return (IlcManagerI*)_manager; }
  IlcCPEngineI* getCPEngineI() const { return IlcGetCPEngineI((IlcManagerI*)_manager); }
  IlcCPEngine getCPEngine() const { return getCPEngineI(); }
  ILCRTTIDECL
};


class IlcSearchLimit {
  ILOCPVISIBLEHANDLEMINI(IlcSearchLimit, IlcSearchLimitI)
};


IlcGoal IlcLimitSearch(IlcGoal goal, IlcSearchLimit searchLimit);


IlcSearchLimit IlcTimeLimit(IlcCPEngine solver, IlcFloat limit);


IlcSearchLimit IlcFailLimit(IlcCPEngine solver, IlcInt limit);


IlcSearchLimit IlcOrLimit(IlcCPEngine solver, IlcInt limit);


IlcSearchLimit IlcBranchLimit(IlcCPEngine solver, IlcInt limit);


class IloSearchLimitI : public IloRttiEnvObjectI {
public:
  IloSearchLimitI(IloEnvI*);
  virtual ~IloSearchLimitI();
  
  virtual IlcSearchLimit extract(const IloCPEngine solver) const=0;
  IlcSearchLimit extract(IlcCPEngineI* engine) const;
  
  virtual IloSearchLimitI* makeClone(IloEnvI* env) const=0;
  virtual void display(ILOSTD(ostream&)) const;
  ILORTTIDECL
};

class IloSearchLimit {
  ILOCPVISIBLEHANDLEMINI(IloSearchLimit, IloSearchLimitI)
public:
  void end() const;
};
ILOSTD(ostream&) operator<<(ILOSTD(ostream&), const IloSearchLimit&);

IloSearchLimit IloTimeLimit(const IloEnv env, IloNum time);
IloSearchLimit IloFailLimit(const IloEnv env, IloInt maxNbFails);
IloSearchLimit IloOrLimit(const IloEnv env, IloInt numOfChoicePts);

IloGoal IloLimitSearch(const IloEnv env, const IloGoal goal, const IloSearchLimit searchLimit);



class IloCPConstraintI : public IloConstraintI {
  ILORTTIDECL
  virtual void visitSubExtractables(IloExtractableVisitor* v);
  IloAny extractToCP(const IloAlgorithm alg) const;
  void unextractFromCP(const IloAlgorithm, IloAny) const;
  void modifyCP(const IloAlgorithm alg, IloAny) const;
public:

  IloCPConstraintI(IloEnvI*, const char*);

  virtual ~IloCPConstraintI();
  // virtual API

  
  virtual IlcConstraint extract(const IloCPEngine solver) const=0;
  IlcConstraint extract(IlcCPEngineI* engine) const;
  

  virtual IloExtractableI* makeClone(IloEnvI* env) const;

  virtual void display(ILOSTD(ostream& out)) const;
  // predefined methods

  void use(const IloCPEngine solver, const IloNumVar x) const;

  void use(const IloCPEngine solver, const IloIntervalVar x) const;

  void use(const IloCPEngine solver, const IloIntervalSequenceVar x) const;


  void use(const IloCPEngine solver, const IloIntVarArray xa) const;

  void use(const IloCPEngine solver, const IloNumVarArray xa) const;

  void use(const IloCPEngine solver, const IloIntervalVarArray xa) const;

  void use(const IloCPEngine solver, const IloIntervalSequenceVarArray xa) const;
};


extern "C" void ilcpo_fail_stop_here();

#ifdef CPPREF_GENERATION

extern ILC_NO_MEMORY_MANAGER;

#endif


////////////////////////////////////////////////////////////////////////////
//
// CUSTOM INFERENCERS
//
////////////////////////////////////////////////////////////////////////////



class IlcCustomInferencerI {
  friend class IloInferencer;
private:
  IloNum _propagationCost;
  IloBool _manualMode;
  IloNum _numberSkippedNodes;
  IlcCPEngineI* _cp;
  void resetCost() {
    _propagationCost=0;
  }
  IloNum getPropagationCost() {return _propagationCost;}
protected:
  
  void addPropagationCost(IloNum c) {_propagationCost+= c;}
  virtual void incrementalExecute(); 
public:
  IlcCPEngineI* getCPEngineI() const { return _cp; }
  
  IlcCustomInferencerI(IlcCPEngine solver, IloBool manualMode=IloFalse, IloNum numberOfSkippedNodes = 20);
  
  virtual ~IlcCustomInferencerI() {};
          
  virtual void execute()=0;
          
  virtual IloNum estimateCost(IloNum bound);
};


class IlcCustomInferencer {
  ILOCPVISIBLEHANDLE(IlcCustomInferencer,IlcCustomInferencerI)
public:
  operator IlcConstraint(); 
};

////////////////////////////////////////////////////////////////////////////
//
// new operator
//
////////////////////////////////////////////////////////////////////////////


void* operator new (size_t s, IlcAllocationStack* heap);

#if defined (ILONEWOPERATOR)
inline void* operator new[] (size_t s, IlcAllocationStack* heap){
    return operator new (s, heap);
}
inline void operator delete [] (void*, IlcAllocationStack*){}
#endif
#if defined(ILODELETEOPERATOR)
inline void operator delete(void*, size_t, IlcAllocationStack *){}
inline void operator delete(void*, IlcAllocationStack *){}
#endif

#ifndef IloIntervalMin
#define IloIntervalMin (IlcIntMin/2 + 1)
#endif
#ifndef IloIntervalMax
#define IloIntervalMax (IlcIntMax/2 - 1)
#endif

// --------------------------------------------------------------------------
// INTERVAL VARIABLE
// --------------------------------------------------------------------------


class IlcIntervalVar {
  ILOCPHIDDENHANDLEMINI(IlcIntervalVar, IlcsIntervalVarI)
  ILCEXTENSIONMETHODSHDECL(IlcIntervalVar)
private:
  IlcCPEngineI*   _getCPEngineI()         const;
  IlcInt  _getStartMin()              const;
  IlcInt  _getStartMax()              const;
  IlcInt  _getEndMin()                const;
  IlcInt  _getEndMax()                const;
  IlcInt  _getSizeMin()               const;
  IlcInt  _getSizeMax()               const;
  IlcInt  _getLengthMin()             const;
  IlcInt  _getLengthMax()             const;
  IlcBool _isPresent()                const;
  IlcBool _isAbsent()                 const;
  IlcBool _isSizeFixed()              const;
  IlcBool _isPresenceFixed()          const;
  IlcBool _isFixed()                  const;
  IlcBool _hasDeltaPresence()         const;
  IlcBool _hasDeltaInterval()         const;
  IlcBool _hasDeltaSize()             const;
  IlcBool _hasDelta()                 const;
  IlcInt  _getOldStartMin()           const;
  IlcInt  _getOldStartMax()           const;
  IlcInt  _getOldEndMin()             const;
  IlcInt  _getOldEndMax()             const;
  IlcInt  _getOldSizeMin()            const;
  IlcInt  _getOldSizeMax()            const;
  IlcInt  _getOldLengthMin()          const;
  IlcInt  _getOldLengthMax()          const;
  IlcBool _isOldPresent()             const;
  IlcBool _isOldAbsent()              const;
  void _whenPresence(const IlcDemon d)         const;
  void _whenIntervalDomain(const IlcDemon d)   const;
  void _whenSize(const IlcDemon d)             const;
  void _setStartRange(IlcInt min, IlcInt max)  const;
  void _setEndRange(IlcInt min, IlcInt max)    const;
  void _setSizeRange(IlcInt min, IlcInt max)   const;
  void _setLengthRange(IlcInt min, IlcInt max) const;
  void _setPresence(IloBool present)           const;
protected:
  friend class IlcCumulElementVar;
  friend void
  IlcEndBeforeStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                    const IlcInt z);
  friend void
  IlcEndBeforeEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
                  const IlcInt z);
  friend void
  IlcStartBeforeStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                      const IlcInt z);
  friend void
  IlcStartBeforeEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
                    const IlcInt z);
  friend void
  IlcEndAtStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                const IlcInt z);
  friend void
  IlcEndAtEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
              const IlcInt z);
  friend void
  IlcStartAtStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                  const IlcInt z);
  friend void
  IlcStartAtEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
                const IlcInt z);
  friend void
  IlcPresenceImply(const IlcIntervalVar x1, const IlcIntervalVar x2);
  friend void
  IlcPresenceImplyNot(const IlcIntervalVar x1, const IlcIntervalVar x2);
  friend void
  IlcPresenceEqual(const IlcIntervalVar x1, const IlcIntervalVar x2);
  friend void
  IlcPresenceDifferent(const IlcIntervalVar x1, const IlcIntervalVar x2);
  friend void
  IlcPresenceOr(const IlcIntervalVar x1, const IlcIntervalVar x2);
  IlcBool _isInSearch()                           const;
  IlcBool _isInPropagation()                      const;
  IlcBool _isInConstraintPosting()                const;
  static IlcBool _isValidIntegerArg(IlcInt z);
  IlcBool _areInSameSearch(IlcIntervalVar x2)     const;
  void _setPrecedenceArc(const IlcIntervalVar,
                         const IlcBool onStart1,
                         const IlcBool onStart2,
                         const IlcBool at,
                         const IlcInt diff) const;
  void _setImplicationArc(const IlcIntervalVar,
                          const IlcBool positive,
                          const IlcBool equality) const;
  void _setOrArc(const IlcIntervalVar) const;
public:
  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getCPEngineI();
  }
                                                  
  IlcInt getStartMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getStartMin();
  }
                                                  
  IlcInt getStartMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getStartMax();
  }
                                                  
  IlcInt getEndMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getEndMin();
  }
                                                  
  IlcInt getEndMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getEndMax();
  }
                                                  
  IlcInt getSizeMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getSizeMin();
  }
                                                  
  IlcInt getSizeMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getSizeMax();
  }
                                                  
  IlcInt getLengthMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getLengthMin();
  }
                                                  
  IlcInt getLengthMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _getLengthMax();
  }
                                                  
  IlcBool isPresent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _isPresent();
  }
                                                  
  IlcBool isAbsent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _isAbsent();
  }
                                                  
  IlcBool isSizeFixed() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _isSizeFixed();
  }
                                                  
  IlcBool isPresenceFixed() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _isPresenceFixed();
  }
                                                  
  IlcBool isFixed() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _isFixed();
  }
                                                  
  IlcBool hasDeltaPresence() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _hasDeltaPresence();
  }
  
   IlcBool hasDeltaInterval() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _hasDeltaInterval();
  }
  
  IlcBool hasDeltaSize() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    return _hasDeltaSize();
  }
                                                    
  IlcInt getOldStartMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldStartMin();
  }
                                                    
  IlcInt getOldStartMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldStartMax();
  }
                                                    
  IlcInt getOldEndMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldEndMin();
  }
                                                    
  IlcInt getOldEndMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldEndMax();
  }
                                                    
  IlcInt getOldSizeMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldSizeMin();
  }
                                                    
  IlcInt getOldSizeMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldSizeMax();
  }
                                                    
   IlcInt getOldLengthMin() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldLengthMin();
  }
                                                    
  IlcInt getOldLengthMax() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _getOldLengthMax();
  }
                                                    
  IlcBool isOldPresent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _isOldPresent();
  }
                                                    
  IlcBool isOldAbsent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalVar: not in propagation");
    return _isOldAbsent();
  }  
  
  void whenPresence(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(d.getImpl(), "IlcDemon: empty handle.");    
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalVar: not in search");
    _whenPresence(d);
  }
  
  void whenIntervalDomain(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(d.getImpl(), "IlcDemon: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalVar: not in search");
    _whenIntervalDomain(d);
  }
  
  void whenSize(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(d.getImpl(), "IlcDemon: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalVar: not in search");
    _whenSize(d);
  }
                                                  
  void setStartMin(IlcInt min) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setStartRange(min, IloIntervalMax);
  }
                                                  
  void setStartMax(IlcInt max) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setStartRange(IloIntervalMin, max);
  }
                                                  
  void setStart(IlcInt value) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setStartRange(value, value);
  }
  
  void setEndMin(IlcInt min) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setEndRange(min, IloIntervalMax);
  }
                                                  
  void setEndMax(IlcInt max) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setEndRange(IloIntervalMin, max);
  }
                                                  
  void setEnd(IlcInt value) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setEndRange(value, value);
  }
                                                  
  void setSizeMin(IlcInt min) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setSizeRange(min, IloIntervalMax);
  }
                                                  
  void setSizeMax(IlcInt max) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setSizeRange(0, max);
  }
                                                  
  void setSize(IlcInt value) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setSizeRange(value, value);
  }
                                                  
  void setLengthMin(IlcInt min) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setLengthRange(min, IloIntervalMax);
  }
                                                  
  void setLengthMax(IlcInt max) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setLengthRange(0, max);
  }
                                                  
  void setLength(IlcInt value) const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setLengthRange(value, value);
  }
                                                  
  
  void setPresent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setPresence(IlcTrue);
  }
                                                  
  void setAbsent() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalVar: not in search");
    _setPresence(IlcFalse);
  }
};

  

ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntervalVar& exp);

  
class IlcIntervalVarArray {
  ILOCPHIDDENHANDLEMINI(IlcIntervalVarArray, IlcIntervalVarArrayI)
  ILCEXTENSIONMETHODSHDECL(IlcIntervalVarArray)
private:
  void _ctor(IlcCPEngineI* cp, const IloInt size);
  IlcCPEngineI* _getCPEngineI()              const;
  IlcInt _getSize()                   const;
  IlcIntervalVarArray _getCopy()      const;
  IlcIntervalVar& _get(IlcInt index)  const;
public:
  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
  
  IlcIntervalVarArray(const IlcCPEngine cp, const IloInt size) {
    IlcCPOAssert(cp.getImpl(), "IlcIntervalVarArray: IlcCPEngine empty handle.");
    IlcCPOAssert(size >= 0, "IlcIntervalVarArray: strictly negative size value");
    _ctor(cp.getImpl(), size);
  }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVar: empty handle.");
    return _getCPEngineI();
  }
  
  IloInt getSize() const {
     IlcCPOAssert(getImpl(), "IlcIntervalVarArray: empty handle.");
     return _getSize();
  }
  
  IlcIntervalVarArray getCopy() const {
    IlcCPOAssert(getImpl(), "IlcIntervalVarArray: empty handle");
    return _getCopy();
  }
  
  IlcIntervalVar& operator[] (IlcInt index) const {
    IlcCPOAssert(getImpl(), "IlcIntVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < _getSize(),
              "IlcIntervalVarArray: index out of range");
    return _get(index);
  }
};
  

ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntervalVarArray& exp);

#ifdef sparc
#pragma error_messages (off,notused)
#endif
                                                
inline void
IlcEndBeforeStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                  const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcFalse, IlcTrue, IlcFalse, z);
}


inline void
IlcEndBeforeEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
                const IlcInt  z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcFalse, IlcFalse, IlcFalse, z);
}


inline void
IlcStartBeforeStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                    const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcTrue, IlcTrue, IlcFalse, z);
}


inline void
IlcStartBeforeEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
                  const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcTrue, IlcFalse, IlcFalse, z);
}

                                                
inline void
IlcEndAtStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
              const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcFalse, IlcTrue, IlcTrue, z);
}

                                                
inline void
IlcEndAtEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
            const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcFalse, IlcFalse, IlcTrue, z);
}

                                                
inline void
IlcStartAtStart(const IlcIntervalVar x1, const IlcIntervalVar x2,
                const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcTrue, IlcTrue, IlcTrue, z);
}

                                                
inline void
IlcStartAtEnd(const IlcIntervalVar x1, const IlcIntervalVar x2,
              const IlcInt z = 0) {
  IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
  IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
  IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
  IlcCPOAssert(x1._isValidIntegerArg(z), "IlcIntervalVar: integer out of range");
  x1._setPrecedenceArc(x2, IlcTrue, IlcFalse, IlcTrue, z);
}

                                                 
inline void
IlcPresenceImply(const IlcIntervalVar x1, const IlcIntervalVar x2) {
 IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
 IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
 x1._setImplicationArc(x2, IlcTrue, IlcFalse);
}

#ifdef sparc
#pragma error_messages (default,notused)
#endif

                                                  
inline void
IlcPresenceImplyNot(const IlcIntervalVar x1, const IlcIntervalVar x2) {
 IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
 IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
 x1._setImplicationArc(x2, IlcFalse, IlcFalse);
}

                                                 
inline void
IlcPresenceOr(const IlcIntervalVar x1, const IlcIntervalVar x2) {
 IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
 IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
 x1._setOrArc(x2);
}

                                                 
 inline void
IlcPresenceEqual(const IlcIntervalVar x1, const IlcIntervalVar x2) {
 IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
 IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
 x1._setImplicationArc(x2, IlcTrue, IlcTrue);
}

                                                  
inline void
IlcPresenceDifferent(const IlcIntervalVar x1, const IlcIntervalVar x2) {
 IlcCPOAssert(x1.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x2.getImpl(), "IlcIntervalVar: empty handle.");
 IlcCPOAssert(x1._isInSearch(), "IlcIntervalVar: not in search");
 IlcCPOAssert(x1._areInSameSearch(x2), "IlcIntervalVar: not in same search");
 x1._setImplicationArc(x2, IlcFalse, IlcTrue);
}


class IlcCumulElementVar {
  ILOCPHIDDENHANDLEMINI(IlcCumulElementVar, IlcsDemandI)
  ILCEXTENSIONMETHODSHDECL(IlcCumulElementVar)
private:
  IlcCPEngineI* _getCPEngineI()          const;
  IlcIntervalVar _getInterval()                  const;
  IlcBool _isNull()                              const;
  IlcBool _isFixed()                             const;
  IlcBool _isHeightFixed()                       const;
  IlcInt _getHeightMin()                         const;
  IlcInt _getHeightMax()                         const;
  IlcBool _hasDeltaHeight()                      const;
  IlcInt _getOldHeightMin()                      const;
  IlcInt _getOldHeightMax()                      const;
  void _setHeightRange(IlcInt hmin, IlcInt hmax) const;
  void _whenHeight(const IlcDemon d)                   const;
  IlcInt _getShape()                             const;
protected:
  IlcBool _isInSearch() const { return _getInterval()._isInSearch();}
  IlcBool _isInPropagation() const {return _getInterval()._isInPropagation();}
public:
  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _getInterval()._getCPEngineI();
  }
                                                  
  IlcBool isStepAtStart() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return (_getShape() == 1) ? IlcTrue : IlcFalse;
  }
                                                  
  IlcBool isStepAtEnd() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return (_getShape() == -1) ? IlcTrue : IlcFalse;
  }
                                                  
  IlcBool isPulse() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return (_getShape() == 0) ? IlcTrue : IlcFalse;
  }
                                                                         
  IlcIntervalVar getInterval() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _getInterval();
  }
                                                  
  IlcBool isFixed() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _isFixed();
  }
                                                  
  IlcBool isHeightFixed() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _isHeightFixed();
  }
                                                  
  IlcBool isNull() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _isNull();
  }
                                                  
  IlcInt getHeightMin() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _getHeightMin();
  }
                                                  
  IlcInt getHeightMax() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _getHeightMax();
  }
                                                    
  IlcInt getOldHeightMin() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcCumulElementVar: not in propagation");
    return _getOldHeightMin();
  }
                                                    
  IlcInt getOldHeightMax() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcCumulElementVar: not in propagation");
    return _getOldHeightMax();
  }
                                                  
  void setHeightMin(IlcInt min) const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    _setHeightRange(min, IloIntervalMax);
  }
                                                  
  void setHeightMax(IlcInt max) const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    _setHeightRange(0, max);
  }
                                                  
  void setHeight(IlcInt value) const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    _setHeightRange(value, value);
  }
  
  IlcBool hasDeltaHeight() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    return _hasDeltaHeight();
  }
  
  void whenHeight(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    IlcCPOAssert(d.getImpl(), "IlcDemon : empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcCumulElementVar: not in search");
    _whenHeight(d);
  }
};


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcCumulElementVar& exp);


  
class IlcCumulElementVarArray {
  ILOCPHIDDENHANDLEMINI(IlcCumulElementVarArray, IlcCumulElementVarArrayI)
  ILCEXTENSIONMETHODSHDECL(IlcCumulElementVarArray)
private:
  void _ctor(IlcCPEngineI* cp, const IloInt size);
  IlcCPEngineI* _getCPEngineI()              const;
  IlcInt _getSize()                   const;
  IlcCumulElementVarArray _getCopy()      const;
  IlcCumulElementVar& _get(IlcInt index)  const;
public:
  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
  
  IlcCumulElementVarArray(const IlcCPEngine cp, const IloInt size) {
    IlcCPOAssert(cp.getImpl(), "IlcCumulElementVarArray: IlcCPEngine empty handle.");
    IlcCPOAssert(size >= 0, "IlcCumulElementVarArray: strictly negative size value");
    _ctor(cp.getImpl(), size);
  }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVar: empty handle.");
    return _getCPEngineI();
  }
  
  IloInt getSize() const {
     IlcCPOAssert(getImpl(), "IlcCumulElementVarArray: empty handle.");
     return _getSize();
  }
  
  IlcCumulElementVarArray getCopy() const {
    IlcCPOAssert(getImpl(), "IlcCumulElementVarArray: empty handle");
    return _getCopy();
  }
  
  IlcCumulElementVar& operator[] (IlcInt index) const {
    IlcCPOAssert(getImpl(), "IlcIntVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < _getSize(),
              "IlcCumulElementVarArray: index out of range");
    return _get(index);
  }

};
  

ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcCumulElementVarArray& exp);



class IlcIntervalSequenceVar {
  ILOCPHIDDENHANDLEMINI(IlcIntervalSequenceVar, IlcsIntervalSequenceVarI)
  ILCEXTENSIONMETHODSHDECL(IlcIntervalSequenceVar)
private:
  IlcCPEngineI* _getCPEngineI()                                 const;
  IlcBool _isIn(const IlcIntervalVar var)                const;
  IlcInt _getType(const IlcIntervalVar var)              const;
  IlcBool _isFixed()                                     const;
  IlcBool _isSequenced()                                 const;
  IlcBool _isSequenced(const IlcIntervalVar var)         const; 
  IlcBool _isPresent(const IlcIntervalVar var)           const;
  IlcBool _isAbsent(const IlcIntervalVar var)            const;
  IlcBool _isInHead(const IlcIntervalVar var)            const;
  IlcBool _isInTail(const IlcIntervalVar var)            const;
  IlcBool _isCandidateHead(const IlcIntervalVar var)     const;
  IlcBool _isCandidateTail(const IlcIntervalVar var)     const;

  IlcIntervalVar _getEarliestInHead()                    const;
  IlcIntervalVar _getEarliestInTail()                    const;
  IlcIntervalVar _getLatestInHead()                      const;
  IlcIntervalVar _getLatestInTail()                      const;
  IlcIntervalVar _getLatestPresentInHead()               const;
  IlcIntervalVar _getLatestPresentInTail()               const;

  IlcIntervalVar
  _getOneEarlierInHead(const IlcIntervalVar var)         const;
  IlcIntervalVar
  _getOneLaterInHead(const IlcIntervalVar var)           const;
  IlcIntervalVar
  _getOneEarlierInTail(const IlcIntervalVar var)         const;
  IlcIntervalVar
  _getOneLaterInTail(const IlcIntervalVar var)           const;

  IlcBool _isEarlierInTail(const IlcIntervalVar earlier,
                           const IlcIntervalVar later)   const;
  IlcBool _isEarlierInHead(const IlcIntervalVar earlier,
                           const IlcIntervalVar later)   const;

  void _setPresence(const IlcIntervalVar var,
                    const IloBool present)               const;
  void _extendHead(const IlcIntervalVar var)             const;
  void _extendTail(const IlcIntervalVar var)             const;
  void _removeCandidateHead(const IlcIntervalVar var)    const;
  void _removeCandidateTail(const IlcIntervalVar var)    const;

  void _whenPresence(const IlcDemon d)                   const;
  IlcIntervalVar _getDeltaPresence()                     const;
  void _whenExtendHead(const IlcDemon d)                 const;
  IlcIntervalVar _getLatestInOldHead()                   const;
  IlcIntervalVar _getEarliestNewInHead()                 const;
  void _whenExtendTail(const IlcDemon d)                 const;
  IlcIntervalVar _getLatestInOldTail()                   const;
  IlcIntervalVar _getEarliestNewInTail()                 const;
  void _whenNotSequenced(const IlcDemon d)               const;
  
  void _setPrevious(const IlcIntervalVar prev,
                    const IlcIntervalVar next)           const;
  void _setBefore(const IlcIntervalVar before,
                  const IlcIntervalVar after)            const;
  IlcBool _isInSearch()                                  const;
  IlcBool _isInPropagation()                             const;
  IlcBool _isInConstraintPosting()                       const;
  IlcBool _isBefore(const IlcIntervalVar v1, const IlcIntervalVar v2) const;
  IlcBool _isPrevious(const IlcIntervalVar v1, const IlcIntervalVar v2) const;
  IlcBool _isFirst(const IlcIntervalVar v) const;
  IlcBool _isLast(const IlcIntervalVar v) const;

public:
                                                                         
  enum Filter {
    
    Head,
    
    Tail,
    
    NotSequenced,
    
    CandidateHead,
    
    CandidateTail
  };
                                                                           
  class Iterator {
  private:
    IlcAny _graph;
    IlcAny _lnodes;
    IlcInt _ite;
    IlcInt _start; // semphore for -- (always valid interval if ok;
    IlcInt _end;  // semaphore for ++
    void initialize(const IlcIntervalSequenceVar seq,
                    IlcIntervalSequenceVar::Filter filter);
  public:
    
    Iterator(const IlcIntervalSequenceVar sequence,
             IlcIntervalSequenceVar::Filter filter,
             const IlcIntervalVar position = 0);
    ~Iterator() {}
    
    IlcBool ok() const { return (_ite != _end);}
    
    IlcIntervalVar operator*() const;
    
    Iterator& operator++();
   
    Iterator& operator--();
  };
  
  IlcGoal tryExtendHead(const IlcIntervalVar var, IlcAny label = 0) const;
  
  IlcGoal tryExtendTail(const IlcIntervalVar var, IlcAny label = 0) const;

  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getCPEngineI();
  }
                                                
  IlcBool isIn(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _isIn(var);
  }
                                                                         
  IlcInt getType(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _getType(var);
  }
                                                
  IlcBool isFixed() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _isFixed();
  }
                                                
  IlcBool isSequenced() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _isSequenced();
  }
                                                 
 IlcBool isPresent(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isPresent(var);
  }
                                                
  IlcBool isAbsent(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isAbsent(var);
  }
                                                  
  IlcBool isSequenced(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isSequenced(var);
    
  }
                                                
  IlcBool isInHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isInHead(var);
  }
                                                
  IlcBool isInTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isInTail(var);
  }
                                                
  IlcBool isCandidateHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isCandidateHead(var);
  }
                                                
   IlcBool isCandidateTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    return _isCandidateTail(var);
  }
                                                
  IlcIntervalVar getEarliestInHead() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getEarliestInHead();
  }
                                                
  IlcIntervalVar getEarliestInTail() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getEarliestInTail();
  }  
                                                
  IlcIntervalVar getLatestInHead() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getLatestInHead();
  }
                                                  
  IlcIntervalVar getLatestInTail() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getLatestInTail();
  }
                                                  
  IlcIntervalVar getLatestPresentInHead() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getLatestPresentInHead();
  }
                                                    
  IlcIntervalVar getLatestPresentInTail() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    return _getLatestPresentInTail();
  }
                                                    
  IlcIntervalVar getOneEarlierInHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    IlcCPOAssert(_isInHead(var), "IlcIntervalSequenceVar: interval variable not in Head");
    return _getOneEarlierInHead(var);
  }
                                                    
  IlcIntervalVar getOneLaterInHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    IlcCPOAssert(_isInHead(var), "IlcIntervalSequenceVar: interval variable not in Head");
    return _getOneLaterInHead(var);
  }
                                                    
   IlcIntervalVar getOneEarlierInTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle.");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    IlcCPOAssert(_isInTail(var), "IlcIntervalSequenceVar: interval variable not in Tail");
    return _getOneEarlierInTail(var);
  }
                                                    
  IlcIntervalVar getOneLaterInTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    IlcCPOAssert(_isInTail(var), "IlcIntervalSequenceVar: interval variable not in Tail");
    return _getOneLaterInTail(var);
  }
  
  
  IlcBool isEarlierInHead(const IlcIntervalVar earlier,
                          const IlcIntervalVar later)  const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(earlier), "IlcIntervalSequenceVar: earlier interval variable not in sequence");
    IlcCPOAssert(_isIn(later), "IlcIntervalSequenceVar: later interval variable not in sequence");
    IlcCPOAssert(_isInHead(earlier), "IlcIntervalSequenceVar: earlier interval variable not in head");
    IlcCPOAssert(_isInHead(later), "IlcIntervalSequenceVar: later interval variable not in head");
    return _isEarlierInTail(earlier, later);
  }

  
  IlcBool isEarlierInTail(const IlcIntervalVar earlier,
                          const IlcIntervalVar later) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(earlier), "IlcIntervalSequenceVar: earlier interval variable not in sequence");
    IlcCPOAssert(_isIn(later), "IlcIntervalSequenceVar: later interval variable not in sequence");
    IlcCPOAssert(_isInTail(later), "IlcIntervalSequenceVar: later interval variable not in tail");
    IlcCPOAssert(_isInTail(earlier), "IlcIntervalSequenceVar: earlier interval variable not in tail");
    return _isEarlierInHead(earlier, later);
  }
  
  void setPresent(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _setPresence(var, IloTrue);
  }
  
  void setAbsent(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _setPresence(var, IloFalse);
  }
  
  
  void extendHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _extendHead(var);
  }
                                                  
  void extendTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _extendTail(var);
  }
                                                    
  void removeCandidateHead(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _removeCandidateHead(var);
  }
                                                    
  void removeCandidateTail(const IlcIntervalVar var) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(var.getImpl(), "IlcIntervalSequenceVar: interval variable empty handle");
    IlcCPOAssert(_isIn(var), "IlcIntervalSequenceVar: interval variable not in sequence");
    _removeCandidateTail(var);
  }
                                                    
  void setPrevious(const IlcIntervalVar prev,
                   const IlcIntervalVar next) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(prev), "IlcIntervalSequenceVar: prev interval not in sequence");
    IlcCPOAssert(_isIn(next), "IlcIntervalSequenceVar: next interval not in sequence");
    _setPrevious(prev, next);
  }
                                                    
  void setBefore(const IlcIntervalVar before,
                 const IlcIntervalVar after) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(before.getImpl(), "IlcIntervalSequenceVar: before interval variable empty handle.");
    IlcCPOAssert(after.getImpl(), "IlcIntervalSequenceVar: before interval variable empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isIn(before), "IlcIntervalSequenceVar: before interval not in sequence");
    IlcCPOAssert(_isIn(after), "IlcIntervalSequenceVar: next interval not in sequence");
    _setBefore(before, after);
  }
    
  void whenPresence(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(d.getImpl(), "IlcIntervalSequenceVar: demon empty handle");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalSequenceVar: not in constraint posting");
    _whenPresence(d);
  }
    
  IlcIntervalVar getDeltaPresence() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalSequenceVar: not in constraint propagation");
    return _getDeltaPresence();
  }
    
  void whenExtendHead(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(d.getImpl(), "IlcIntervalSequenceVar: demon empty handle");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalSequenceVar: not in constraint posting");
    _whenExtendHead(d);
  }
    
  IlcIntervalVar getLatestInOldHead() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalSequenceVar: not in constraint propagation");
    return _getLatestInOldHead();
  }
    
  IlcIntervalVar getEarliestNewInHead() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalSequenceVar: not in constraint propagation");
    return _getEarliestNewInHead();
  }
    
  void whenExtendTail(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(d.getImpl(), "IlcIntervalSequenceVar: demon empty handle");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalSequenceVar: not in constraint posting");
    _whenExtendTail(d);
  }
    
  IlcIntervalVar getLatestInOldTail() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalSequenceVar: not in constraint propagation");
    return _getLatestInOldTail();
  }
    
  IlcIntervalVar getEarliestNewInTail() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(_isInPropagation(), "IlcIntervalSequenceVar: not in constraint propagation");
    return _getEarliestNewInTail();
  }

  
  void whenNotSequenced(const IlcDemon d) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isInSearch(), "IlcIntervalSequenceVar: not in search");
    IlcCPOAssert(d.getImpl(), "IlcIntervalSequenceVar: demon empty handle");
    IlcCPOAssert(_isInConstraintPosting(), "IlcIntervalSequenceVar: not in constraint posting");
    _whenNotSequenced(d);
  }
  
  IlcBool isBefore(const IlcIntervalVar pred,
                   const IlcIntervalVar succ) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(pred), "IlcIntervalSequenceVar: predecessor interval not in sequence");
    IlcCPOAssert(_isIn(succ), "IlcIntervalSequenceVar: successor interval not in sequence");
    return _isBefore(pred, succ);
  }
  
  IlcBool isPrevious(const IlcIntervalVar prev,
                     const IlcIntervalVar next) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(prev), "IlcIntervalSequenceVar: prev interval not in sequence");
    IlcCPOAssert(_isIn(next), "IlcIntervalSequenceVar: next interval not in sequence");
    return _isPrevious(prev, next);
  }
  
  IlcBool isFirst(const IlcIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(a), "IlcIntervalSequenceVar: interval not in sequence");
    return _isFirst(a);
  }
  
  IlcBool isLast(const IlcIntervalVar a) const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    IlcCPOAssert(_isIn(a), "IlcIntervalSequenceVar: interval not in sequence");
    return _isLast(a);
  }
};
  


ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntervalSequenceVar& exp);


  
class IlcIntervalSequenceVarArray {
  ILOCPHIDDENHANDLEMINI(IlcIntervalSequenceVarArray, IlcIntervalSequenceVarArrayI)
  ILCEXTENSIONMETHODSHDECL(IlcIntervalSequenceVarArray)
private:
  void _ctor(IlcCPEngineI* cp, const IloInt size);
  IlcCPEngineI* _getCPEngineI()              const;
  IlcInt _getSize()                   const;
  IlcIntervalSequenceVarArray _getCopy()      const;
  IlcIntervalSequenceVar& _get(IlcInt index)  const;
public:
  
  IlcIntervalSequenceVarArray(const IlcCPEngine cp, const IloInt size) {
    IlcCPOAssert(cp.getImpl(), "IlcIntervalSequenceVarArray: IlcCPEngine empty handle.");
    IlcCPOAssert(size >= 0, "IlcIntervalSequenceVarArray: strictly negative size value");
    _ctor(cp.getImpl(), size);
  }

  IlcCPEngineI* getCPEngineI() const { return _getCPEngineI(); }
                                                                         
  IlcCPEngine getCPEngine() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVar: empty handle.");
    return _getCPEngineI();
  }
  
  IloInt getSize() const {
     IlcCPOAssert(getImpl(), "IlcIntervalSequenceVarArray: empty handle.");
     return _getSize();
  }
  
  IlcIntervalSequenceVarArray getCopy() const {
    IlcCPOAssert(getImpl(), "IlcIntervalSequenceVarArray: empty handle");
    return _getCopy();
  }
  
  IlcIntervalSequenceVar& operator[] (IlcInt index) const {
    IlcCPOAssert(getImpl(), "IlcIntVarArray: empty handle");
    IlcCPOAssert(index >= 0 && index < _getSize(),
              "IlcIntervalSequenceVarArray: index out of range");
    return _get(index);
  }
};
  

ILOSTD(ostream)& operator<< (ILOSTD(ostream)& str, const IlcIntervalSequenceVarArray& exp);

//----------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(pop)
#endif

ILCGCCEXPORTOFF

#endif
