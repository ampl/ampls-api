from amplpy import AMPL
from math import sqrt
import random
import grbpy_c as gpy
import os
import time


VERBOSE = True
ENABLE_CALLBACK = True
ENABLE_MTZ = False
ENABLE_CB_MIPNODE = True


if ENABLE_MTZ:
    ENABLE_CB_MIPSOL = False  # not necessary if MTZ cuts are in place
else:
    ENABLE_CB_MIPSOL = True  # must add sub-tour elimination constraints

def toAMPLIndex(index):
  try:
    val = float(index)
    return str(index)
  except ValueError:
    return '\'{}\''.format(index)

def toAMPLName(*args):
  if len(args) == 1:
    return args[0]
  return args[0] + '['+ ','.join(toAMPLIndex(x) for x in args[1:]) + ']'

def getVarToIndexDict(m, varName):
  dmap= m.getVarMapFiltered(varName);
  return {k:v for k,v in dmap.iteritems()}

def read_tsp(fname):
    """Load a TSP instance from a file."""
    xs, ys = [], []
    with open(fname) as f:
        lst = list(map(float, f.read().split()))
        n = int(lst.pop(0))
        for i in range(n):
            xs.append(lst.pop(0))
            ys.append(lst.pop(0))
    return n, xs, ys


ampl = AMPL()
ampl.eval('''
param n;
set V := 1..n;
set A := {(i,j) in V cross V : i != j};
param c{A} >= 0 default Infinity;

var x{A}, binary;
var u{V} >= 0;

minimize total: sum{(i,j) in A} c[i,j] * x[i,j];
s.t. enter{j in V}: sum{i in V: i != j} x[i, j] == 1;
s.t. leave{i in V}: sum{j in V: j != i} x[i, j] == 1;
''')

if ENABLE_MTZ:
    ampl.eval('''
    # subtour elimination Miller, Tucker and Zemlin (MTZ) (1960)
    subject to MTZ{(i,j) in A: i != 1}: u[i]-u[j] + (n-1)*x[i,j] <= n-2;
    ''')

n, xs, ys = read_tsp("../../../../../../test/models/tsp_51_1.txt")
dist = {
    (i+1, j+1): sqrt((xs[j] - xs[i])**2 + (ys[j] - ys[i])**2)
    for i in range(n) for j in range(n)
    if i != j
}

V = list(range(1, n + 1))
ampl.param['n'] = n
ampl.param['c'] = dist
ampl.option['auxfiles'] = 'c'
ampl.eval('write gmodel;')
os.environ['gurobi_options'] = 'outlev=1 timelim=60'
d= gpy.GurobiDrv()
m = d.loadModel('model.nl')

if ENABLE_CB_MIPSOL:  # needs lazy constraints
    gpy.GRBsetintparam(m.getGRBenv(), gpy.GRB_INT_PAR_LAZYCONSTRAINTS, 1)

varMap = getVarToIndexDict(m, "x")
sortedvarMap = sorted(varMap.items(), key=lambda kv: kv[1])
xvars = {}
for i in range(len(varMap)):
  var = sortedvarMap[i][0]
  xvars[i] = tuple(var[var.find('[')+1: -1].split(','))

vertices = list(
    sorted(
        set(
             [x[0] for x in xvars.values()] + [x[1] for x in xvars.values()]
        )
    )
)

class UnionFind(object):
    def __init__(self):
        self.p = {}
        self.rank = {}

    def find(self, x):
        if x not in self.p:
            self.p[x] = x
            self.rank[x] = 0
        elif self.p[x] != x:
            self.p[x] = self.find(self.p[x])
        return self.p[x]

    def link(self, x, y):
        x = self.find(x)
        y = self.find(y)
        if x != y:
            if self.rank[x] > self.rank[y]:
                self.p[y] = x
            else:
                self.p[x] = y
                if self.rank[x] == self.rank[y]:
                    self.rank[y] += 1

    def groups(self):
        from collections import defaultdict
        grps = defaultdict(list)
        for x in self.p:
            grps[self.find(x)].append(x)
        return list(grps.values())


def get_adj(graph):
    adj = {}
    for (i, j), cap in graph.items():
        if cap == 0:
            continue
        if i in adj:
            adj[i].append(j)
        else:
            adj[i] = [j]
    return adj


def bfs(graph, s, t):
    adj = get_adj(graph)
    Q = [s]
    pred = {}
    visited = set()
    marked = set([s])
    while Q != []:
        u = Q.pop()
        visited.add(u)
        if u == t:
            break
        if u not in adj:
            continue
        for v in adj[u]:
            if v not in visited and v not in marked:
                pred[v] = u
                Q.append(v)
                marked.add(v)
    return visited, pred


