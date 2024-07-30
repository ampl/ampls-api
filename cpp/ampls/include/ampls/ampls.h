#ifndef ampls_H_INCLUDE_
#define ampls_H_INCLUDE_

#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <cstdarg>
#include <stdexcept>    // std::runtime_error
#include <cmath> // for std::nan
#include <memory> // std::unique_ptr
#include <iostream> // for operator overloading

// This declaration is used in the solver-specific implementations
// to import functions from the solver libraries
#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#else
#define ENTRYPOINT
#endif

namespace ampls
{
/**
Wrapper for all ampl solver exceptions
*/
class AMPLSolverException : public std::runtime_error
{
public:
  /**Constructor for exception with message*/
  AMPLSolverException(const char* msg) : std::runtime_error(msg) { }
  /**Constructor for exception with message*/
  AMPLSolverException(std::string& msg) : std::runtime_error(msg) { }
  /**Constructor for exception with format message (printf semantics)*/
  static AMPLSolverException format(const char* msg, ...)
  {
    char buffer[1000];
    va_list aptr;
    va_start(aptr, msg);
    vsprintf(buffer, msg, aptr);
    va_end(aptr);
    
    return AMPLSolverException(buffer);
  }
};


/**
* Stores a solver return value.
* Note that the strings are owned by the solver itself,
* no need to destroy them
*/
struct Variant
{
  /**Access the string reference*/
  const char* str; // type 0
  /**Access the integer value*/
  int integer;     // type 1
  /**Access the double value*/
  double dbl;      // type 2
  /**Stores the type of this variant*/
  int type;
  /** Create an empty Variant (type = -1) */
  Variant() : str(NULL), integer(0), dbl(0), type(-1) {}
  /** Create a string reference Variant (type 0), contains a reference 
  to a string, accessible via .str */
  Variant(const char* c) : str(c), integer(0), dbl(0), type(0) {}
  /** Create an int Variant (type 1), contains an integer
  accessible via .integer */
  explicit Variant(int v) : str(NULL), integer(v), dbl(0), type(1) {}
  /** Create a double Variant (type 2), contains a double
  accessible via .dbl */
  Variant(double v) : str(NULL), integer(0), dbl(v), type(2) {}

  friend std::ostream& operator<<(std::ostream& out, const Variant& v)
  {
    switch (v.type)
    {
    case 2:
      out << v.dbl;
      break;
    case 1:
      out << v.integer;
      break;
    case 0:
      out << v.str;
      break;
    case -1:
      out << "NULL";
      break;
    default:
      throw std::runtime_error("Wrong variant");
    }
    return out;
  }

};

// Forward declarations
class AMPLModel;
class GenericCallback;

char** generateArguments(const char* modelName);
void deleteParams(char** params);

struct VarType {
  enum Type {
    Continuous=0,
    Binary=1,
    Integer=2
  };
};

struct SolverAttributes {
  enum Attribs {
    /** Current solution's relative MIP gap */
    DBL_RelMIPGap,
    /** Current objective bound */
    DBL_CurrentObjBound,
    /** Number of integer variables in the presolved problem (if applicable) */
    INT_NumIntegerVars,

  };

};
struct SolverParams
{
  /**
  * Enumerate generic solver control parameters
  * (e.g. parameters that are mapped to then solver-dependent controls).
  * In case a not mapped control is required, refer to the solver-specific
  * API.
  */
  enum SolverParameters
  {
    /** Stopping relative MIP gap */
    DBL_MIPGap,
    /** Stopping time limit */
    DBL_TimeLimit,
    /** Stopping number of solutions limit (for MIP) */
    INT_SolutionLimit,
    /** Algorithm for LP problems */
    INT_LP_Algorithm
  };
};

struct LPAlgorithms
{
  /**
  Algorithm to use when solving continous problems.
  Use solver-specific API to set other methods.
  */ 
  enum LPAlgorithm
  {
    /** Let the solver decide*/
    Auto=0,
    /** Primal simplex */
    PrimalSimplex=1,
    /** Dual simplex */
    DualSimplex=2,
    /** Barrier algorithm */
    Barrier=3
  };
};

struct Where
{
  /**
  * These values (generic) identify where in the solution
  * process a callback has been called; to get this generic value
  * call GenericCallback::getAMPLType().
  * Not all solvers "where" are mapped to these values; in case
  * the callback is called with a not-mapped "where" parameter,
  * refer to the solver-specific functionality.
  */
  enum CBWhere
  {
    /** When the solver wants to print a message, obtain it via GenericCallback::getMessage.*/
    MSG = 0,
    /** Presolve phase */
    PRESOLVE = 1,
    /** Executing simplex */
    LPSOLVE = 2,
    /** Exploring a MIP node*/
    MIPNODE = 3,
    /** Found a new MIP solution*/
    MIPSOL = 4,
    /** Executing MIP algorithm*/
    MIP = 5,
    /** Not mapped, refer to the specific user documentation*/
    NOTMAPPED = 10
  };
};

struct CanDo
{
  enum Functionality
  {
    /** Can a solution be imported at this stage */
    IMPORT_SOLUTION = 1,
    /** Can get current value of the relaxation solution */
    GET_LP_SOLUTION = 2,
    /** Can get current value of the integer solution */
    GET_MIP_SOLUTION = 4,
    /** Add lazy constraint */
    ADD_LAZY_CONSTRAINT = 8,
    /** Add user cut */
    ADD_USER_CUT = 16
  };
};

struct Value
{
  /**
  * Which values (generic) to get in a callback; just a subset of all the
  * value types are mapped here.
  * In case a not mapped value is required, refer to the solver-specific
  * API.
  */
  enum  CBValue {
    /** Objective value*/
    OBJ = 0,
    /** Presolve: number of deleted columns*/
    PRE_DELCOLS = 1,
    /** Presolve: number of deleted rows*/
    PRE_DELROWS = 2,
    /** Presolve: number of coefficients changed*/
    PRE_COEFFCHANGED = 3,
    /** Number of algorithm iterations*/
    ITERATIONS = 4,
    /** Time since solution start*/
    RUNTIME = 5,
    /** Number of MIP nodes*/
    MIP_NODES = 6,
    /** Current relative MIP gap*/
    MIP_RELATIVEGAP = 7,
    /** Current best bound on objective */
    MIP_OBJBOUND=8,
    /* Relaxed solution - only in MIP node */
    MIP_SOL_RELAXED =9,
    /* Number of rows in terms of the original problem */
    N_ROWS = 10,
    /* Number of columns in terms of the original problem */
    N_COLS = 11
  };
};

struct CutDirection {
  /** Direction of a cut to be added*/
  enum Direction {
    /** = Equal*/
    EQ = 0,
    /** >= Greater or equal*/
    GE = 1,
    /** <= Less or equal*/
    LE = 2
  };
  static std::string toString(Direction dir) {
    if (dir == Direction::EQ)
      return "=";
    if (dir == Direction::GE)
      return ">=";
    if (dir == Direction::LE)
      return "<=";
    throw std::invalid_argument("Unexpected cut direction value");
  }
};

struct Status
{
  /**
  * Solution status (generic)
  */
  enum SolStatus {
    /**Not known*/
    UNKNOWN,
    /**Optimal*/
    OPTIMAL,
    /**Infeasible problem*/
    INFEASIBLE,
    /**Unbounded problem*/
    UNBOUNDED,
    /**Hit an iterations limit*/
    LIMIT_ITERATION,
    /**Hit a nodes limit*/
    LIMIT_NODE,
    /**Hit a time limit*/
    LIMIT_TIME,
    /**Hit a number of solutionslimit*/
    LIMIT_SOLUTION,
    /**Interrupted by the user*/
    INTERRUPTED,
    NOTMAPPED
  };
};
} // namespace ampls

