#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR
#include "ampl/ampl.h"

#include <vector>
#include <map>
#include <limits>
#include <algorithm>>
#include <iostream>


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


const char SOLVER[] = "cplex";

void create_common(ampl::AMPL& a) {
  a.setOption("solver", SOLVER);
  a.eval("set FACILITIES;"
    "set CUSTOMERS;"
    "set SCENARIOS;"
    "param prob{SCENARIOS} default 1/card(SCENARIOS);"
    "param ss symbolic in SCENARIOS;"
    "param customer_demand{CUSTOMERS, SCENARIOS} >= 0;"

    "param facility_capacity{FACILITIES} >= 0;"
    "param variable_cost{ FACILITIES, CUSTOMERS } >= 0;"
    "param is_facility_open{ FACILITIES } default 1; "
  );
}



void create_sub_problem(ampl::AMPL& a) {
  create_common(a);
  a.eval("var production{FACILITIES, CUSTOMERS, SCENARIOS} >= 0;"
    "minimize ProductionCost_Scenario:"
    "prob[ss] * (sum{ i in FACILITIES, j in CUSTOMERS }"
    "variable_cost[i, j] * production[i, j, ss]); "

    "s.t.satisfying_customer_demand{ j in CUSTOMERS }:"
    "sum{i in FACILITIES} production[i, j, ss] >= customer_demand[j, ss];"

    "s.t.facility_capacity_limits{i in FACILITIES}:"
    "sum{ j in CUSTOMERS } production[i, j, ss] <= facility_capacity[i] * is_facility_open[i];");
}


void create_master_problem(ampl::AMPL& a) {
  create_common(a);
  a.eval("var max_stage2_cost>=0;"
    "param fixed_cost{ FACILITIES } >= 0;"
    "var facility_open{ FACILITIES } binary;"
    "minimize TotalCost :"
    "sum{ i in FACILITIES } fixed_cost[i] * facility_open[i] + max_stage2_cost;");
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
  a.getParameter("facility_capacity").setValues(std::get<0>(fc).data(), std::get<1>(fc).data(), 3);

  auto vc = to_tuple(variable_cost_map);
  a.getParameter("variable_cost").setValues(std::get<0>(vc).data(), std::get<1>(vc).data(), std::get<0>(vc).size());

  auto cd = to_tuple(customer_demand_map);
  a.getParameter("customer_demand").setValues(std::get<0>(cd).data(), std::get<1>(cd).data(), std::get<0>(cd).size());
}
void load_master_data(ampl::AMPL& a) {
  a.eval("data;"
    "param fixed_cost := Watson 400000 Bayshore 500000 Orange 600000; ");
}



double calculate_sum(ampl::DataFrame& d, std::map<std::string, double> & weight) {
  double sum = 0;
  for (auto r : d) 
    sum += weight[r[0].c_str()] * r[1].dbl();
  return sum;

}

int findIndexWithSubstring(const std::map<std::string, int>& myMap, const std::string& substring) {
  for (const auto& pair : myMap) {
if (pair.first.find(substring) != std::string::npos) {
  return pair.second; // Return the integer associated with the key
}
    }
  throw std::runtime_error("Index not found!");
}


