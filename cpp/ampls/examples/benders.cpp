#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR
#include "ampl/ampl.h"

#include <vector>
#include <map>
#include <limits>
#include <algorithm>>
#include <iostream>
#include <iomanip>
#include <cassert>

// Utility function to extract keys and values from a map<string, double>
std::pair<std::vector<ampl::Tuple>, std::vector<double>> to_tuple(const std::map<std::string, double>& myMap) {
  std::vector<ampl::Tuple> keys;
  std::vector<double> values;

  for (const auto& pair : myMap) {
    keys.push_back(ampl::Tuple(pair.first));
    values.push_back(pair.second);
  }

  return std::make_pair(keys, values);
}

// Utility function to extract keys and values from a map<string, map<string, double>>
std::pair<std::vector<ampl::Tuple>, std::vector<double>> to_tuple(const
  std::map<std::string, std::map<std::string, double>>& myMap) {
  std::vector<ampl::Tuple> keys;
  std::vector<double> values;

  for (const auto& outerPair : myMap) {
    for (const auto& innerPair : outerPair.second) {
      keys.push_back(ampl::Tuple(innerPair.first, outerPair.first));
      values.push_back(innerPair.second);
    }
  }

  return std::make_pair(keys, values);
}

void create_common(ampl::AMPL& a) {
  a.eval("set FACILITIES;"
    "set CUSTOMERS;"
    "set SCENARIOS;"
    "param prob{SCENARIOS} default 1/card(SCENARIOS);"
    "param sub_scenario symbolic in SCENARIOS;"
    "param customer_demand{CUSTOMERS, SCENARIOS} >= 0;"

    "param facility_capacity{FACILITIES} >= 0;"
    "param variable_cost{ FACILITIES, CUSTOMERS } >= 0;"

  );
}

void create_sub_problem(ampl::AMPL& a) {
  create_common(a);
  a.eval(
    "param sub_facility_open{ FACILITIES } default 1; "
    "var production{FACILITIES, CUSTOMERS, SCENARIOS} >= 0;"
    "minimize operating_cost:"
    "sum{ i in FACILITIES, j in CUSTOMERS }"
    "variable_cost[i, j] * production[i, j, sub_scenario]; "

    "s.t.satisfying_customer_demand{ j in CUSTOMERS }:"
    "sum{i in FACILITIES} production[i, j, sub_scenario] >= customer_demand[j, sub_scenario];"

    "s.t.facility_capacity_limits{i in FACILITIES}:"
    "sum{ j in CUSTOMERS } production[i, j, sub_scenario] <= facility_capacity[i] * sub_facility_open[i];");
}

void create_master_problem(ampl::AMPL& a) {
  create_common(a);
  a.eval("param fixed_cost{ FACILITIES } >= 0;"
    "var sub_variable_cost{SCENARIOS} >= 0;"
    "var facility_open{ FACILITIES } binary;"
    "minimize TotalCost :"
    "sum{ i in FACILITIES } fixed_cost[i] * facility_open[i] + sum{s in SCENARIOS} prob[s]*sub_variable_cost[s];"
    "s.t. sufficient_production_capacity:"
    "sum{ i in FACILITIES } facility_capacity[i] * facility_open[i] >= max{ s in SCENARIOS } sum{ j in CUSTOMERS } customer_demand[j, s];");
}