// Mutex utility
#if defined(_MSC_VER)
#if _MSC_VER >=1700 // __cplusplus is not supported on windows compiler 
// unless we specify the /Zc compiler setting, so we resort to the _MSC_VER
// std::mutex was first implemented in VC 2012.
#define USECPP11MUTEX
#else
#define USEWINLOCK
#include <windows.h>
typedef SRWLOCK MUTEXIMPL;
#endif  
#else
#if __cplusplus >= 201103L && __STDC_HOSTED__ == 1 && __STDCPP_THREADS__ == 1
#define USECPP11MUTEX
#else
#define USEPTHREADMUTEX
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
typedef pthread_mutex_t  MUTEXIMPL;
#endif
#endif
#ifdef USECPP11MUTEX
#include <mutex>
typedef std::mutex MUTEXIMPL;
#endif

namespace ampls{
  namespace impl {
    namespace mp {
      // *********** For MP-solvers, taken from ampls-c-api.h **********
      /// An AMPLS solver instance
      typedef struct AMPLS_MP_Solver_T {
        /// AMPLS internal info
        void* internal_info_;
        /// Extra info, managed by the specific solver
        void* solver_info_;
        /// User info, free to assign
        void* user_info_;
      } AMPLS_MP_Solver;

      typedef struct AMPLSCOption_T {
        const char* name;
        const char* description;
        int type; // 0=int, 1=double, 2=string
      } AMPLS_C_Option;
      extern "C" {
        // Imported from the ampls driver library, declared in ampls-c-api.h
        ENTRYPOINT void AMPLSReportResults(void* slv, const char* solFileName);
        ENTRYPOINT const AMPLS_C_Option* AMPLSGetOptions(void* slv);
        ENTRYPOINT int AMPLSSetIntOption(void* slv, const char* name, int value);
        ENTRYPOINT int AMPLSGetIntOption(void* slv, const char* name, int* value);
        ENTRYPOINT int AMPLSSetDblOption(void* slv, const char* name, double value);
        ENTRYPOINT int AMPLSGetDblOption(void* slv, const char* name, double* value);
        ENTRYPOINT int AMPLSSetStrOption(void* slv, const char* name, const char* value);
        ENTRYPOINT int AMPLSGetStrOption(void* slv, const char* name, const char** value);
        ENTRYPOINT int AMPLSLoadNLModel(AMPLS_MP_Solver* slv, const char* nl_filename, char** options);
        ENTRYPOINT void AMPLSReadExtras(AMPLS_MP_Solver* slv);
        ENTRYPOINT void AMPLSSolve(AMPLS_MP_Solver* slv);
        ENTRYPOINT const char* const* AMPLSGetMessages(AMPLS_MP_Solver* slv);
      }
    } // namespace mp
    // ******* end MP ******* 

    class Records; // forward for toAMPLString
    template<typename ... Args>
    std::string string_format(const std::string& format, Args ... args)
    {
      int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
      if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
      auto size = static_cast<size_t>(size_s);
      std::vector<char> buf(size);
      std::snprintf(buf.data(), size, format.c_str(), args ...);
      return std::string(buf.data(), buf.data() + size - 1); // We don't want the '\0' inside
    }
    double calculateRelMIPGAP(double obj, double bbound);
  }

  class Entity
  {
    friend class impl::Records;
    
    std::vector<int> indices_;
    std::vector<double> coeffs_;
    int solverIndex_;
    double value_;
    bool exportedToAMPL_;
  protected:
    std::string name_;
    AMPLModel* parent_;
  public:
    Entity(AMPLModel* parent, std::string name, const std::vector<int> &indices,
      const std::vector<double> &coeffs) : name_(name), indices_(indices), coeffs_(coeffs), 
      solverIndex_(-1), value_(nanl("")), exportedToAMPL_(false), parent_(parent)
    {}

    Entity(AMPLModel* parent, const char* name, int nnz, const int* indices,
      const double* coeffs) : solverIndex_(-1),
      value_(nanl("")), exportedToAMPL_(false), parent_(parent) {
      if (name != NULL)
        name_ = std::string(name);
      indices_.insert(indices_.begin(), &indices[0], &indices[nnz]);
      coeffs_.insert(coeffs_.begin(), &coeffs[0], &coeffs[nnz]);
    }

    std::string name() const { return name_; }

    std::vector<int> indices() const { return indices_; }
    std::vector<double> coeffs() const { return coeffs_; }

    void solverIndex(int index) { solverIndex_ = index; }
    int solverIndex()const { return solverIndex_; }

    void value(double v) { value_ = v; }
    double value() const { return value_; }
    virtual std::string toAMPLString(const std::map<int, std::string>& varMap,
      const std::map<int, std::string>& consMap,
      const impl::Records& records) const = 0;
    virtual bool operator==(const Entity& other) const {
      if (indices_ != other.indices_) return false;
      if (coeffs_ != other.coeffs_) return false;
      return true;
    }

   
  };
  
  class Constraint : public Entity
  {
    friend class AMPLModel;
    ampls::CutDirection::Direction sense_;
    double rhs_;
    void assignName();
    
  public:
    Constraint(AMPLModel* parent, std::string name, const std::vector<int> &indices,
      const std::vector<double> &coeffs, CutDirection::Direction sense, double rhs) :
      Entity(parent, name, indices, coeffs), sense_(sense), rhs_(rhs) {
      if (name.size() == 0)
        assignName();
    }

