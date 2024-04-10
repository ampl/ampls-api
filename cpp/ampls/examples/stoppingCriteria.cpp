#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <cstdio>  
#include <cassert>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <optional>

// This example illustrates how to implement a custom
// stoppping criteria using callbacks

// Set to true to activate the MIP gap criteria
const bool STOP_AT_MIPGAP = true;
// Set to true to stop when no solution improvement has been detected
// for a certain time
const bool STOP_AT_NOSOLUTIONIMPROVEMENT = true;

// Desired MIP gap where to stop execution
const double DESIREDGAP = 0.4;
// Define the maximum time without improvement in
// the solution (in seconds)
const double MAX_TIME = 6;
// Flag to turn on verbose output
const bool MUTE = true;

// User-defined class implementing the stopping functionality
class StoppingCallback : public ampls::GenericCallback {
public:
  StoppingCallback() {}

  int evaluateMIPgap(bool mipsol) {
    // At each solution and each node, we check if the desired gap
    // abs(bestobjective-objbound)/bestobjective
    // has been achieved. If so, terminate the solution process.
    double objBound = getValue(ampls::Value::MIP_OBJBOUND).dbl;
    double gap = getValue(ampls::Value::MIP_RELATIVEGAP).dbl;
    double obj = getObj();

    if ((lastBound >= objBound) && (lastObj <= obj)) {
      if (!MUTE) std::cout << "No improvement, skipping evaluation" << std::endl;
      return 0;
    }
    lastBound = objBound;
    if (!STOP_AT_NOSOLUTIONIMPROVEMENT) lastObj = obj;
    
    // Count non-zeroes
    auto sol = getSolutionVector();
    int nnz = sol.size() - std::count(sol.begin(), sol.end(), 0);

    // Output progress
    std::cout <<  std::setprecision(4) << "Objective: " 
      << std::setw(5) << obj << " - bound: "
      << std::setw(5) << objBound << " - relgap: " << std::setw(6) << gap 
      << " - nonzeroes: " << std::setw(3) << nnz << std::endl;
    if (gap < DESIREDGAP) {
      std::cout << "Desired gap reached, terminating" << std::endl;
      return -1;
    }
    return 0;
  }

  int evaluateSolutionProgress() {
    if (lastObjTime == std::chrono::steady_clock::time_point::min())
    {
      // first solution, start computing times
      lastObjTime = std::chrono::steady_clock::now();
      return 0;
    }
    // Calculate the duration in seconds
    auto now = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastObjTime).count() / 1000.0;

    double obj = getObj();
    // If objective has improved, update its value and the timestamp
    if (obj < lastObj) {
      std::cout << "Solution improvement after " << duration << "s" << std::endl;
      lastObjTime = now;
      lastObj = obj;
      return 0; // Continue solving
    }
    if (duration > MAX_TIME) {
        std::cout << "No improvement in the solution for " << duration << "s, terminating." << std::endl;
        return -1; // Stop solving
    }
    return 0;
  }
      
  int run() override {
    switch (getAMPLWhere())
    {
      case ampls::Where::MIPNODE:
        if (STOP_AT_MIPGAP) {
          if (!MUTE) std::cout << "*MIP node* ";
          return evaluateMIPgap(false);
        }
        break;
      case ampls::Where::MIPSOL:
        if (!MUTE) std::cout << "*MIP sol*  ";
        if (STOP_AT_MIPGAP) {
          if (evaluateMIPgap(true)==-1)
            return -1;
          if (STOP_AT_NOSOLUTIONIMPROVEMENT)
            return evaluateSolutionProgress();
        }
        break;
    }
    return 0;
  }

private:
  double lastBound    = 0;
  double lastObj      = std::numeric_limits<double>::infinity();
  std::chrono::steady_clock::time_point lastObjTime = std::chrono::steady_clock::time_point::min();
};

template<class T> void example() 
{
  const char* MODELNAME = "queens18.nl";
  std::string md(MODELS_DIR);
  md += MODELNAME;

  T m = ampls::AMPLModel::load<T>(md.c_str());
  // Set a (generic) callback
  StoppingCallback cb;
  m.setCallback(&cb);
  // Start the optimization process
  m.optimize();

  // Get the objective value
  double obj = m.getObj();
  printf("\nSolution with %s=%f\n", m.driver(), obj);
  
  assert( (obj>= 158*(1-DESIREDGAP)) && (obj <= 158 * (1 + DESIREDGAP)) );
  ampls::Status::SolStatus s = m.getStatus();
  assert(s == ampls::Status::INTERRUPTED);
  switch (s)
  {
    case ampls::Status::OPTIMAL:
      printf("Optimal.\n");
      break;
    case ampls::Status::INFEASIBLE:
      printf("Infeasible.\n");
      break;
    case ampls::Status::UNBOUNDED:
      printf("Unbounded.\n");
      break;
    case ampls::Status::INTERRUPTED:
      printf("Interrupted.\n");
      break;
    default:
      printf("Status: %d\n", s);
  }

  // Write the AMPL sol file
  m.writeSol();
}
int main(int argc, char** argv) {

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif

#ifdef USE_highs
  example<ampls::HighsModel >();
#endif

#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif

#ifdef USE_cplex
  example<ampls::CPLEXModel >();
#endif


#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif
  

  
#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cbcmp
  example<ampls::CbcModel>();
#endif*/
  return 0;
 
}
