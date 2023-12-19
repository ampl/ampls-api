#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.realpath(__file__)))

from amplpy import AMPL
import amplpy_gurobi as ampls
import os
from pprint import pprint

MUTE = True

## Add mipgap monitoring and/or stopping criteria
DESIREDGAP = 0.1

def load_model(modelfile : str) -> AMPL:
    a = AMPL()
    a.read(modelfile)
    return a

class Callback(ampls.GurobiCallback):
    def __init__(self):
        super(Callback, self).__init__()
        self.calls = {w: 0 for w in ampls.Where}
        self.not_mapped = []
        self.last_bound = 10e10

    def _evaluate_objs(self, mipsol : bool):
        obj_bound = self.get_value(ampls.Value.MIP_OBJBOUND)
        gap = self.get_value(ampls.Value.MIP_RELATIVEGAP)
        if not mipsol: # print only if bound has improved
            if self.last_bound == obj_bound: return 0
            self.last_bound = obj_bound;
        count=0
        #ccc=self.get_solution_dict()
        #for key, value in self.get_solution_dict().items():
        #    if value != 0: count += 1
        print(f"Current objective: {self.getObj()} - bound: {obj_bound} - relgap: {gap} - nonzeroes: {count}")
        if gap < DESIREDGAP:
            print("Desired gap reached, terminating")
            return -1
        return 0

    def run(self):
        t = self.get_ampl_where()
        self.calls[t] += 1
        if not MUTE:
            print("AMPLS Phase: {}, from solver: {}".format(t, self.get_where_string()))
        if t == ampls.Where.MSG:
            if not MUTE:
                print(self.get_message())
        if t == ampls.Where.LPSOLVE:
            if not MUTE:
                print(f"LP solve, {self.get_value(ampls.Value.ITERATIONS)} iterations")
            return 0
        if t == ampls.Where.PRESOLVE:
            if not MUTE:
                print(
                    "Presolve, eliminated {} rows and {} columns.".format(
                        self.get_value(ampls.Value.PRE_DELROWS),
                        self.get_value(ampls.Value.PRE_DELCOLS),
                    )
                )
            return 0
        if t == ampls.Where.MIPNODE:
            if not MUTE:
                print(f"New MIP node, count {self.get_value(ampls.Value.MIP_NODES)}")
            return self._evaluate_objs(False)
        if t == ampls.Where.MIPSOL:
            return self._evaluate_objs(False)
        if t == ampls.Where.NOTMAPPED:
            self.not_mapped.append(self.getWhereString())
        return 0



cb = Callback()
ampl = load_model("queens.mod")
model = ampl.to_ampls("gurobi")
cb.model=model
model.set_callback(cb)
model.optimize()
obj = model.get_obj()

if abs(158 - obj) >= DESIREDGAP:
    print("Error")
ampl.import_solution(model)
ampl_obj = ampl.get_current_objective().value()

if abs(158 - ampl_obj) >= DESIREDGAP:
    print("Error")

pprint(cb.calls)