// Load data into the AMPL model
void load_data(ampl::AMPL& a, const std::vector<const char*>& facilities,
  const std::vector<const char*>& customers, const std::vector<const char*>& scenarios,
  const std::map<std::string, double>& facility_capacity_map,
  const std::map<std::string, std::map<std::string, double>>& customer_demand_map,
  const std::map<std::string, std::map<std::string, double>>& variable_cost_map
) {

  a.getSet("FACILITIES").setValues(facilities.data(), facilities.size());
  a.getSet("CUSTOMERS").setValues(customers.data(), customers.size());
  a.getSet("SCENARIOS").setValues(scenarios.data(), scenarios.size());

  auto fc = to_tuple(facility_capacity_map);
  a.getParameter("facility_capacity").setValues(std::get<0>(fc).data(), std::get<1>(fc).data(), std::get<0>(fc).size());
  auto vc = to_tuple(variable_cost_map);
  a.getParameter("variable_cost").setValues(std::get<0>(vc).data(), std::get<1>(vc).data(), std::get<0>(vc).size());

  auto cd = to_tuple(customer_demand_map);
  a.getParameter("customer_demand").setValues(std::get<0>(cd).data(), std::get<1>(cd).data(), std::get<0>(cd).size());
}
void load_master_data(ampl::AMPL& a) {
  a.eval("data;"
    "param fixed_cost :=   Watson 39268000 Bayshore 65268000 Orange 60268000 Evanston 48268000;");
}


int findIndexWithSubstring(const std::map<std::string, int>& myMap, const std::string& substring) {
  for (const auto& pair : myMap) {
if (pair.first.find(substring) != std::string::npos) {
  return pair.second; // Return the integer associated with the key
}
    }
  throw std::runtime_error("Index not found!");
}

/** Add a cut (optimality of feasibility) to the model*/
void addBendersCut(ampls::AMPLModel& model, const ampl::DataFrame& cdduals,
  const ampl::DataFrame& fcduals, 
  const std::map<std::string, int>& index_facility_open,
  int index_sub_variable_cost_scen,
  const std::map<std::string, double>& facility_capacity,
  const std::map<std::string, double>& scenario_customer_demand,
  bool optimalityCut) {

  // Note that to accomodate for solver view, we formulate it as below
  // sub_variable_cost[s] - sum{ i in FACILITIES } facility_price[i, s, k] * facility_capacity[i] * facility_open[i]
  // >=   sum{j in CUSTOMERS} customer_price[j,s,k]*customer_demand[j,s]; 
  std::vector<int> indices;
  std::vector<double> coeffs;
  for (auto fc : fcduals) {
      indices.push_back(index_facility_open.at(fc[0].str()));
      coeffs.push_back(-fc[1].dbl() * facility_capacity.at(fc[0].str()));
  }
  if (optimalityCut) { 
    indices.push_back(index_sub_variable_cost_scen);
    coeffs.push_back(1);
  }
  double rhs = 0;
  for (auto cd : cdduals) {
    rhs += cd[1].dbl() * scenario_customer_demand.at(cd[0].str());
  }
  std::string name = optimalityCut ? "optimality" : "feasibility";
  auto cut = model.addConstraint(indices.size(), indices.data(), coeffs.data(),
    ampls::CutDirection::GE, rhs);
  model.record(cut);
  std::cout << "Added " << name << " cut:\n" << cut.toString();
}

