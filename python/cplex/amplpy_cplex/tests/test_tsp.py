#!/usr/bin/env python
# -*- coding: utf-8 -*-
from tsp_helpers import tsp_model, ford_fulkerson, UnionFind
from test_base import TestBase
import unittest
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))

import amplpy_cplex as ampls
SOLVER = "cplex"

var2tuple = ampls.var2tuple
tuple2var = ampls.tuple2var

VERBOSE = True
ENABLE_MTZ = False

ENABLE_CB_MIPNODE = True
if ENABLE_MTZ:
    ENABLE_CB_MIPSOL = False  # not necessary if MTZ cuts are in place
else:
    ENABLE_CB_MIPSOL = True  # must add sub-tour elimination constraints


class my_callback(ampls.GenericCallback):
    CALL_COUNT_MIPSOL = 0
    CALL_COUNT_MIPNODE = 0

    def set_data(self, var_map, xvars, vertices):
        self.var_map = var_map
        self.xvars = xvars
        self.vertices = vertices

    def mipsol(self):
        self.CALL_COUNT_MIPSOL += 1
        sol = self.get_solution_vector()
        nv = sum(abs(x) > 1e-5 for x in sol)
        if VERBOSE:
            print("MIPSOL #{}, nnz={}".format(self.CALL_COUNT_MIPSOL, nv))

        values = {self.xvars[i]: sol[i] for i in self.xvars if abs(sol[i]) > 1e-5}
        uf = UnionFind()
        for u, v in values:
            uf.link(u, v)
        groups = uf.groups()
        if len(groups) == 1:
            print("Valid solution!")
            for grp in groups:
                if VERBOSE:
                    print("> sub-tour: ", grp)
        else:
            for grp in groups:
                if VERBOSE:
                    print("> sub-tour: ", grp)
                cutvarnames = [tuple2var("x", i, j) for i in grp for j in grp if i != j]
                coeffs = [1 for i in range(len(cutvarnames))]
                
                self.addLazy(cutvarnames, coeffs, ampls.CutDirection.LE, len(grp) - 1)
        return 0

    def mipnode(self):
        self.CALL_COUNT_MIPNODE += 1
        if VERBOSE:
            print("MIPNODE #{}!".format(self.CALL_COUNT_MIPNODE))
        if self.CALL_COUNT_MIPNODE >= 1000:
            return 1
        sol = self.get_solution_vector()
        capacities = {self.xvars[i]: sol[i] for i in self.xvars if abs(sol[i]) > 1e-5}
        for i in range(1, len(self.vertices)):
            max_flow, partition = ford_fulkerson(
                capacities, self.vertices[0], self.vertices[i]
            )
            if max_flow <= 1 - 1e-5:
                p1 = set(partition)
                p2 = set(self.vertices) - p1
                min_cut = sum(capacities.get((i, j), 0) for i in p1 for j in p2)
                cutvarnames = [tuple2var("x", i, j) for i in p1 for j in p2]
                coeffs = [1 for i in range(len(cutvarnames))]
                self.add_cut(cutvarnames, coeffs, ampls.CutDirection.GE, 1)
                print(
                    "> max-flow: {}, min-cut: {}, must be == 1".format(
                        max_flow, min_cut
                    )
                )
                return 0
        return 0

    def run(self):
        try:
          
            if ENABLE_CB_MIPSOL and self.get_ampl_where()== ampls.Where.MIPSOL and self.can_do(ampls.CanDo.ADD_LAZY_CONSTRAINT):
                return self.mipsol()
            elif ENABLE_CB_MIPNODE and self.get_ampl_where() == ampls.Where.MIPNODE:
                return self.mipnode()
            else:
                return 0
        except Exception as e:
            print("Error:", e)
            return 1


class Test_Tsp(TestBase):
    def test_tsp(self):
        # Create model in AMPL
        ampl = tsp_model("data/tsp_40_1.txt", ENABLE_MTZ)
        # Export it to the solver
        m = ampl.to_ampls(SOLVER)
        # Display info
        print("Model loaded, nvars=", m.get_num_vars())

        if ENABLE_CB_MIPSOL:  # needs lazy constraints
            m.enable_lazy_constraints()

        # Get model data useful in callback
        var_map = dict(m.getVarMapFiltered("x"))
        xvars = {index: var2tuple(var)[1:] for var, index in var_map.items()}
        vertices = list(
            sorted(set([x[0] for x in xvars.values()] + [x[1] for x in xvars.values()]))
        )

        # Set up callback
        cb = my_callback()
        cb.set_data(var_map, xvars, vertices)
        m.setCallback(cb)

        # Implemeting multithreaded callbacks for CPLEX is more involving
        if SOLVER == "cplex":
            m.set_option("threads", 1)

        # Start optimization
        m.optimize()

        # Get results
        assert m.get_status() == ampls.Status.OPTIMAL
        obj = m.get_obj()
        nvars = m.get_num_vars()
        print("Solved for {} variables, objective {}".format(nvars, obj))

        # Import results to AMPL
        ampl.import_solution(m)
        # Get the result from AMPL and print all the costs of the used arcs
        arcs = ampl.get_data("{(a,b) in A : x[a,b] != 0} c[a,b]").to_pandas()
        print(arcs)
        self.assertAlmostEqual(ampl.getObjective("total").value(), 376.9665, 3)


if __name__ == "__main__":
    unittest.main()