    Constraint(AMPLModel* parent, const char* name, int nnz, const int* indices,
      const double* coeffs, ampls::CutDirection::Direction sense, double rhs) :
      Entity(parent, name, nnz, indices, coeffs), sense_(sense), rhs_(rhs) {
      if ((name == NULL) || strlen(name) == 0)
        assignName();
    }

    ampls::CutDirection::Direction sense() const { return sense_; }
    double rhs() const { return rhs_; }
    std::string toAMPLString(const std::map<int, std::string>& varMap, 
      const std::map<int, std::string>& consMap,
      const impl::Records& records) const;
    std::string toString(const std::map<int, std::string>& varMap = {});
    bool operator==(const Constraint& other) const {
      if (!Entity::operator==(other)) return false;
      return rhs_ == other.rhs_;
    }
   



  };
  
  class Variable : public Entity
  {
    friend class AMPLModel;
    void assignName();
  public:
    Variable(AMPLModel* parent, std::string name, const std::vector<int> &indices,
      const std::vector<double> &coeffs, double lb, double ub,
      double objCoefficient, VarType::Type type) :
      Entity(parent, name, indices, coeffs), ub_(ub), lb_(lb), obj_(objCoefficient), type_(type) {
      if (name.size() == 0)
        assignName();
    }

    Variable(AMPLModel* parent, const char* name, int nnz, const int* indices,
      const double* coeffs, double lb, double ub,
      double objCoefficient, VarType::Type type) :
      Entity(parent, name, nnz, indices, coeffs), ub_(ub), lb_(lb), obj_(objCoefficient), type_(type) {
      if (name == NULL)
        assignName();
    }

    double ub_;
    double lb_;
    double obj_;
    VarType::Type type_;
    std::string toAMPLString(const std::map<int, std::string>& map, 
      const std::map<int, std::string>& consMap,
      const impl::Records& records) const;
  };
  
namespace impl
{
  class Records
  {
    friend class ampls::AMPLModel;
    friend class ampls::Constraint;
    friend class ampls::Variable;

    AMPLModel* parent_;
    std::vector<Variable> vars_;
    std::vector<Constraint> cons_;
    // The following is to retain the order in which the entities are added,
    // to be replicated when adding them to the AMPL model
    std::vector<std::pair<bool, int>> entities_;

    Records() : parent_(nullptr) { }
  public:
    Records(AMPLModel &a) : parent_(&a) {
    }

    Records(Records&& r) noexcept : 
      parent_(r.parent_), vars_(std::move(r.vars_)),  
    cons_(std::move(r.cons_)), entities_(std::move(r.entities_)) {
    }

    Records(AMPLModel& a, const Records& other): parent_(&a),
      vars_(other.vars_), cons_(other.cons_),
      entities_(other.entities_) { }

    Records& operator=(const Records& other)
    {
      if (this != &other) {
        parent_ = other.parent_;
        vars_ = other.vars_;
        cons_ = other.cons_;
        entities_ = other.entities_;
      }
      return *this;
    }
    Records& operator=(Records&& other) 
    {
      if (this != &other) {
        parent_ = std::move(other.parent_);
        vars_ = std::move(other.vars_);
        cons_ = std::move(other.cons_);
        entities_ = std::move(other.entities_);
        other.parent_ = nullptr;
      }
      return *this;
    }


    std::string getRecordedEntities(bool exportToAMPL = false);

    void addVariable(const Variable& v)
    {
      vars_.push_back(v);

      entities_.push_back({ 1, vars_.size() - 1 });
    }

    void addConstraint(const Constraint& c)
    {
      cons_.push_back(c);
      entities_.push_back({ 0, cons_.size() - 1 });
    }

    void getVarIndices(int& min, int& max)
    {
      min = vars_.front().solverIndex();
      max = vars_.back().solverIndex();
    }
    void getConsIndices(int &min, int &max)
    {
      min = cons_.front().solverIndex();
      max = cons_.back().solverIndex();
    }

    std::size_t getNumConstraints() {
      return cons_.size();
    }
    std::size_t getNumVariables() {
      return vars_.size();
    }

  };

class AMPLMutex {
public:
  inline AMPLMutex();
  inline ~AMPLMutex();
  inline void Lock();    
  inline void Unlock();
private:
  MUTEXIMPL mutex_;
  // Catch the error of writing Mutex when intending MutexLock.
      AMPLMutex(AMPLMutex*){}
      AMPLMutex(const AMPLMutex&) {}
      AMPLMutex& operator=(const AMPLMutex& m);
    };
#ifdef USECPP11MUTEX
    AMPLMutex::AMPLMutex() { }
    AMPLMutex::~AMPLMutex() { }
    void AMPLMutex::Lock() { mutex_.lock(); }
    void AMPLMutex::Unlock() { mutex_.unlock(); }
#elif defined(USEWINLOCK)
    AMPLMutex::AMPLMutex() { InitializeSRWLock(&mutex_); }
    AMPLMutex::~AMPLMutex() { }
    void AMPLMutex::Lock() { AcquireSRWLockExclusive(&mutex_); }
    void AMPLMutex::Unlock() { ReleaseSRWLockExclusive(&mutex_); }
#elif defined(USEPTHREADMUTEX)
    AMPLMutex::AMPLMutex() { pthread_mutex_init(&mutex_, NULL); }
    AMPLMutex::~AMPLMutex() { pthread_mutex_destroy(&mutex_); }
    void AMPLMutex::Lock() { pthread_mutex_lock(&mutex_); }
    void AMPLMutex::Unlock() { pthread_mutex_unlock(&mutex_); }
#undef SAFE_PTHREAD

#endif

/**
* Infrastructure, should not be used directly.
* Base class for all callback objects, solvers-specific and/or generic.
*/
    class BaseCallback
    {
      friend class ampls::AMPLModel;
      friend class ampls::GenericCallback;
      bool cutDebug_;
      bool cutDebugIntCoefficients_;
      bool cutDebugPrintVarNames_;
    protected:
      AMPLModel* model_;
      int where_;
      virtual int doAddCut(const ampls::Constraint& c, int type, void* additionalParams = nullptr) = 0;

      ampls::Constraint callAddCut(std::vector<std::string>& vars,
        const double* coeffs, CutDirection::Direction direction, double rhs,
        int type, void* additionalParams = nullptr);

      ampls::Constraint callDoAddCut(int length, const int* indices,
        const double* coeffs, CutDirection::Direction direction, double rhs,
        int type, void* additionalParams = nullptr);

