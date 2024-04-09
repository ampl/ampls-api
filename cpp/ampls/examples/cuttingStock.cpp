#include <iostream>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "iomanip"
#include <cassert>
#include <string>
#include "ampl/ampl.h"


// Example of column generation
/// <summary>
/// Represents a pattern (a way to cut a roll)
/// </summary>
struct Pattern {
  std::map<double, double> p;
 
  int size() { return p.size(); }
  void addCut(double width, int number) {
    p[width] = number;
  }
};

/// <summary>
/// Stores all the pattern types we created 
/// </summary>
class Patterns {
  std::vector<Pattern> pats_;
  std::vector<double> widths_;
public:
  int size() { return pats_.size(); }
  static Patterns generateDefaultPatterns(const std::vector<double> &widths, int totalWidth) {
    Patterns pats;
    for (auto w : widths) {
      Pattern pat;
      pat.addCut(w, std::floor(totalWidth / w));
      pats.pats_.push_back(pat);
    }
    pats.widths_ = widths;
    return pats;
  }

  void add(const Pattern& p) {
    pats_.push_back(p);
  }

  /**
  * Print all the patterns currently defined. Useful when using ampls native entities recording
  */
  void print() {
    std::cout << std::left << std::setw(7) << "Widths:";
      for (double w : widths_)
        std::cout << std::right << std::setw(4) << w;
    std::cout << std::endl;
    
    int i = 0;
    for (const auto& pat : pats_)
    {
      std::cout << std::left << std::setw(7) << "pat" << ++i;
      for (double w : widths_)
      {
        auto it = pat.p.find(w);
        if (it == pat.p.end())
          std::cout << std::right << std::setw(4) << "";
        else
          std::cout << std::right << std::setw(4)<< pat.p.at(w);
      }
      std::cout << std::endl;
    }
  }

  /// <summary>
  /// Add the current patterns to AMPL. Note that it just appends
  /// the ones not already present in the AMPL instance
  /// </summary>
  void addtoAMPL(ampl::AMPL& a, const char* npatternsparam,
    const char* divisionparam) {
    // Check how many patterns we have in AMPL
    auto param = a.getParameter(npatternsparam);
    int existingPatterns = (int)param.get().dbl();
    // Set the new number of patterns
    param.set(size()); 

    // Only add the "new" patterns
    ampl::DataFrame df(2, { "width", "pattern", divisionparam });
    for (int patternindex = existingPatterns; patternindex < size(); patternindex++)
      for (auto cut : pats_[patternindex].p) 
        df.addRow(cut.first, patternindex+1, cut.second);

    // Set the data in AMPL
    a.eval("update data " + std::string(divisionparam) + ";");
    a.setData(df);
   }
};

/// <summary>
/// Set data for the main problem instance
/// </summary>
Patterns setData(ampl::AMPL& a) {
  int nWidths = 5;
  double roll_width = 110;
  std::vector<double>ordersWidths = { 20, 45, 50, 55, 75 };
  std::vector<double>ordersAmount = { 48, 35, 24, 10, 8 };

  a.getParameter("nPatterns").set(0);
  a.getParameter("rawWidth").set(roll_width);
  ampl::DataFrame df(1, { "WIDTHS", "order" });
  df.setColumn("WIDTHS", ordersWidths.data(), nWidths);
  df.setColumn("order", ordersAmount.data(), nWidths);
  a.setData(df, "WIDTHS");
  Patterns p = Patterns::generateDefaultPatterns(ordersWidths, roll_width);
  p.addtoAMPL(a, "nPatterns", "rolls");
  return p;
}
void declareCuttingModel(ampl::AMPL& a) {
  a.setIntOption("presolve", 0);

  a.eval("param nPatterns integer >= 0;"
    "set PATTERNS = 1..nPatterns;  "
    "set WIDTHS;"                   // finished widths
    "param order{ WIDTHS } >= 0;"   // rolls of width j ordered
    "param rawWidth;"               // width of raw rolls to be cut
    "param rolls{ WIDTHS,PATTERNS } >= 0, default 0;"
    "var Cut{ PATTERNS } integer >= 0;" // raw rolls to cut in each pattern
    
    "minimize TotalRawRolls : sum{ p in PATTERNS } Cut[p] + to_come;"
    "subject to OrderLimits{ w in WIDTHS }:"
    "sum{ p in PATTERNS } rolls[w, p] * Cut[p]+ to_come >= order[w];"); 
}

void declareKnapsackModel(ampl::AMPL& a) {
  a.eval("param price {WIDTHS} default 0.0;");
  a.eval("var Use {WIDTHS} integer >= 0;");
  a.eval("minimize Reduced_Cost: 1 - sum{ i in WIDTHS } price[i] * Use[i]; ");
  a.eval("subject to Width_Limit: sum{ i in WIDTHS } i * Use[i] <=rawWidth; ");
  a.eval("problem Pattern_Gen: Use, Reduced_Cost, Width_Limit; option relax_integrality 0; ");
}