template <class T> void solve(ampl::AMPL& master, ampl::AMPL& sub) {

  // Set some options
  sub.eval("suffix dunbdd;");
  master.setIntOption("presolve", 0);
  sub.setIntOption("presolve", 0);
  sub.setIntOption("solver_msg", 0);

  // Data
  std::vector<const char*> FACILITIES({ "Watson", "Bayshore", "Orange", "Evanston"});
  std::vector<const char*> CUSTOMERS({ "CEM_Store_41594", "CEM_Cypress_Market", "CEM_Time_Mart_23", "CEM_Cypress_CEM", "CEM_Fuel_Maxx_84", "CEM_Commercial_CEM", "CEM_New_Brunswick", "CEM_Bernards_PGroup" });
  std::vector < const char*> SCENARIOS({ "Low", "Medium", "High" });
  std::map<std::string, double> facility_capacity_map{
        {"Watson", 75000},
        {"Bayshore",200000},
        {"Orange", 150000},
        {"Evanston", 90000}
  };

  std::map<std::string, double> lowDemand, mediumDemand, highDemand;
  for (auto c : CUSTOMERS) {
    lowDemand[c] = 10000;
    mediumDemand[c] = 15000;
    highDemand[c] = 20000;
  }

  std::map<std::string, std::map<std::string, double>> customer_demand_map{
      { "Low",  lowDemand},
      { "Medium",  mediumDemand },
      { "High",  highDemand }
  };

  std::map<std::string, std::map<std::string, double>> variable_cost_map{
        {"CEM_Store_41594", { {"Watson", 6739.72500}, {"Bayshore", 10355.05000}, {"Orange", 7650.40000}, {"Evanston", 5219.50000} }},
        {"CEM_Cypress_Market", { {"Watson", 4739.72500}, {"Bayshore", 8355.05000}, {"Orange", 17320.40000}, {"Evanston", 15000.50000} }},
        {"CEM_Time_Mart_23", { {"Watson", 7739.72500}, {"Bayshore", 9355.05000}, {"Orange", 7320.40000}, {"Evanston", 5000.50000} }},
        {"CEM_Cypress_CEM", { {"Watson", 5739.72500}, {"Bayshore", 6355.05000}, {"Orange", 7320.40000}, {"Evanston", 9433.70000}}},
        {"CEM_Fuel_Maxx_84", { {"Watson", 9739.72500}, {"Bayshore", 10355.05000}, {"Orange", 9650.40000}, {"Evanston", 5219.50000} }},
        {"CEM_Commercial_CEM", { {"Watson", 5093.25000}, {"Bayshore", 11355.05000}, {"Orange", 5320.40000}, {"Evanston", 10433.70000}}},
        {"CEM_New_Brunswick", { {"Watson", 4987.72500}, {"Bayshore", 8935.05000}, {"Orange", 7320.40000}, {"Evanston", 5875.50000}}},
        {"CEM_Bernards_PGroup", { {"Watson", 8011.72500}, {"Bayshore", 8300.05000}, {"Orange", 12110.40000}, {"Evanston", 10000.50000}}}
  };

  // Load data into AMPL instances
  load_data(master, FACILITIES, CUSTOMERS, SCENARIOS, facility_capacity_map, customer_demand_map,
    variable_cost_map);
  load_master_data(master);
  load_data(sub, FACILITIES, CUSTOMERS, SCENARIOS, facility_capacity_map, customer_demand_map,
    variable_cost_map);

  // Export to AMPLS
  auto master_ampls = ampls::AMPLAPIInterface::exportModel<T>(master);
  auto map = master_ampls.getVarMap();
  
  master.setOption("solver", master_ampls.driver());
  sub.setOption("solver", master_ampls.driver());

  // Store some maps between the AMPL variables and the position in the solvers view
  std::map<std::string, int> index_facility_open, index_sub_variable_cost;
  std::map<int, std::string> revindex_facility_open, revindex_sub_variable_cost;
  for (auto f : FACILITIES) {
    int in = findIndexWithSubstring(map, f);
    index_facility_open[f] = in;      // map FACILITY -> solverindex facility_open[FACILITY]
    revindex_facility_open[in] = f;   // map solverindex facility_open[FACILITY] -> FACILITY
  }
  for (auto s : SCENARIOS) {
    int in = findIndexWithSubstring(map, s);
    index_sub_variable_cost[s] = in;      // map SCENARIOS -> solverindex sub_variable_cost[SCENARIOS]
    revindex_sub_variable_cost[in] = s;   // map solverindex sub_variable_cost[SCENARIOS] -> CUSTOMER

  }
  
  // Constraints of subproblems that we will use to extract dual/unbounded rays
  auto satisfying_customer_demand = sub.getConstraint("satisfying_customer_demand");
  auto facility_capacity_limits = sub.getConstraint("facility_capacity_limits");
  auto scenario_subproblem = sub.getParameter("sub_scenario");
  
  // Maps to store indices and coefficients (used when actually adding cut using AMPLS)
  int n_noviolations = 0;
  double epsilon = 0.00000001;
  std::map<std::string, double> sub_variable_cost;
  for (auto s : SCENARIOS) {
    sub_variable_cost[s] = 0;
  }
  
  // Set formatting to avoid scientific notation
  std::cout << std::fixed << std::setprecision(2);
  // Main iterations loop
  for (int it = 1; ;it++) {
    std::cout << std::endl << "******* Iteration " << it << " *******";
    n_noviolations = 0;
    for (auto s : SCENARIOS) {
      scenario_subproblem.set(s); // set the scenario in the subproblem
      sub.getOutput("solve;");
      sub.solve();
      auto result = sub.getValue("solve_result").str();
      std::cout << std::endl << std::endl << "Scenario " << s << "   ";
      if (result == "infeasible") {
        auto scd = satisfying_customer_demand.getValues("dunbdd");
        auto fcl = facility_capacity_limits.getValues("dunbdd");
        addBendersCut(master_ampls, scd, fcl,
          index_facility_open, 0, facility_capacity_map,
          customer_demand_map.at(s), false);
      }
      else if (sub.getValue("operating_cost").dbl() > sub_variable_cost[s] + epsilon)
      {
        auto scd = satisfying_customer_demand.getValues("dual");
        auto fcl = facility_capacity_limits.getValues("dual");
        addBendersCut(master_ampls, scd, fcl,
          index_facility_open, index_sub_variable_cost.at(s), facility_capacity_map,
          customer_demand_map.at(s), true);
      }
      else
      {
        std::cout << "No violation." << std::endl;
        n_noviolations += 1;
      }
    }
    // If no scenario violates optimality and feasibility conditions
    // we converged to a solution
    
    if (n_noviolations == SCENARIOS.size()) break;

    // Otherwise resolve the master
    std::cout << "Resolving master problem" << std::endl;
    master_ampls.optimize();
    std::cout << "OBJ=" << master_ampls.getObj() << std::endl;
    // Set sub_facility_open with the master solution
    // Here we need to pass from solver view (indices) to AMPL 
    // view (variable name and index value)
    auto sol = master_ampls.getSolutionVector();
    std::vector<ampl::Tuple> indices;
    std::vector<double> values;
    for (auto a : revindex_facility_open) {
      indices.push_back(ampl::Tuple(a.second));
      values.push_back(sol[a.first]);
    }
    sub.getParameter("sub_facility_open").setValues(indices.data(), values.data(), indices.size());
    // Update the sub_variable_cost values
    for (auto a : revindex_sub_variable_cost)
      sub_variable_cost[a.second] = sol[a.first];
  }
  std::cout << std::endl << std::endl << "***** Optimal solution found, cost: " << master_ampls.getObj() << std::endl;

  // Import the AMPLS model back to AMPL, and show the additional cuts
  ampls::AMPLAPIInterface::importModel(master, master_ampls);
  std::cout << "Optimal solution in AMPL:" << master.getObjective("TotalCost").value() << std::endl;
  master.eval("expand {c in 1.._ncons} _con[c];");

  assert(abs(master.getObjective("TotalCost").value() - 756943875) < 10e-3);
  
}

template <class T> void example() {
  ampl::AMPL master, sub;
  try {
    create_master_problem(master);
    create_sub_problem(sub);
    solve<T>(master, sub);
   
  }
  catch (const std::exception& e) {
    std::cout << e.what();
  }
}

int main(int argc, char** argv) {
  double obj;
#ifdef USE_highs
  example<ampls::HighsModel>();
#endif

#ifdef USE_scip
  example<ampls::SCIPModel>();
#endif

#ifdef USE_gurobi
  example<ampls::GurobiModel>();
#endif




#ifdef USE_xpress
  example<ampls::XPRESSModel>();
#endif

#ifdef USE_cplex
 example<ampls::CPLEXModel>();
#endif

#ifdef USE_copt
  example<ampls::CoptModel>();
#endif

#ifdef USE_cbcmp
 example<ampls::CbcModel>();
#endif

  return 0;
}