      void printCut(const ampls::Constraint& c, bool intCoeffs = false,
        bool varNames = false)
      {
        std::string sense;
        switch (c.sense())
        {
        case CutDirection::EQ:
          sense = "= ";
          break;
        case CutDirection::GE:
          sense = ">=";
          break;
        case CutDirection::LE:
          sense = "<=";
          break;
        default:
          throw AMPLSolverException("Unexpected cut direction");
        }
        std::size_t nvars = c.indices().size();
        if (varNames)
        {
          std::map<int, std::string> imap = getVarMapInverse();
          if (intCoeffs)
          {
            for (std::size_t i = 0; i < nvars; ++i) {
              printf("%d*%s", (int)c.coeffs()[i], imap[c.indices()[i]].c_str());
              if (i < nvars - 1)
                printf(" + ");
            }
          }
          else
          {
            for (std::size_t i = 0; i < nvars; ++i) {
              printf("%f*%s", c.coeffs()[i], imap[c.indices()[i]].c_str());
              if (i < nvars - 1)
                printf(" + ");
            }
          }
        }
        else
        {
          if (intCoeffs)
          {
            for (int i = 0; i < nvars; ++i) {
              printf("%d*x[%d]", (int)c.coeffs()[i], c.indices()[i]);
              if (i < nvars - 1)
                printf(" + ");
            }
          }
          else
          {
            for (int i = 0; i < nvars; ++i) {
              printf("%f*x[%d]", c.coeffs()[i], c.indices()[i]);
              if (i < nvars - 1)
                printf(" + ");
            }
          }
        }
        printf(" %s %f\n", sense.c_str(), c.rhs());

      }
      int currentCapabilities_;

      virtual Variant getValueImpl(Value::CBValue v) {
        throw ampls::AMPLSolverException("Not implemented in base class");
      }
      virtual Where::CBWhere getAMPLWhereImpl() {
        throw ampls::AMPLSolverException("Not implemented in base class");
      }
public:
  

  // Check if the specified functionality is available at this stage
  virtual bool canDo(CanDo::Functionality f) {
    return currentCapabilities_ & (int)f;
  }

  void record(const ampls::Variable& v);
  void record(const ampls::Constraint& c);

  ampls::Variable addVariable(const std::vector<int>& cons,
    const std::vector<double>& coefficients,
    double lb, double ub, double objCoefficient,
    VarType::Type type, bool relaxed = false, const char* name = NULL) {
    return addVariable(cons.size(), cons.data(), coefficients.data(),
      lb, ub, objCoefficient, type, relaxed, name);
  }


  ampls::Variable addVariable(int nnz, const int* cons,
    const double* coefficients, double lb, double ub, double objCoefficient,
    VarType::Type type, bool relaxed = false, const char* name = NULL);

  ampls::Variable addVariable(double lb, double ub,
    VarType::Type type, bool relaxed = false, const char* name = NULL) {
    return addVariable(0, NULL, NULL, lb, ub, 0, type, name);
  }

  void setDebugCuts(bool cutDebug, bool cutDebugIntCoefficients, bool cutDebugPrintVarNames)
  {
    cutDebug_ = cutDebug;
    cutDebugIntCoefficients_ = cutDebugIntCoefficients;
    cutDebugPrintVarNames_ = cutDebugPrintVarNames;
  }

  BaseCallback() :cutDebug_(false), cutDebugIntCoefficients_(false),
    cutDebugPrintVarNames_(false), model_(NULL), where_(0),
  currentCapabilities_(0){}
  /** Function to override, called periodically by the optimizer*/
  virtual int run() = 0;
  /** Get the map AMPLEntityName -> SolverVarIndex*/
  std::map<std::string, int>& getVarMap();
  /** Get the map SolverVarIndex -> AMPLEntityName*/
  std::map<int, std::string>& getVarMapInverse();

  virtual ~BaseCallback() {};

  /** Add a user cut using AMPL variables names.
  * @param vars Vector of AMPL variable names
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  ampls::Constraint addCut(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 0);
  }
    /** Add a lazy constraint using AMPL variables names.
  * @param vars Vector of AMPL variable names
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  ampls::Constraint addLazy(std::vector<std::string> vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callAddCut(vars, coeffs, direction, rhs, 1);
  }

  /** Add a user cut using solver indics
  * @param nvars Number of variables in the cut (length of *vars)
  * @param vars Vector of variable indices (in the solvers representation)
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  ampls::Constraint addCutIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callDoAddCut(nvars, vars, coeffs, direction, rhs, 0);
  }
  /** Add a lazy constraint using solver indics
  * @param nvars Number of variables in the cut (length of *vars)
  * @param vars Vector of variable indices (in the solvers representation)
  * @param coeffs Vector of cut coefficients 
  * @param direction Direction of the constraint ampls::CBDirection::Direction
  * @param rhs Right hand side value
  */
  ampls::Constraint addLazyIndices(int nvars, const int* vars,
    const double* coeffs, CutDirection::Direction direction, double rhs)
  {
    return callDoAddCut(nvars, vars, coeffs, direction, rhs, 1);
  }

  virtual int setHeuristicSolution(int nvars, const int* indices, const double* values) = 0;
    // Gurobi: where: GRB_CB_MIPNODE, GRBcbsolution https://www.gurobi.com/documentation/9.1/refman/c_cbsolution.html
    // CPLEX:  CPXsetheuristiccallbackfunc, http://www-eio.upc.es/lceio/manuals/cplex-11/html/usrcplex/advMIPcontrolInterface5.html#:~:text=Once%20this%20routine%20has%20been,the%20best%20available%20integer%20solution).
    // and https://github.com/renvieir/ioc/blob/master/cplex/examples/src/c/admipex2.c
    // Xpress: https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/XPRSaddmipsol.html

  
  /** Get the current solution vector */
  virtual std::vector<double> getSolutionVector();
  /** Get the current solution */
  virtual int getSolution(int len, double* sol) = 0;
  /** Get the current objective value */
  virtual double getObj() = 0;
  /** Get an iteger representing where in the solution process the callback has been called.
     NOTE: this is expressed using the solver's own (not generic) values
  */
  virtual int getWhere() { 
    return where_; }
  /** Get a textual representation of where in the solution process the callback has been called.
   * NOTE: this is expressed using the solver's own (not generic) values
   */
  virtual const char* getWhereString() = 0;
  
  /** Get the message from the solver (available only for specific values of getWhere) */
  virtual const char* getMessage() = 0;

  /** Get where in the solution process the callback has been called (generic) */
  virtual Where::CBWhere getAMPLWhere() { return getAMPLWhereImpl();}
  
