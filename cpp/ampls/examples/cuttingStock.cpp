#include <iostream>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include "ampl/ampl.h"


void declareModel(ampl::AMPL& a) {

  a.eval("param nPatterns integer >= 0;"
    "set PATTERNS = 1..nPatterns;  "
    "set WIDTHS;"                   // finished widths
    "param order{ WIDTHS } >= 0;"   // rolls of width j ordered
    "param overrun;"                // permitted overrun on any width
    "param rawWidth;"               // width of raw rolls to be cut
    "param rolls{ WIDTHS,PATTERNS } >= 0, default 0;"

    "var Cut{ PATTERNS } integer >= 0;" // raw rolls to cut in each pattern
    "minimize TotalRawRolls : sum{ p in PATTERNS } Cut[p];"
    "subject to OrderLimits{ w in WIDTHS }:"
    "order[w] <= sum{ p in PATTERNS } rolls[w, p] * Cut[p];"); // <= order[w] + overrun; ");
  //a.eval("show Cut;");
  int overrun = 0;
  int nPatterns = 5;
  double roll_width = 110;
  double ordersWidths[] = { 20, 45, 50, 55, 75 };
  double ordersAmount[] = { 48, 35, 24, 10, 8 };
  /*int nPatterns = 49;
  double ordersWidths[] = { 1630, 1625, 1620, 1617, 1540, 1529, 1528, 1505, 1504, 1484, 1466,
    1450, 1283, 1017, 970, 930, 916, 898, 894, 881, 855, 844, 805, 787, 786, 780, 754,
    746, 707, 698, 651, 644, 638, 605, 477, 473, 471, 468, 460, 458, 453, 447, 441,
    422, 421, 419, 396, 309, 266 };
  double ordersAmount[] = { 172, 714, 110, 262, 32, 100, 76, 110,20, 58, 15, 10, 40, 50, 70, 8, 210, 395,
      49, 17, 20, 10, 718, 17, 710, 150, 34, 15, 122, 7, 10, 15, 10, 10, 4, 34, 25, 10, 908,
      161, 765, 21, 20, 318, 22, 382, 22, 123, 35 };
 */
  a.getParameter("nPatterns").set(nPatterns); 
  a.getParameter("overrun").set(overrun);
  a.getParameter("rawWidth").set(roll_width);
  ampl::DataFrame df(1, { "WIDTHS", "order" });
  df.setColumn("WIDTHS", ordersWidths, nPatterns);
  df.setColumn("order", ordersAmount, nPatterns);
  a.setData(df, "WIDTHS");
  a.display("WIDTHS");
 

  /*
  # ----------------------------------------
# KNAPSACK SUBPROBLEM FOR CUTTING STOCK
# ----------------------------------------



var Use {WIDTHS} integer &gt;= 0;



*/
}

void AMPLSol(ampl::AMPL &a) {
  a.eval("param price {WIDTHS} default 0.0;");
  a.eval("var Use {WIDTHS} integer >= 0;");
  a.eval("minimize Reduced_Cost: 1 - sum{ i in WIDTHS } price[i] * Use[i]; ");
  a.eval("subject to Width_Limit: sum{ i in WIDTHS } i * Use[i] <=rawWidth; ");
  a.eval("problem Cutting_Opt: Cut, TotalRawRolls, OrderLimits; option relax_integrality 1; ");
  a.eval("problem Pattern_Gen: Use, Reduced_Cost, Width_Limit; option relax_integrality 0; ");

  // Start
  a.eval("let nPatterns := 0;"
    "for {i in WIDTHS} {"
    "let nPatterns := nPatterns + 1;"
    "let rolls[i, nPatterns] := floor(rawWidth / i);"
    "let{ i2 in WIDTHS : i2 <> i } rolls[i2, nPatterns] := 0;}");

  a.eval("repeat {"
    "solve Cutting_Opt;"
    "let{ i in WIDTHS } price[i] := OrderLimits[i].dual;"

    "solve Pattern_Gen;"
    "if Reduced_Cost < -0.00001 then{"
    "let nPatterns := nPatterns + 1;"
    "let {i in WIDTHS} rolls[i,nPatterns] := Use[i];"
    "}"
    "else break;"
    "};");
  a.eval("display rolls, Cut;"
    "option Cutting_Opt.relax_integrality 0;"
    "solve Cutting_Opt;"
    "display Cut;");
}
template <class T> void doStuff()
{
  try {
    ampl::AMPL a;
    declareModel(a);
    AMPLSol(a);
  }
  catch (const std::exception& e) {
    printf("%s\n", e.what());
  }
 // auto m= ampls::AMPLAPIInterface::exportModel<T>(a);


}
int main(int argc, char** argv) {
#ifdef USE_gurobi
  doStuff<ampls::GurobiModel>();
#endif
  /*
#ifdef USE_copt
  doStuff<ampls::CoptModel>();
#endif
#ifdef USE_cplexmp
  doStuff<ampls::CPLEXModel>();
#endif
#ifdef USE_cbcmp
  doStuff<ampls::CbcModel>();
#endif
#ifdef USE_xpress
  doStuff<ampls::XPRESSModel>();
#endif*/
  return 0;
 
}