def find_augmenting_path(graph, s, t):
    visited, pred = bfs(graph, s, t)
    if t not in visited:
        return 0, None
    path = []
    u = t
    flow = float('inf')
    while u != s:
        path.append(u)
        flow = min(flow, graph[pred[u], u])
        u = pred[u]
    path.append(s)
    return flow, list(reversed(path))


def find_min_cut(graph, s, t):
    visited, pred = bfs(graph, s, t)
    assert t not in visited
    return [
        (u, v)
        for (u, v) in graph
        if u in visited and v not in visited
    ]


def find_partition(graph, s, t):
    visited, pred = bfs(graph, s, t)
    assert t not in visited
    return list(visited)


def ford_fulkerson(capacities, s, t):
    max_flow = 0
    graph = dict(capacities.items())
    for (i, j) in capacities:
        if (j, i) not in capacities:
            graph[j, i] = 0
    while True:
        flow, path = find_augmenting_path(graph, s, t)
        if flow == 0:
            break
        max_flow += flow
        for i in range(1, len(path)):
            u, v = path[i-1], path[i]
            graph[u, v] -= flow
            graph[v, u] += flow
    # min_cut = [
    #     (u, v) for u, v in find_min_cut(graph, s, t)
    #     if (u, v) in capacities and capacities[u, v] > 0
    # ]
    # assert sum(capacities[u, v] for u, v in min_cut) == max_flow
    # return max_flow, min_cut
    partition = find_partition(graph, s, t)
    return max_flow, partition


class MyCallback(gpy.GRBCallback):
    CALL_COUNT_MIPSOL = 0
    CALL_COUNT_MIPNODE = 0

    def mipsol(self):
        self.CALL_COUNT_MIPSOL += 1
        if VERBOSE:
            print("GRB_CB_MIPSOL #{}!".format(self.CALL_COUNT_MIPSOL))
        sol = self.getSolutionVector()
        values = {
            xvars[i]: sol[i]
            for i in xvars
            if abs(sol[i]) > 1e-5
        }
        uf = UnionFind()
        for (u, v) in values:
            uf.link(u, v)
        groups = uf.groups()
        if len(groups) == 1:
            print('Valid solution!')
        else:
            cutvarsoriginal = []
            cutvarsmy = []
            for grp in groups:
                print('> sub-tour: ', grp)
                cutvarnames = [toAMPLName('x', i,j) for i in grp for j in grp if i != j] 
                coeffs = [1 for i in range(len(cutvarnames))]
                self.addLazy(cutvarnames, coeffs, gpy.GRB_LESS_EQUAL, len(grp)-1)
        return 0

    def mipnode(self):
        self.CALL_COUNT_MIPNODE += 1
        if VERBOSE:
            print("GRB_CB_MIP #{}!".format(self.CALL_COUNT_MIPNODE))
        if self.CALL_COUNT_MIPNODE >= 1000:
            return 1
        sol = self.getSolutionVector()
        capacities = {
            xvars[i]: sol[i]
            for i in xvars
            if abs(sol[i]) > 1e-5
        }
        for i in range(1, len(vertices)):
            max_flow, partition = ford_fulkerson(
                capacities, vertices[0], vertices[i]
            )
            if max_flow <= 1-1e-5:
                p1 = set(partition)
                p2 = set(vertices) - p1
                min_cut = sum(
                    capacities.get((i, j), 0)
                    for i in p1 for j in p2
                )
                cutvarnames = [toAMPLName('x', i,j) for i in p1 for j in p2] 
                coeffs = [1 for i in range(len(cutvarnames))]
                self.addCut(cutvarnames, coeffs, gpy.GRB_GREATER_EQUAL, 1)
                print('> max-flow: {}, min-cut: {}, must be == 1'.format(
                        max_flow, min_cut))
                return 0
        return 0

    def run(self, model, cbdata, where):
        try:
              if ENABLE_CB_MIPSOL and where == gpy.GRB_CB_MIPSOL:
                return self.mipsol()
              elif ENABLE_CB_MIPNODE and where == gpy.GRB_CB_MIPNODE:
                return self.mipnode()
              else:
                return 0
        except Exception as e:
            print('Error:', e)
            return 1


if ENABLE_CALLBACK:
    cb = MyCallback()
    m.setCallback(cb)

m.optimize()
obj = m.getObj()
nvars = m.getNumVars()
m.writeSol()


ampl.eval("solution model.sol;")
print("Solved for {} variables, objective {}".format(nvars, obj))
ampl.display('total')