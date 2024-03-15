#!/usr/bin/env python
# -*- coding: utf-8 -*-
from calendar import c
from json.encoder import INFINITY
from amplpy import AMPL, variable
import pandas as pd

import amplpy_gurobi as ampls 
SOLVER = "gurobi"

def create_extended_form() ->AMPL:
    a = AMPL()
    a.eval("""
                set FACILITIES; 
                set CUSTOMERS;  
                set SCENARIOS;  

                var facility_open{FACILITIES} binary;                   
                var production{FACILITIES, CUSTOMERS, SCENARIOS} >= 0;  

                param fixed_cost{FACILITIES} >= 0;                
                param variable_cost{FACILITIES, CUSTOMERS} >= 0; 
                param customer_demand{CUSTOMERS, SCENARIOS} >= 0; 
                param facility_capacity{FACILITIES} >= 0;        
                param prob{SCENARIOS} default 1/card(SCENARIOS);  


                minimize TotalCost: 
                    sum{i in FACILITIES} fixed_cost[i] * facility_open[i] +                                                             
                    sum{s in SCENARIOS, i in FACILITIES, j in CUSTOMERS} prob[s] * variable_cost[i,j] * production[i,j,s];  

                s.t. satisfying_customer_demand{s in SCENARIOS, j in CUSTOMERS}:
                    sum{i in FACILITIES} production[i,j,s] >= customer_demand[j,s];

                s.t. facility_capacity_limits{s in SCENARIOS, i in FACILITIES}:
                    sum{j in CUSTOMERS} production[i,j,s] <= facility_capacity[i] * facility_open[i];

                s.t. sufficient_production_capacity:
                    sum{i in FACILITIES} facility_capacity[i]*facility_open[i] >= max{s in SCENARIOS} sum{j in CUSTOMERS} customer_demand[j,s];  
           """)
    a.option["solver"]=SOLVER
    return a

def set_data(a: AMPL, is_master: bool):
    a.eval("""data; set FACILITIES  := Watson Bayshore Orange Evanston;
            set CUSTOMERS   := CEM_Store_41594 CEM_Cypress_Market CEM_Time_Mart_23 CEM_Cypress_CEM CEM_Fuel_Maxx_84 CEM_Commercial_CEM CEM_New_Brunswick CEM_Bernards_PGroup;
            set SCENARIOS   := Low Medium High;
            param variable_cost (tr):       Watson      Bayshore        Orange              Evanston    :=
                    CEM_Store_41594         6739.72500  10355.05000     7650.40000          5219.50000
                    CEM_Cypress_Market      4739.72500  8355.05000      17320.40000         15000.50000
                    CEM_Time_Mart_23        7739.72500  9355.05000      7320.40000          5000.50000
                    CEM_Cypress_CEM         5739.72500  6355.05000      7320.40000          9433.70000
                    CEM_Fuel_Maxx_84        9739.72500  10355.05000     9650.40000          5219.50000
                    CEM_Commercial_CEM      5093.25000  11355.05000     5320.40000          10433.70000
                    CEM_New_Brunswick       4987.72500  8935.05000      7320.40000          5875.50000
                    CEM_Bernards_PGroup     8011.72500  8300.05000      12110.40000         10000.50000;

            param customer_demand:          Low         Medium          High    :=
                    CEM_Store_41594         10000       15000           20000
                    CEM_Cypress_Market      10000       15000           20000
                    CEM_Time_Mart_23        10000       15000           20000
                    CEM_Cypress_CEM         10000       15000           20000
                    CEM_Fuel_Maxx_84        10000       15000           20000
                    CEM_Commercial_CEM      10000       15000           20000
                    CEM_New_Brunswick       10000       15000           20000
                    CEM_Bernards_PGroup     10000       15000           20000;

            param facility_capacity := Watson 75000 Bayshore 200000 Orange 150000 Evanston 90000;
            """)
    if is_master:
        a.eval("param fixed_cost := Watson 39268000 Bayshore 65268000 Orange 60268000 Evanston 48268000;")
   
