import sys
from tsp_helpers import tsp_model
# import amplpy_ampls as ampls
import amplpy_gurobi as ampls_gurobi
import amplpy_cplex as ampls_cplex
ampls = ampls_gurobi

# Define my generic callback function


class MyCallback(ampls.GenericCallback):
    def __init__(self):
        super(MyCallback, self).__init__()
        self.nMIPnodes = 0

    def run(self):
        try:
            t = self.getAMPLWhere()
            print("AMPL Phase: {}, from solver: {}".format(
                t, self.getWhereString()))
            if t == ampls.Where.MSG:
                # print(self.getMessage())
                return 0
            if t == ampls.Where.LPSOLVE:
                print("LP solve, {} iterations".format(
                    self.getValue(ampls.Value.ITERATIONS).integer))
                return 0
            if t == ampls.Where.PRESOLVE:
                print("Presolve, eliminated {} rows and {} columns.".format(
                    self.getValue(ampls.Value.PRE_DELROWS).integer,
                    self.getValue(ampls.Value.PRE_DELCOLS).integer))
                return 0
            if t == ampls.Where.MIPNODE:
                self.nMIPnodes += 1
                print("New MIP node, count {}".format(self.nMIPnodes))
            if t == ampls.Where.MIPSOL:
                print("MIP Solution = {}".format(self.getObj()))
        except Exception as e:
            print(e)
        return 0


class MyCplexCallback(ampls_cplex.CPLEXCallback):
    def run(self):
        try:
            print('>>', dir(self))
            w = self.getWhere()
            #where = self.getWhereText()
            print(w)
            return 0
        except Exception as e:
            print(e)


class MyGurobiCallback(ampls_gurobi.GurobiCallback):
    def run(self):
        print(dir(self))
        w = self.getWhere()
        #where = self.getWhereText()
        print(w)
        return 0


def doStuff(a, cb):
    cb = MyCallback()
    a.setCallback(cb)
    a.optimize()
    print(a.getObj())


ampl = tsp_model('tsp_51_1.txt')

print('Cplex with generic callback:')
cm = ampl.exportCplexModel()
doStuff(cm, MyCallback())

print('Cplex with cplex callback:')
cm = ampl.exportCplexModel()
doStuff(cm, MyCplexCallback())

print('Gurobi with generic callback:')
gm = ampl.exportGurobiModel()
doStuff(gm, MyCallback())

print('Gurobi with gurobi callback:')
gm = ampl.exportGurobiModel()
doStuff(gm, MyGurobiCallback())
