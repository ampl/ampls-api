from amplpy import AMPL, DataFrame
import amplpy_gurobi as ampls
from amplpy_gurobi.patch import  *
import time
import matplotlib.pyplot as plt
import matplotlib.colors as colors

import tsplib95 as tsp

tspAMPLModel = """set NODES ordered;
param hpos {NODES};
param vpos {NODES};

set PAIRS := {i in NODES, j in NODES: ord(i) < ord(j)};

param distance {(i,j) in PAIRS}
   := sqrt((hpos[j]-hpos[i])**2 + (vpos[j]-vpos[i])**2);

var X {PAIRS} binary;

minimize Tour_Length: sum {(i,j) in PAIRS} distance[i,j] * X[i,j];

subject to Visit_All {i in NODES}:
   sum {(i,j) in PAIRS} X[i,j] + sum {(j,i) in PAIRS} X[j,i] = 2;"""

# Set execution parameters
PLOTSUBTOURS = False
solver = "gurobi"
tspFile = "tsp/gr96.tsp"
# Load model in AMPL
ampl = AMPL()
ampl.eval(tspAMPLModel)
ampl.option["solver"]=solver


def getDictFromTspFile(tspFile):
  p = tsp.load(tspFile)
  if not p.is_depictable:
    printf("Problem is not depictable!")

  # Amendments as we need the nodes lexographically ordered
  nnodes = len(list(p.get_nodes()))
  i = 0
  while nnodes>1:
    nnodes = nnodes/10
    i+=1
  formatString = f"{{:0{i}d}}"
  nodes = {formatString.format(value) : p.node_coords[index+1] for index, value in enumerate(p.get_nodes())}
  return nodes

# Set problem data from tsp file
nodes = getDictFromTspFile(tspFile)

# Pass them to AMPL using a dataframe
df = DataFrame(index=[('NODES')], columns=['hpos', 'vpos'])
df.setValues(nodes)
ampl.setData(df, "NODES")

# Set some globals that never change during the execution of the problem
NODES = set(nodes.keys())
CPOINTS = {node : complex(coordinate[0], coordinate[1]) for (node, coordinate) in nodes.items()}



# Plot helpers
def plotTours(tours: list, points_coordinate: dict):
    # Plot all the tours in the list each with a different color
    colors = ['b', 'g', 'c', 'm', 'y', 'k']
    for i, tour in enumerate(tours):
        tourCoordinates = [points_coordinate[point.strip("'")] for point in tour]
        color = colors[i % len(colors)]
        plot_all(tourCoordinates, color = color)
    plt.show()

def plot_all(tour, alpha=1, color=None):
    # Plot the tour as blue lines between blue circles
    plotline(list(tour) + [tour[0]], alpha=alpha, color=color)
    plotline([tour[0]], 's', alpha=alpha, color=color)
    
def plotline(points, style='o-', alpha=1, color=None):
    "Plot a list of points (complex numbers) in the 2-D plane."
    X, Y = XY(points)
    if color:
        plt.plot(X, Y, style, alpha=alpha, color=color)
    else:
        plt.plot(X, Y, style, alpha=alpha)
    
def XY(points):
    "Given a list of points, return two lists: X coordinates, and Y coordinates."
    return [p.real for p in points], [p.imag for p in points]


# Graphs helper routines
def trasverse(node, arcs : set, allnodes : set, subtour = None) -> list:
   # Trasverses all the arcs in the set arcs, starting from node
   # and returns the tour
   if not subtour:
     subtour = list()
   # Find arcs involving the current node
   myarcs = [(i,j) for (i,j) in arcs if node == i or node == j]
   if len(myarcs) == 0:
     return 
   # Append the current node to the current subtour
   subtour.append(node)

   # Use the first arc found
   myarc = myarcs[0]
   # Find destination (or origin) node
   destination = [(i) for i in myarc if i != node][0]
   # Remove from arcs and nodes to visit
   arcs.remove(myarc)
   if node in allnodes:
      allnodes.remove(node)

   trasverse(destination, arcs, allnodes, subtour)
   return subtour

def findSubTours(arcs : set, allnodes : set):
   """Find all the subtours defined by a set of arcs and
      return them as a list of list
   """
   subtours = list()
   while len(allnodes) > 0:
     l = trasverse(next(iter(allnodes)), arcs, allnodes)
     subtours.append(l)
   if len(subtours) > 1:
     print(f"Found {len(subtours)} subtours:")
    # for st in subtours:
    #    print(st)
   return subtours

 
# Global variables to store entities needed by the callbacks
xvars = None
xinverse = None
vertices = None



# Callback class that actually add the cuts if subtours are found in a solution
class MyCallback(ampls.GurobiCallback):
   def __init__(self):
     super().__init__()
     self.iteration = 0


   def run(self):
     try:
        # For each solution
        if self.getAMPLWhere() == ampls.Where.MIPSOL:
          self.iteration += 1
          print(f"\nIteration {self.iteration}: Finding subtours")
          sol = self.getSolutionVector()
          arcs = [xvars[i] for i,value in enumerate(sol) if value > 0]
          subTours = findSubTours(set(arcs), set(vertices))
          if len(subTours) ==1:
            print("No subtours detected.")
            return 0
          print(f"Adding {len(subTours)} cuts")
          if PLOTSUBTOURS:
            plotTours(subTours, CPOINTS)
          for subTour in subTours:
            st1 = set(subTour)
            nst1 = set(vertices) - st1
            externalArcs = [(i,j) if i < j else (j,i) for i in st1 for j in nst1]
            varsExternalArcs = [xinverse[(i,j)] for (i,j) in externalArcs]
            coeffs = [1 for i in range(len(varsExternalArcs))]
            varsExternalArcs = sorted(varsExternalArcs)
            if False:
              print(f"Adding cut {varsExternalArcs}")
            self.addLazyIndices(varsExternalArcs , coeffs,
                                  ampls.CutDirection.GE, 2)
            if len(subTours) == 2:
              return 0
          print("Continue solving")
        return 0
     except Exception as e:
       print('Error:', e)
       return 0

# Export the model using ampls
model = ampl.exportModel(solver)
grbModel = model.getGRBmodel()

GRBwrite(grbModel, "d:\\tspg96.lp");
GRBwrite(grbModel, "d:\\tspg96.mps");

model.enableLazyConstraints()

# Get the global maps between solver vars and AMPL entities
varMap = model.getVarMapFiltered("X")
inverse = model.getVarMapInverse()
xvars = {
  index: var2tuple(var)[1:] for var, index in varMap.items()}
xinverse = {
  var2tuple(var)[1:] : index for index, var in inverse.items()}
vertices = list(sorted(set([x[0] for x in xvars.values()] + [x[1] for x in xvars.values()])))

# Assign the callback
callback = MyCallback()
model.setCallback(callback)
# Start the optimization
model.optimize()
# Import the solution back to AMPL
ampl.importSolution(model)


# Get the solution into ARCS
ARCS = ampl.getData("{(i,j) in PAIRS : X[i,j]>0} X[i,j];")
ARCS = set([(i,j) for (i,j,k)in ARCS.toList()])
# Print it
print(ARCS)
# Display it
tours = findSubTours(ARCS, NODES)
plotTours(tours, CPOINTS)