def create_common() -> AMPL:
    a = AMPL();
    a.option["solver"]="cplex"
    a.option["cplex_options"]="presolve 0 dualopt"
    a.eval("set FACILITIES;"
    "set CUSTOMERS;"
    "set SCENARIOS;"
    "param prob{SCENARIOS} default 1/card(SCENARIOS);"
    "param sub_scenario symbolic in SCENARIOS;"
    "param customer_demand{CUSTOMERS, SCENARIOS} >= 0;"

    "param facility_capacity{FACILITIES} >= 0;"
    "param variable_cost{ FACILITIES, CUSTOMERS } >= 0;")
    return a
def create_sub_problem() ->AMPL:
  a = create_common()
  a.eval("var production{FACILITIES, CUSTOMERS, SCENARIOS} >= 0;"
        "param sub_facility_open{ FACILITIES } default 1; "
        "minimize operating_cost:"
        "sum{ i in FACILITIES, j in CUSTOMERS }"
        "variable_cost[i, j] * production[i, j, sub_scenario]; "

        "s.t.satisfying_customer_demand{ j in CUSTOMERS }:"
        "sum{i in FACILITIES} production[i, j, sub_scenario] >= customer_demand[j, sub_scenario];"

        "s.t.facility_capacity_limits{i in FACILITIES}:"
        "sum{ j in CUSTOMERS } production[i, j, sub_scenario] <= facility_capacity[i] * sub_facility_open[i];")
  return a

def create_master_problem() ->AMPL:
  a=create_common()
  a.eval(
      """param fixed_cost{ FACILITIES } >= 0;
         var sub_variable_cost{SCENARIOS} >= 0;
         var facility_open{ FACILITIES } binary;
         minimize TotalCost :
            sum{ i in FACILITIES } fixed_cost[i] * facility_open[i] + sum{s in SCENARIOS} prob[s]*sub_variable_cost[s];
         s.t. sufficient_production_capacity:
            sum{i in FACILITIES} facility_capacity[i]*facility_open[i] >= max{s in SCENARIOS} sum{j in CUSTOMERS} customer_demand[j,s];
        """)
  return a


def get_data_from_ampl(a: AMPL) -> tuple:
    """Get data we need in the algorithm from AMPL"""
    facilities=a.set["FACILITIES"].get_values().to_list()
    customers=a.set["CUSTOMERS"].get_values().to_list()
    scenarios=a.set["SCENARIOS"].get_values().to_list()
    facility_capacity=a.param["facility_capacity"].get_values().to_dict()
    customer_demand = a.param["customer_demand"].get_values().to_dict()
    variable_cost = a.param["variable_cost"].get_values().to_dict()
    return (facilities, customers, scenarios, facility_capacity, customer_demand, variable_cost)
    
def get_tuple_map(model: ampls.AMPLModel, varname: str):
    '''Get a map indexingsetitem -> index for a specified variable'''
    var_map = model.getVarMapFiltered(varname)
    beg = len(varname) + 2
    return { name[beg:-2] : value for name,value in var_map.items()} 

def get_maps(model) -> tuple:
    '''Get all the maps we will need for the execution'''
    index_facility_open = get_tuple_map(model, "facility_open")
    index_sub_variable_cost = get_tuple_map(model, "sub_variable_cost")
    revindex_facility_open = {value : name for name,value in index_facility_open.items()}
    revindex_sub_variable_cost = {value : name for name,value in index_sub_variable_cost.items()}
    return (index_facility_open, revindex_facility_open, index_sub_variable_cost, revindex_sub_variable_cost)

   
def add_benders_cut(model: ampls.AMPLModel, cdduals: dict, fcduals: dict, 
                    index_facility_open: dict, index_sub_variable_cost: dict,
                    facility_capacity: dict, customer_demand: dict, 
                    s: str, customers: list, cutype: str):
    """Add a Bender's cut to the model."""
    # Note that to accomodate for solver view, we formulate it as below
    # sub_variable_cost[s]-sum{i in FACILITIES} facility_price[i,s,k]*facility_capacity[i]*facility_open[i]
    # >=   sum{j in CUSTOMERS} customer_price[j,s,k]*customer_demand[j,s]; 
    indices = [index_facility_open[f] for f in fcduals.keys()]
    coeffs = [-fcduals[f] * facility_capacity[f] for f in fcduals.keys()]
    if cutype=="optimality":
        indices.append(index_sub_variable_cost[s])
        coeffs.append(1)
    rhs = sum(cdduals[c] * customer_demand[(c,s)] for c in customers)
    opt = model.addConstraint(indices, coeffs, ampls.CutDirection.GE, rhs)
    # With the following, we keep track of the cuts we add, to then add them back
    # into AMPL
    model.record(opt);
    print(f"Added {cutype} cut: {opt.toString()}\n")
    