  /** Get a (generic) value */
  virtual Variant getValue(Value::CBValue v) { return getValueImpl(v); }
  /** Get a (generic) array */
  virtual std::vector<double> getValueArray(Value::CBValue v) = 0;
};

/**
* Infrastructure, should not be used directly.
* Base class for the solver drivers.
*/
template<class T> class SolverDriver
{
  AMPLMutex loadMutex;
protected:
  virtual T loadModelImpl(char** args, const char** options) = 0;
  /**
  Not to be used directly; to be called in the solver driver `loadModel` function implementations to provide
  common functionalities like mutex and exception handling
  */
  T loadModelGeneric(const char* modelName, const char** options)
  {
    FILE* f = fopen(modelName, "rb");
    if (!f)
      throw ampls::AMPLSolverException::format("Could not find file: %s, make sure it is exported from AMPL.\n"
        "Note that if AMPL presolver detects an unfeasibility, it will not export it. To overcome this, set the\n"
        "option 'presolve' to 0 before exporting the model.", modelName);
    else
      fclose(f);

    char** args = NULL;
    try {
      args = generateArguments(modelName);
      loadMutex.Lock();
      T mod = loadModelImpl(args, options);
      loadMutex.Unlock();
      deleteParams(args);
      return mod;
    }
    catch (const ampls::AMPLSolverException& e) {
      loadMutex.Unlock();
      deleteParams(args);
      throw e;
    }
    catch (const std::runtime_error &e) {
      loadMutex.Unlock();
      deleteParams(args);
      throw e;
    }
    catch (const std::exception& e) {
      loadMutex.Unlock();
      deleteParams(args);
      throw e;
    }
  }
public:
  /**
  * Load a model from an NL file.
  * Mappings between solver row and column numbers and AMPL names are
  * available only if the row and col files have been generated as well,
  * by means of the ampl option `option auxfiles cr;` before writing the NL file.
  */
  
  T loadModel(const char* modelName, const char** options=nullptr) {
    return loadModelGeneric(modelName, options);
  }
  T loadModel(const char* modelName, std::vector<std::string> options) {
    std::vector<const char*> pointers;
    pointers.reserve(options.size() + 1);
    for (auto &s : options)
      pointers.push_back(s.data());
    pointers.push_back(nullptr);
    return loadModelGeneric(modelName, pointers.data());
  }


  SolverDriver() {}
  ~SolverDriver() {}
};
} // namespace impl

/**
* Base abstract class for generic callbacks, inherit from this class to implement
* a generic callback.
* Provides all mapping between solver-specific and generic values.
* To implement a callback, you should implement the run() method and
* set it via AMPLModel::setCallback() before starting the solution
* process via AMPLModel::optimize().
* Depending on where in the solution process the callback is called from, 
* you can obtain various information about the progress of the optimization 
* and can modify the behaviour of the solver.
*/
class GenericCallback : public impl::BaseCallback
{
  friend class AMPLModel;

private:
  std::unique_ptr<impl::BaseCallback> impl_;

protected:
  int doAddCut(const ampls::Constraint& c, int type, 
      void* additionalParams = nullptr)
  {
    return impl_->doAddCut(c, type, additionalParams);
  }
  Variant getValueImpl(Value::CBValue v)
  {
    return impl_->getValueImpl(v);
  }
public:

  using BaseCallback::addLazy;
  using BaseCallback::addCut;

  /** Get the current solution vector   */
  int getSolution(int len, double *sol)
  {
    return impl_->getSolution(len, sol);
  }
  std::vector<double> getSolutionVector()
  {
    return impl_-> getSolutionVector();
  }
  std::vector<double>  getValueArray(Value::CBValue v)
  {
    return impl_->getValueArray(v);
  }
  int setHeuristicSolution(int nvars, const int* indices, const double* values)
  {
    return impl_->setHeuristicSolution(nvars, indices, values);
  }
  /** Get the current objective value */
  double getObj()
  {
    return impl_->getObj();
  }
  /** Get an iteger representing where in the solution process the callback has been called.
   * NOTE: this is expressed using the solver's own (not generic) values
  */
  int getWhere()
  {
    return impl_->getWhere();
  }
  /** Get where in the solution process the callback has been called (generic) */
  Where::CBWhere getAMPLWhere()
  {
    return impl_->getAMPLWhere();
  }
  /** Get a textual representation of the current solver status*/
  const char *getWhereString()
  {
    return impl_->getWhereString();
  }
  /** Get the message that was being printed (if where == msg) */
  const char *getMessage()
  {
    return impl_->getMessage();
  }
  /** Get a value from the solver */
  Variant getValue(Value::CBValue v)
  {
    return impl_->getValue(v);
  }
  bool canDo(CanDo::Functionality f) {
    return impl_->canDo(f);
  }

};

// Non owning struct conveying option descriptions
struct Option {
  const char* name_;
  const char* description_;

  enum Type {
    INT,
    BOOL,
    DOUBLE,
    STRING,
    UNKNOWN
  };
  Type type_;
  Option() : name_(nullptr), description_(nullptr), type_(UNKNOWN) {}
  Option(const char* name, const char* description, int type) : name_(name),
  description_(description), type_((Type)type) { }
  const char* name() { return name_; }
  const char* description() { return description_; }
  Type type() { return type_; }
  const char* typeStr() { 
    switch (type())
    {
    case INT:
      return "int";
    case BOOL:
      return "bool";
    case DOUBLE:
      return "double";
    case STRING:
      return "string";
    case UNKNOWN:
      return "unknown";
    default:
      throw std::runtime_error("Should not see this");
    }
  }
  std::string toString() {
    return impl::string_format("%s (%s)\n%s", name(), typeStr(), description());
  }
};


/**
* Store an in-memory representation of an AMPL model, which 
* can be constructed by loading it from an NL file using the `loadModel` function
* available in a solver driver (i.e. CPLEXDrv::loadModel(), 
* GurobiDrv::loadModel() or XPRESSDrv::loadModel()).
* It also contains two-way mappings between solver column and row numbers and
* AMPL entity names.
*/
class AMPLModel
{
  friend std::map<std::string, int>& impl::BaseCallback::getVarMap();
  friend std::map<int, std::string>& impl::BaseCallback::getVarMapInverse();
  friend std::string impl::Records::getRecordedEntities(bool);
  friend void ampls::Constraint::assignName();
  friend void ampls::Variable::assignName();


  std::map<int, std::string> varMapInverse_;
  std::map<std::string, int> varMap_;
  /*
  Create a cache of the names to indices maps, to be used
  in subsequent calls to a callback
  */
  void getVarMapsInternal()
  {
    if (varMap_.size() == 0)
      varMap_ = getVarMapFiltered(NULL);
    if (varMapInverse_.size() == 0)
      varMapInverse_ = getVarMapInverse();
  }

  int numConsAdded() {
    return static_cast<int>(records_.cons_.size()+1);
  }
  int numVarsAdded() {
    return static_cast<int>(records_.vars_.size()+1);
  }


protected:
  std::string fileName_;
  impl::Records records_;