Pattern generatePattern(ampl::AMPL& a, std::vector<double> duals) {
  // Use knapsack problem to generate new patterns to be considered
  a.getParameter("price").setValues(duals.data(), duals.size());
  a.eval("solve Pattern_Gen;");
  auto val = a.getObjective("Reduced_Cost").value();
  Pattern p;
  if (val < -0.00001) {
    auto use = a.getData("Use"); // return new pattern to add
    for (auto r : use)
      p.addCut(r[0].dbl(), r[1].dbl());
  }
  return p;
}

std::vector<double> get_column(ampl::DataFrame& df, const std::string& name) {
  auto duals_col = df.getColumn(name);
  std::vector<double> res;
  res.reserve(df.getNumRows());
  for (auto r : duals_col)
    res.push_back(r.dbl());
  return res;
}

template <class T> double SolveWithAMPLS(ampl::AMPL& a, Patterns &pat) {

  // Create a second AMPL instance just for the knapsack problem
  ampl::AMPL ab;
  ab.eval("set WIDTHS; param rawWidth;");
  declareKnapsackModel(ab);
  ab.setData(a.getData("WIDTHS"), "WIDTHS");
  ab.getParameter("rawWidth").set(a.getParameter("rawWidth").get());

  // Export model to ampls
  a.eval("display order, rolls;");
  a.setIntOption("relax_integrality", 1);
  auto cutting_opt = ampls::AMPLAPIInterface::exportModel<T>(a);

#ifdef USE_scip
  if (std::is_same<T, ampls::SCIPModel>::value)
  {
    cutting_opt.setOption("pre:maxrounds", 0);
    cutting_opt.setOption("heu:settings",3);
    cutting_opt.setOption("pro:maxrounds", 0);
    cutting_opt.setOption("pro:maxroundsroot", 0);
  }
#endif

  ab.setOption("solver", cutting_opt.driver());
  // Solution cycle
  for (int it = 0;; it++) {
    
    // Get the solution of the relaxed cutting model
    cutting_opt.optimize();
    printf("Solved iteration, obj=%f\n", cutting_opt.getObj());
    // Get the duals
    auto vv = cutting_opt.getDualVector();
    auto p = generatePattern(ab, vv);
    
    if (p.size() > 0)
    {
      pat.add(p);
      std::vector<int> indices;
      std::vector<double> coeffs;
      int rowindex = 0;
      // Get variable coefficients for each constraint
      for (auto r : p.p)
      {
        if (r.second != 0)
        {
          indices.push_back(rowindex);
          coeffs.push_back(r.second);
        }
        rowindex++;
      }
      // Add variable in the solver model via ampls
      // Note that the variables are continuous because we only solve
      // the relaxation at ampls level
      cutting_opt.record(cutting_opt.addVariable(
        indices.size(),
        indices.data(), coeffs.data(), 0, cutting_opt.infinity(),
        1, ampls::VarType::Integer, true));

    }
    else
      break;
  }

  // At the end of the solution process, we add all the patterns
  // we devised to AMPL before importing the model, so that we have 
  // "space" to read the solution back
  //pat.addtoAMPL(a, "nPatterns", "rolls");
  // Import the solution in AMPL
  ampls::AMPLAPIInterface::importModel(a, cutting_opt);
  // Solve the integer problem
  a.setIntOption("relax_integrality", 0);
  a.setOption("solver", cutting_opt.driver());
  a.solve();
  // Display the result
  a.display("rolls, Cut;");
  a.display("{i in 1.._nvars} _varname[i]");
  pat.print();
  return a.getObjective("TotalRawRolls").value();
}

double SolveWithAMPLScript(ampl::AMPL &a, Patterns &patterns) {
  declareKnapsackModel(a);
  // For solving using AMPL scripts + problems, we declare problems
  // so that we can solve them separately in one AMPL instance
  a.eval("problem Cutting_Opt: Cut, TotalRawRolls, OrderLimits; option relax_integrality 1; ");
  a.setOption("solver", "gurobi");
  while(true) {
    a.eval("solve Cutting_Opt;");
    auto duals = a.getData("OrderLimits.dual");
    std::cout << duals.toString();
    auto duals_array = get_column(duals, "OrderLimits.dual");
    auto p= generatePattern(a, duals_array);
    
    if (p.size() > 0)
    {
      patterns.add(p);
      patterns.addtoAMPL(a, "nPatterns", "rolls");
    }
    else
      break;
  }

  a.eval("option Cutting_Opt.relax_integrality 0;"
    "solve Cutting_Opt;"
    "display TotalRawRolls, rolls, Cut;");
  return a.getObjective("TotalRawRolls").value();

}

template <class T> void example()
{
  try {
    ampl::AMPL a;
    declareCuttingModel(a);
    Patterns p = setData(a);
    double ampls = SolveWithAMPLS<T>(a, p);

    a.reset();
    declareCuttingModel(a);
    p = setData(a);
    double ampl = SolveWithAMPLScript(a, p);
    assert(ampls == ampl);
  }
  catch (const std::exception& e) {
    printf("%s\n", e.what());
  }


}


int main(int argc, char** argv) {
#ifdef USE_gurobi
  //example<ampls::GurobiModel>();
#endif

#ifdef USE_highs
  example<ampls::HighsModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif

#ifdef USE_cplex
  example<ampls::CPLEXModel>();
#endif

#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif
  return 0;
 
}
