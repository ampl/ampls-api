def load_tsp_data(fname):
    """Load a TSP instance from a file."""
    xs, ys = [], []
    with open(fname) as f:
        lst = list(map(float, f.read().split()))
        n = int(lst.pop(0))
        for _ in range(n):
            xs.append(lst.pop(0))
            ys.append(lst.pop(0))
    return n, xs, ys


def tsp_model(tsp_data, enable_mtz=True):
    from amplpy import AMPL
    from math import sqrt
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

    if enable_mtz:
        ampl.eval('''
        # subtour elimination Miller, Tucker and Zemlin (MTZ) (1960)
        subject to MTZ{(i,j) in A: i != 1}: u[i]-u[j] + (n-1)*x[i,j] <= n-2;
        ''')

    n, xs, ys = load_tsp_data(tsp_data)
    dist = {
        (i+1, j+1): sqrt((xs[j] - xs[i])**2 + (ys[j] - ys[i])**2)
        for i in range(n) for j in range(n)
        if i != j
    }
    ampl.param['n'] = n
    ampl.param['c'] = dist
    return ampl


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
    partition = find_partition(graph, s, t)
    return max_flow, partition