  AMPLModel() : records_(*this) {}

  AMPLModel(const AMPLModel& other) : fileName_(other.fileName_),
    records_(*this, other.records_),
    varMap_ (other.varMap_),
    varMapInverse_(other.varMapInverse_) {
  }

  AMPLModel(AMPLModel&& other) noexcept:
    fileName_(std::move(other.fileName_)),
    records_(std::move(other.records_)),
    varMap_(std::move(other.varMap_)),
    varMapInverse_(std::move(other.varMapInverse_)) {
    records_.parent_ = this;
    ;
  }

  AMPLModel& operator=(AMPLModel& other) {
    if (this != &other)
    {
      fileName_ = other.fileName_;
      varMap_ = other.varMap_;
      varMapInverse_ = other.varMapInverse_;
      records_ = impl::Records(*this, other.records_);
    }
    return *this;
  }

  AMPLModel& operator=(AMPLModel&& other) noexcept {
    if (this != &other)
    {
      std::swap(fileName_, other.fileName_);
      std::swap(varMap_, other.varMap_);
      std::swap(varMapInverse_, other.varMapInverse_);
      std::swap(records_, other.records_);
      records_.parent_ = this;
    }
    return *this;
  }
  void resetVarMapInternal()
  {
    // Clear the internally cached maps
    varMap_.clear();
    varMapInverse_.clear();
  }