template <class T> void solve(ampl::AMPL& master, ampl::AMPL& sub) {

  // Set some options
  sub.eval("suffix dunbdd;");
  sub.setIntOption("presolve", 0);
  sub.setIntOption("solver_msg", 0);

  // Data
  std::vector<const char*> FACILITIES({ "Watson", "Bayshore", "Orange" });
  std::vector<const char*> CUSTOMERS({ "Station_LA", "Store_TX", "Center_TX", "Store_AZ" });
  std::vector < const char*> SCENARIOS({ "Low", "Medium", "High" });
  std::map<std::string, double> facility_capacity_map{
        {"Watson", 1550},
        {"Bayshore",650},
        {"Orange", 1750}
  };

  std::map<std::string, std::map<std::string, double>> customer_demand_map{
      { "Low",  { { "Station_LA", 166}, { "Store_TX", 91}, { "Center_TX", 679}, {"Store_AZ", 1441} } },
      { "Medium",  { { "Station_LA", 166}, { "Store_TX", 105}, { "Center_TX", 716}, {"Store_AZ", 1500} } },
      { "High",  { { "Station_LA", 177}, { "Store_TX", 106}, { "Center_TX", 873}, {"Store_AZ", 1528} } },
  };

  std::map<std::string, std::map<std::string, double>> variable_cost_map{
    { "Station_LA",  { { "Watson", 6739.725}, { "Bayshore", 10355.05}, { "Orange", 7650.40}} },
    { "Store_TX",  { { "Watson", 3204.8625}, { "Bayshore", 5457.075}, { "Orange", 3845.4} } },
    { "Center_TX",  { { "Watson", 4914}, { "Bayshore", 26409.6}, { "Orange", 19622.4} } },
    { "Store_AZ",  { { "Watson", 32372.1125}, { "Bayshore", 29982.225}, { "Orange", 21024.325} } },
  };

  // Load data into AMPL instances
  load_data(master, FACILITIES, CUSTOMERS, SCENARIOS, facility_capacity_map, customer_demand_map,
    variable_cost_map);
  load_master_data(master);
  load_data(sub, FACILITIES, CUSTOMERS, SCENARIOS, facility_capacity_map, customer_demand_map,
    variable_cost_map);

  // Export to AMPLS
  auto master_ampls = ampls::AMPLAPIInterface::exportModel<ampls::GurobiModel>(master);
  auto map = master_ampls.getVarMap();
  
  // Store some maps between the AMPL variables and the position in the solvers Jacobian
  int index_max_stage2_cost = map["max_stage2_cost"];
  std::map<std::string, int> index_facility_open;
  std::map<int, std::string> revindex_facility_open;
  for (auto f : FACILITIES) {
    int in = findIndexWithSubstring(map, f);
    index_facility_open[f] = in;      // map FACILITY -> solverindex facility_open[FACILITY]
    revindex_facility_open[in] = f;   // map solverindex facility_open[FACILITY] -> FACILITY
  }
  
  // Constraints of subproblems that we will use to extract dual/unbounded rays
  auto satisfying_customer_demand = sub.getConstraint("satisfying_customer_demand");
  auto facility_capacity_limits = sub.getConstraint("facility_capacity_limits");
  auto scenario_subproblem = sub.getParameter("ss");
  // Maps to accumulate the coefficients of the optimality and feasibility cuts
  std::map<std::string, double> fcl_feas, fcl_opt;
  // Scalars to accumulate the RHS of the cuts
  double optim_cut_rhs = 0, feas_cut_rhs = 0;

  // Maps to store indices and coefficients (used when actually adding cut using AMPLS)
  std::vector<int> indices;
  std::vector<double> coeffs;

  double newGap = std::numeric_limits<double>::infinity();
  double Gap = std::numeric_limits<double>::infinity();
  
  double epsilon = 0.00000001;
  int nOptimalityCuts = 0, nFeasibilityCuts = 0;

  // Main iterations loop
  for (int it = 0; it < 5; it++) {
    std::cout << std::endl << "******* Iteration " << it << "*******" << std::endl;

    for (auto f : FACILITIES)
    {
      fcl_feas[f] = 0;
      fcl_opt[f] = 0;
    }
    optim_cut_rhs = 0;
    feas_cut_rhs = 0;

    for (auto s : SCENARIOS) {
      scenario_subproblem.set(s); // set the scenario in the subproblem
      sub.getOutput("solve;"); // solve
      auto result = sub.getValue("solve_result").str();

      if (result == "infeasible") {
        // Accumulate coefficients and RHS for feasibility cut
        auto scd = satisfying_customer_demand.getValues("dunbdd");
        auto fcl = facility_capacity_limits.getValues("dunbdd");
        feas_cut_rhs += calculate_sum(scd, customer_demand_map[s]);
        for (auto r : fcl)
          fcl_feas[r[0].str()] -= r[1].dbl();
      }
      else
      {
        // Accumulate coefficients and RHS for optimality cut
        newGap -= sub.getValue("ProductionCost_Scenario").dbl();
        auto scd = satisfying_customer_demand.getValues("dual");
        auto fcl = facility_capacity_limits.getValues("dual");
        optim_cut_rhs += calculate_sum(scd, customer_demand_map[s]);
        for (auto r : fcl)
          fcl_opt[r[0].str()] -= r[1].dbl();
      }
    }
    // Add optimality cut
    if (optim_cut_rhs > 0) {
        nOptimalityCuts++;
        indices.clear();
        coeffs.clear();
        indices.push_back(index_max_stage2_cost);
        coeffs.push_back(1);
        for (auto f : FACILITIES)
        {
          if (fcl_opt[f] != 0) {
            indices.push_back(index_facility_open[f]);
            coeffs.push_back(fcl_opt[f] * facility_capacity_map[f]);
          }
        }
        auto opt = master_ampls.addConstraint(indices.size(), indices.data(), coeffs.data(), ampls::CutDirection::GE,
          optim_cut_rhs);
        master_ampls.record(opt);
        std::cout << "Added optimality cut:" <<std::endl << opt.toString() << std::endl;
    }

    // Add feasibility cut
    indices.clear();
    coeffs.clear();
    for (auto f : FACILITIES)
    {
      if (fcl_feas[f] != 0) {
        indices.push_back(index_facility_open[f]);
        coeffs.push_back(fcl_feas[f] * facility_capacity_map[f]);
      }
    }
    if (indices.size() > 0) {
      nFeasibilityCuts++;
      auto feas = master_ampls.addConstraint(indices.size(), indices.data(), coeffs.data(), ampls::CutDirection::GE,
        feas_cut_rhs);
      master_ampls.record(feas);
      std::cout << "Added feasibility cut:" << std::endl << feas.toString() << std::endl;
    }


    std::cout << "Resolving master problem" << std::endl;
    master_ampls.optimize();

    
    if (newGap > epsilon)
      Gap = std::max(Gap, newGap);
    else 
      break; // Desired gap achieved
    
    std::cout << "Gap = " << Gap << std::endl;

    // Set is_facility_open with the master solution, affected by feasibility cuts
    auto sol = master_ampls.getSolutionVector();
    std::vector<ampl::Tuple> ii;
    std::vector<double> vv;
    for (auto a : revindex_facility_open) {
      ii.push_back(ampl::Tuple(a.second));
      vv.push_back(sol[a.first]);
    }
    sub.getParameter("is_facility_open").setValues(ii.data(), vv.data(), ii.size());

    // Set newGap to the value of max_stage2_cost
    newGap = sol[index_max_stage2_cost];
  }
  std::cout << "Gap = " << newGap << std::endl;
  std::cout << "Optimal solution found, cost: " << master_ampls.getObj() << std::endl;

  // Import the AMPLS model back to AMPL, and show the additional cuts
  ampls::AMPLAPIInterface::importModel(master, master_ampls);
  std::cout << "Optimal solution in AMPL:" << master.getObjective("TotalCost").value() << std::endl;
  master.eval("expand {c in 1.._ncons} _con[c];");
  
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