def doStuff(set_data_function):
    '''Generic function doing the optimization and reading the results'''
    master=create_master_problem()
    sub=create_sub_problem()
    
    set_data_function(sub, False)
    set_data_function(master, True)
    
    # Get the data we will use for iterations
    (facilities, customers, scenarios, facility_capacity, customer_demand, variable_cost)=get_data_from_ampl(master)
    
    # Set some options
    master.option["presolve"]=0
    sub.eval("suffix dunbdd;")
    sub.option["presolve"]=0 # set else AMPL likely presolves the variable facility_open
    
    # Export the master problem to ampls
    master_ampls = master.to_ampls(SOLVER)
    # Get maps between solver vars and AMPL entities
    (index_facility_open, revindex_facility_open, index_sub_variable_cost, revindex_sub_variable_cost)=get_maps(master_ampls)
    
    # Constraints of subproblems that we will use to extract dual/unbounded rays
    # Could do without but this makes the implementation slightly cleaner
    satisfying_customer_demand = sub.con["satisfying_customer_demand"]
    facility_capacity_limits = sub.con["facility_capacity_limits"]
    
    # Initialize Benders's counter and params
    epsilon = 0.00000001
    sub_variable_cost = {s : 0 for s in scenarios}
    it =0
    while True:
       it+=1
       print(f"****** Iteration {it} *******")
       nviolations = 0
       for s in scenarios:
           # Solve the subproblem
           sub.param["sub_scenario"]=s # Assign scenario
           sub.get_output("solve;")
           result = sub.get_value("solve_result")
           print(f"Scenario {s} Objective={sub.get_current_objective().value()}")
           # Decide what cut to add, if any
           if (result == "infeasible"):
                cdduals = satisfying_customer_demand.get_values("dunbdd").to_dict();
                fcduals = facility_capacity_limits.get_values("dunbdd").to_dict()
                add_benders_cut(master_ampls, cdduals, fcduals, index_facility_open, index_sub_variable_cost,
                                    facility_capacity, customer_demand, s, customers, "feasibility")
           elif sub.getValue("operating_cost") > sub_variable_cost[s] + epsilon:
               cdduals = satisfying_customer_demand.get_values("dual").to_dict()
               fcduals = facility_capacity_limits.get_values("dual").to_dict()
               add_benders_cut(master_ampls, cdduals, fcduals, index_facility_open, index_sub_variable_cost,
                                    facility_capacity, customer_demand, s, customers, "optimality")
           else:
               nviolations+=1
               
       # It no scenario had a violation, we exit
       if nviolations == len(scenarios):
           break;
       
       print("SOLVING MASTER PROBLEM")
       master_ampls.optimize();
    
       # Get the solution vector from ampls
       sol = master_ampls.getSolutionVector()
       # AMPL: let {f in FACILITIES} sub_facility_open[f] := facility_open[f];
       sub.param["sub_facility_open"]={f : sol[i] for i,f in revindex_facility_open.items()}
       # Assign the costs from the master problem for next iteration
       sub_variable_cost =            {s : sol[i] for i,s in revindex_sub_variable_cost.items()}
    # End of Bender's loop
   
    print(f"Optimal solution found, cost: {master_ampls.get_obj()}\n\n")
    # Import the AMPLS model back to AMPL, and show the additional cuts
    master.import_solution(master_ampls, keep_files=True)
    master.eval(master_ampls.getRecordedEntities())
    master_obj=master.get_current_objective().value()
    master.eval("expand {c in 1.._ncons} _con[c];")
    print(f"Optimal solution benders: {master_obj}")    
    
    # Check with the extended form
    extended = create_extended_form()
    set_data_function(extended, True)
    extended.solve()
    extended_obj = extended.get_current_objective().value()
    print(f"Optimal solution extended: {extended_obj}")
    if abs(extended_obj-master_obj)>10e-4:
        print(f"Not good at all, discrepancy of {abs(extended_obj-master_obj)} found!!!")
        print(f"extended_obj={extended_obj} master_obj={master_obj}")
        
doStuff(set_data)