  virtual int setCallbackDerived(impl::BaseCallback* callback) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual impl::BaseCallback *createCallbackImplDerived(GenericCallback *callback)   {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual void writeSolImpl(const char* solFileName) {
    throw AMPLSolverException("Not implemented in base class!");
  }
 
  virtual std::vector<double> getConstraintsValueImpl(int offset, int length) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual std::vector<double> getVarsValueImpl(int offset, int length) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual int addConstraintImpl(const char* name, int numnz, const int vars[], const double coefficients[],
    ampls::CutDirection::Direction sense, double rhs) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual int addVariableImpl(const char* name, int numnz, const int cons[], const double coefficients[],
    double lb, double ub, double objcoeff, ampls::VarType::Type type) {
    throw AMPLSolverException("Not implemented in base class!");
  }
public:
  /// <summary>
  /// Load a model from an NL file
  /// </summary>
  /// <typeparam name="T">Concrete class (e.g. CPLEXModel, GurobiModel, ...) to create</typeparam>
  /// <param name="nlfile">NL file path</param>
  /// <param name="options">NULL terminated array of startup options</param>
  /// <returns></returns>
  template<class T> static T load(const char* nlfile, const char** options=nullptr) { 
    typename T::Driver d;
    return d.loadModel(nlfile, options);
  }

  virtual const char* driver() { throw AMPLSolverException("Not implemented in base class!"); }
  std::string getRecordedEntities(bool exportToAMPL = true) {
    return records_.getRecordedEntities(exportToAMPL);
  }


  ampls::Constraint addConstraint(const std::vector<int> &vars,
    const std::vector<double> &coefficients, ampls::CutDirection::Direction sense, 
    double rhs, const char* name) {
    Constraint c = Constraint(this, name, vars, coefficients, sense, rhs);
    int index = addConstraintImpl(name, vars.size(), vars.data(), coefficients.data(), sense, rhs);
    c.solverIndex(index);
    return c;
  }

  ampls::Constraint addConstraint(int nnz, const int* vars,
    const double* coefficients, ampls::CutDirection::Direction sense, double rhs) {
    return addConstraint(nnz, vars, coefficients, sense, rhs, "");
  }
  ampls::Constraint addConstraint(int nnz, const int* vars,
    const double* coefficients, ampls::CutDirection::Direction sense, double rhs, const std::string& name) {
    Constraint c = Constraint(this, name.c_str(), nnz, vars, coefficients, sense, rhs);
    int index = addConstraintImpl(name.c_str(), nnz, vars, coefficients, sense, rhs);
    c.solverIndex(index);
    return c;
  }
  void record(const ampls::Constraint &c) {
    records_.addConstraint(c);
  }
  void record(const ampls::Variable& v) {
    records_.addVariable(v);
  }

  virtual double infinity() {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual double negInfinity() {
    return -infinity();
  }

  ampls::Variable addVariable(const std::vector<int>& cons,
    const std::vector<double>& coefficients,
    double lb, double ub, double objCoefficient,
    VarType::Type type, bool relaxed = false,
    const char* name = NULL
    ) {
    return addVariable(cons.size(), cons.data(), coefficients.data(),
      lb, ub, objCoefficient, type, relaxed, name);
  }

  ampls::Variable addVariable(double lb, double ub,
    VarType::Type type, bool relaxed = false, const char* name = NULL) {
    return addVariable(0, NULL, NULL, lb, ub, 0, type, relaxed, name);
  }

  ampls::Variable addVariable(int nnz, const int* cons,
      const double* coefficients, double lb, double ub, double objCoefficient,
      VarType::Type type, bool relaxed = false, const char* name = NULL) {
    Variable v = Variable(this, name, nnz, cons, coefficients, lb, ub, objCoefficient, type);
    if (relaxed) type = VarType::Continuous;
    int index = addVariableImpl(name, nnz, cons, coefficients, lb, ub, objCoefficient, type);
    v.solverIndex(index);
    return v;
  }


  /**
  Get the name of the NL file from which the model has been loaded from
  */
  std::string getFileName() {
    return fileName_;
  }
  /**
  Get the map from variable index in the solver interface to AMPL variable instance name
  */
  std::map<int, std::string> getVarMapInverse();
  /**
  Get the map from constraint index in the solver interface to AMPL variable instance name
  */
  std::map<int, std::string> getConMapInverse();

  /**
  Get the map from variable name to index in the solver interface
  */
  std::map<std::string, int> getVarMap()
  {
    return getVarMapFiltered(NULL);
  }

  /**
  Get the map from constraint name to index in the solver interface
  */
  std::map<std::string, int> getConMap()
  {
    return getConMapFiltered(NULL);
  }

  /**
  Return the variable map filtered by the variable name, to avoid
  getting the whole (possibly large) map
  @param beginWith Prefix to be matched
  */
  std::map<std::string, int> getVarMapFiltered(const char *beginWith);

  /**
  Return the constraint map filtered by the constraint name, to avoid
  getting the whole (possibly large) map
  @param beginWith Prefix to be matched
  */
  std::map<std::string, int> getConMapFiltered(const char* beginWith);

  /**
  Set a generic callback to be called during optimization. This function is
  automatically dispatched when (and only when) assigning an ampls::GenericCallback,
  as it needs a special treatment to automatically create the solver-specific wrapper
  @param callback The generic callback to be set
  */
  virtual int setCallback(GenericCallback *callback)
  {
    callback->model_ = this;
    impl::BaseCallback *realcb = createCallbackImplDerived(callback);
    callback->impl_.reset(realcb);
    return setCallback(callback->impl_.get());
  }
  /**
  Set callback to be called during optimization
  @param callback The callback to be set
  */
  int setCallback(impl::BaseCallback *callback)
  {
    callback->model_ = this;
    return setCallbackDerived(callback);
  }
  /**
  Get all the variables of the current problem
  */
  std::vector<double> getSolutionVector();
  /**
  Get all the dual values of the current problem
  */
  std::vector<double> getDualVector() {
    return getConstraintsValueImpl(0, getNumCons());
  }
  /**
  Get the number of variables
  */
  virtual int getNumVars() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get the number of constraints
  */
  virtual int getNumCons() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /** 
  Get the solution status  P(generic)
  */
  virtual Status::SolStatus getStatus() {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /**
  Start the optimization process
  */
  virtual void optimize() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Write the solution file to the defualt location (filename.sol in the original directory)
  */
  virtual void writeSol() {
    writeSolImpl(NULL);
  };
  /**
  Write the solution file to a specific file
  @param solFileName Path of the solution file to write
  */
  virtual void writeSol(const char* solFileName) {
    writeSolImpl(solFileName);
  };
  /**
  Get "length" variables of the current problem in an array, starting at the specified
  position 
  @param first Index of the first variable to return
  @param length Number of variables to return
  @param sol Array to store the values
  */
  virtual int getSolution(int first, int length, double *sol) {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get the current objective value
  */
  virtual double getObj() {
    throw AMPLSolverException("Not implemented in base class!");
  };
  /**
  Get the error message corresponding to the code
  */
  virtual std::string error(int code) {
    throw AMPLSolverException("Not implemented in base class!");
  };

  /**
  Enable adding lazy constraints via callbacks (to be called only once),
  call before calling AMPLModel::optimize() if planning to add lazy constraints
  via callbacks
  */
  virtual void enableLazyConstraints() { }
  /**
  Utility function: prints all variables to screen
  */
  void printModelVars(bool onlyNonZero);

  /**
  Set an integer parameter (solver control) using ampls generic aliases
  @param param The parameter to be set
  @param value The integer value to set
  */
  virtual void setAMPLParameter(SolverParams::SolverParameters param,
    int value) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /**
  Set an double parameter (solver control) using ampls generic aliases
  @param param The parameter to be set
  @param value The double value to set
  */
  virtual void setAMPLParameter(SolverParams::SolverParameters params,
    double value) {
    throw AMPLSolverException("Not implemented in base class!");
  }

  /**
  Get the value of an integer parameter (solver control) using ampls generic aliases
  @param param The parameter to get
  */
  virtual int getAMPLIntParameter(SolverParams::SolverParameters params)  {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /**
  Get the value of a double parameter (solver control) using ampls generic aliases
  @param param The parameter to get
  */
  virtual double getAMPLDoubleParameter(SolverParams::SolverParameters param) {
    throw AMPLSolverException("Not implemented in base class!");
  }

  virtual int getAMPLIntAttribute(SolverAttributes::Attribs)  {
    throw AMPLSolverException("Not implemented in base class!");
  }

  virtual double getAMPLDoubleAttribute(SolverAttributes::Attribs) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /// <summary>
  /// Get a list of the options supported by the solver, as seen from the -= output
  /// of the solver driver. Note that these options are the ones found in the list 
  /// of the corresponding solver driver options at:
  /// https://dev.ampl.com/solvers/index.html
  /// </summary>
  /// <returns>A vector of option descriptions</returns>
  virtual std::vector<Option> getOptions() { 
    throw AMPLSolverException("Not implemented in base class!");
  };
  /// <summary>
  /// Set a solver driver option to the specified value. 
  /// See getOptions() for a list of supported options. Note that some options
  /// (most notably the converter options, with prefixes acc: and cvt:) cannot 
  /// be specified after the model has been loaded. Such options should be specified
  /// when loading/importing the model (AMPLModel::load or AMPLAPIInterface::exportModel<T>()).
  /// When in doubt, specify all options at loading time.
  /// </summary>
  virtual void setOption(const char* name, int value) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual void setOption(const char* name, double value) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  virtual void setOption(const char* name, const char* value) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /// <summary>
  /// Get the current value of the option 'name'.
  /// See getOptions() for a list of supported options.
  /// </summary>
  virtual int getIntOption(const char* name) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /// <summary>
  /// Get the current value of the option 'name'.
  /// See getOptions() for a list of supported options.
  /// </summary>
  virtual double getDoubleOption(const char* name) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /// <summary>
/// Get the current value of the option 'name'.
/// See getOptions() for a list of supported options.
/// </summary>
  virtual std::string getStringOption(const char* name) {
    throw AMPLSolverException("Not implemented in base class!");
  }
  /// <summary>
  /// Refresh the underlying solver driver, useful in case solver
  /// options that involve reading suffixes have been changed with 
  /// the AMPLModel::setOption set of functions
  /// </summary>
  virtual void refresh() {
    throw AMPLSolverException("Not implemented in base class!");
  }
};

class AMPLMPModel : public AMPLModel {
  // Store a cache of the available options to avoid unnecessary API calls
  std::vector<Option> options_;
   
protected:
  impl::mp::AMPLS_MP_Solver* solver_; // common across all MP-solvers
  mutable bool copied_;

  AMPLMPModel() :AMPLModel(), solver_(NULL), copied_(false) {}
  
  AMPLMPModel(const AMPLMPModel& other) : 
    AMPLModel(other), solver_(other.solver_),
    copied_(false) {
    other.copied_ = true;
  }
  AMPLMPModel(AMPLMPModel&& other) noexcept:
    AMPLModel(other), solver_(other.solver_),
    copied_(false) {
    other.copied_ = true;
  }
  
  AMPLMPModel& operator=(AMPLMPModel& other) {
    if (this != &other)
    {
      AMPLModel::operator=(other);
      solver_ = other.solver_;
    }
    other.copied_ = true;
    return *this;
  }
  AMPLMPModel& operator=(AMPLMPModel&& other) noexcept {
    if (this != &other)
    {
      AMPLModel::operator=(std::move(other));
      std::swap(solver_, other.solver_);
      other.solver_ = nullptr;
    }
    other.copied_ = true;
    return *this;
  }

  AMPLMPModel(impl::mp::AMPLS_MP_Solver* mp, const char* nlfile,
    const char** options)  : copied_(false) {
    if (impl::mp::AMPLSLoadNLModel(mp, nlfile, const_cast<char**>(options)))
    {
      std::string s;
      auto msgs = impl::mp::AMPLSGetMessages(mp);
      auto msg = msgs;
      while (*msg != nullptr) {
        s += *msg++;
      }
      throw std::runtime_error("Problem loading the model:\n"+ s);
    }
    solver_ = mp;
    fileName_ = nlfile;
  }
  void writeSolImpl(const char* solFileName) {
    impl::mp::AMPLSReportResults(solver_, solFileName);
  }

public:
  /// <summary>
  /// Get a list of the options supported by the solver, as seen from the -= output
  /// of the solver driver. Note that these options are the ones found in the list 
  /// of the corresponding solver driver options at:
  /// https://dev.ampl.com/solvers/index.html
  /// </summary>
  /// <returns>A vector of option descriptions</returns>
  virtual std::vector<Option> getOptions() {
    if (options_.size() == 0)
    {
      auto opt = impl::mp::AMPLSGetOptions(solver_);
      int i;
      impl::mp::AMPLS_C_Option o;
      for (i = 0, o = opt[0]; o.name != NULL; o=opt[++i])
        options_.push_back(Option (o.name, o.description, o.type)); // TODO
    }
    return options_;
  };
  /// <summary>
  /// Set a solver driver option to the specified value. 
  /// See getOptions() for a list of supported options.
  /// </summary>
  virtual void setOption(const char* name, int value);
  /// <summary>
  /// Set an option to the specified value.
  /// See getOptions() for a list of supported options.
  /// </summary>
  virtual void setOption(const char* name, double value);
  /// <summary>
/// Set an option to the specified value.
/// See getOptions() for a list of supported options.
/// </summary>
  virtual void setOption(const char* name, const char* value);
  /// <summary>
  /// Get the current value of the option 'name'.
  /// See getOptions() for a list of supported options.
  /// </summary>
  int getIntOption(const char* name) {
    int v;
    impl::mp::AMPLSGetIntOption(solver_, name, &v);
    return v;
  }
  /// <summary>
  /// Get the current value of the option 'name'.
  /// See getOptions() for a list of supported options.
  /// </summary>
  double getDoubleOption(const char* name) {
      double v;
      impl::mp::AMPLSGetDblOption(solver_, name, &v);
      return v;
  }
  /// <summary>
/// Get the current value of the option 'name'.
/// See getOptions() for a list of supported options.
/// </summary>
  std::string getStringOption(const char* name) {
    const char* v;
    impl::mp::AMPLSGetStrOption(solver_, name, &v); 
    return std::string(v);
  }

  /// <summary>
  /// Refresh the underlying model (especially useful if modifying
  /// with setOption() functionalities that need suffixes to be read
  /// from the NL file
  /// </summary>
  void refresh() {
    impl::mp::AMPLSReadExtras(solver_); }

  /// <summary>
  /// Solve using the underlying MP solver driver functionality
  /// </summary>
  void optimize() {
    impl::mp::AMPLSSolve(solver_);
    resetVarMapInternal();
  }

};

} // namespace

// Convenience macro to use solver-specific libs
#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_highs
#include "highs_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif
#ifdef USE_cbcmp
#include "cbcmp_interface.h"
#endif
#ifdef USE_copt
#include "copt_interface.h"
#endif
#ifdef USE_scip
#include "scip_interface.h"
#endif


#ifdef USE_amplapi
// Functions to link ampls and amplapi
// Binaries linked with this flag need to be linked 
// with the AMPLAPI library
#include "ampl/ampl.h"
#include <cstdio>

namespace ampls {

  

  namespace AMPLAPIInterface
  {

    namespace impl {
      inline void doExport(ampl::AMPL& a) {
        a.write("g___modelexport___", "cr");
      }
      inline void doRemove() {
        std::remove("___modelexport___.nl");
        //std::remove("___modelexport___.row");
       // std::remove("___modelexport___.col");
      }
      template <class T> inline T exportModel(ampl::AMPL& a, const char** options = nullptr);

#ifdef USE_gurobi
      template<> inline GurobiModel exportModel<GurobiModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        GurobiDrv gurobi;
        auto m =  gurobi.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif
#ifdef USE_highs
      template<> inline HighsModel exportModel<HighsModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        HighsDrv gurobi;
        auto m = gurobi.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif
#ifdef USE_cbcmp
      template<> inline CbcModel exportModel<CbcModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        CbcDrv cbc;
        auto m =  cbc.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif

#ifdef USE_copt
      template<> inline CoptModel exportModel<CoptModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        CoptDrv copt;
        auto m =  copt.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif

#ifdef USE_cplex
      template<> inline CPLEXModel exportModel<CPLEXModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        CPLEXDrv cplex;
        auto m =  cplex.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif

#ifdef USE_xpress
      template<> inline XPRESSModel exportModel<XPRESSModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        XPRESSDrv xpress;
        auto m =  xpress.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif

#ifdef USE_scip
      template<> inline SCIPModel exportModel<SCIPModel>(ampl::AMPL& a, const char** options) {
        doExport(a);
        SCIPDrv scip;
        auto m = scip.loadModel("___modelexport___.nl", options);
        doRemove();
        return m;
      }
#endif
    } // namespace impl
    /// <summary>
    /// Export model from the AMPLAPI instance using the specified options
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="a"></param>
    /// <param name="options"></param>
    /// <returns></returns>
    template <class T> inline T exportModel(ampl::AMPL& a) {
      return impl::exportModel<T>(a, nullptr);
    }
     template <class T> inline  T exportModel(ampl::AMPL& a, const char** options) {
      return impl::exportModel<T>(a, options);
    }
    template  <class T> inline  T exportModel(ampl::AMPL& a, const std::vector<std::string> &options) {
      const char** myptr = nullptr;
      std::vector<const char*> ptrs;
      if (options.size() > 0) {
        ptrs.reserve(options.size() + 1);
        for (const std::string &a : options)
          ptrs.push_back(a.data());
        ptrs.push_back(nullptr);
        myptr = ptrs.data();
      }
      return exportModel<T>(a, myptr);
    }

    inline void importModel(ampl::AMPL& a, AMPLModel& g) {
      g.setOption("wantsol", 9);
      g.writeSol();
      a.eval("solution ___modelexport___.sol;");
      g.setOption("wantsol", 1);
      std::remove("___modelexport___.sol;");
      a.eval(g.getRecordedEntities());
    }
  } // namespace AMPLAPIInterface
} // namespace ampls
#endif // USE_amplapi

#endif // ampls_H_INCLUDE_
