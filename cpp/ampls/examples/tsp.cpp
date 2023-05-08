#include <list>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <regex>
#include <iostream>
#include <exception>

#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#ifdef USE_amplapi
    #include "ampl/ampl.h"
#endif


const double INTTOLERANCE = 1e-4;

std::vector<std::string> split(const std::string str, const std::string regex_str)
{
  const std::regex regex(regex_str);
  std::smatch matches;
  std::vector<std::string> ret;

  if (std::regex_search(str, matches, regex)) {
    for (size_t i = 0; i < matches.size(); ++i) {
      ret.push_back(matches[i].str());
    }
  }
  else 
    throw std::runtime_error("Match not found");
  return ret;
}

struct GraphArc {
  int from;
  int to;
  GraphArc() : from(0), to(0) {}
  GraphArc(int from, int to)
  {
    this->from = from;
    this->to = to;
  }
  static GraphArc fromVar(const std::string& var)
  {
  #if __cplusplus >= 201103L
    auto s = split(var, "[A-Za-z0-9]*\\['([A-Za-z0-9]*)','([A-Za-z0-9]*)']");
  #else
    auto s = split(var, "[A-Za-z0-9]*\\[[']*([A-Za-z0-9]*)[']*,[']*([A-Za-z0-9]*)[']*\\]");
  #endif
    return GraphArc(std::stoi(s[1]), std::stoi(s[2]));
  }

  friend std::ostream& operator<<(std::ostream& out, const GraphArc& c)
  {
    out << '(' << c.from << "->" << c.to << ')';
    return out;
  }

  /**
  * Find arc connected to the passed one.
  * If it finds one, it changes the arc parameter to the next arc
  * in the sequence and returns true. Returns false otherwise.
  */
  static bool findArcFromTo(const std::list<GraphArc>& arcs, GraphArc& arc)
  {
    for (auto other : arcs) // Find next arc in the sequence
      // since they are not directed, any "from" can be connected
      // to any "to"
    {
      if ((other.from == arc.to) || (other.to == arc.to) ||
        (other.from == arc.from) || (other.to == arc.from))
      {
        arc = other;
        return true;
      }
    }
    return false;
  }

  static std::list<GraphArc> solutionToArcs(const std::vector<double>& sol,
    std::map<int, GraphArc>& xvar, std::size_t startVar)
  {
    std::list<GraphArc> res;
    for (int i = startVar; i < sol.size(); i++)
    {
      if (sol[i] > INTTOLERANCE)
        res.push_back(xvar[i]);
    }
    return res;
  }

};

inline bool operator<(const GraphArc& c1, const GraphArc& c2)
{
  if (c1.from < c2.from)
    return true;
  if (c1.from > c2.from)
    return false;
  return (c1.to < c2.to);
}
inline bool operator== (const GraphArc& c1, const GraphArc& c2)
{
  return (c1.from == c2.from) && (c1.to == c2.to);
}

class Tour
{
  std::vector<GraphArc> arcs;
public:
  void add(const GraphArc& arc)
  {
    arcs.push_back(arc);
  }
  /**
  * Get all different nodes tourched by this tour
  */
  std::set<int> getNodes() const {
    std::set<int> vertices;
    for (auto a : arcs)
    {
      vertices.insert(a.from);
      vertices.insert(a.to);
    }
    return vertices;
  }

  std::size_t numNodes() const {
    return getNodes().size();
  }

  friend std::ostream& operator<<(std::ostream& out, const Tour& t)
  {
    std::set<GraphArc> toVisit(++t.arcs.begin(), t.arcs.end());
    GraphArc current = *t.arcs.begin();
    out << current.from << "-" << current.to;
    int lastTo = current.to;
    bool found = false;
    while (toVisit.size() > 0)
    {
      found = false;
      for (auto a : toVisit)
      {
        if ((a.from == lastTo) || (a.to == lastTo))
        {
          toVisit.erase(a);
          current = a;
          if (a.from == lastTo)
          {
            out << "-" << a.to;
            lastTo = a.to;
          }
          else
          {
            out << "-" << a.from;
            lastTo = a.from;
          }
          found = true;
          break;
        }
      }
      if (!found)
      {
        current = *toVisit.begin();
        toVisit.erase(current);
        lastTo = current.to;
        out << "MALFORMED: (" << current.from << "-" << current.to << ") ";
      }
    }
    return out;
  }

  static std::vector<Tour> findSubtours(std::list<GraphArc> arcs)
  {
    std::vector<Tour> subTours;

    while (arcs.size() > 0)
    {
      Tour t;
      GraphArc start = arcs.front();
      t.add(start);
      arcs.remove(start);
      while (GraphArc::findArcFromTo(arcs, start))
      {
        t.add(start);
        arcs.remove(start);
      }
      subTours.push_back(t);
    }
    return subTours;
  }
};

std::set<int> setDiff(std::set<int> s1, std::set<int> s2)
{
  std::set<int> result;
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(result, result.end()));
  return result;
}

class MyGenericCallbackCut : public ampls::GenericCallback
{
  std::size_t minXIndex;
  std::map<int, GraphArc> xvar;
  std::map<GraphArc, int> xinverse;
  std::set<int> vertices;
  int nrun;
  
  
public:
  MyGenericCallbackCut() : nrun(0) {}

  void setMap(std::map<std::string, int> map)
  {
     minXIndex = map.size();
     for (auto it : map)
     {
        GraphArc cur = GraphArc::fromVar(it.first);
        xvar[it.second] = cur;
        xinverse[cur] = it.second;
        if (it.second < minXIndex)
          minXIndex = it.second;
        vertices.insert(cur.from);
        vertices.insert(cur.to);
     }
  }

  virtual int run()
  {
   /* if (checkCanDo(ampls::CanDo::GET_LP_SOLUTION))
    {
      int nnz = 0;
      auto s = getValueArray(ampls::Value::MIP_SOL_RELAXED);
      for (auto d : s)
        if(d != 0)
          nnz++;
     // std::cout << "Number of non zeros in node solution: " << nnz << "\n";
    }*/
    if (getAMPLSWhere() == ampls::Where::MSG)
      std::cout << getMessage() << "\n";
    // Get the generic mapping
    if (getAMPLSWhere() == ampls::Where::MIPSOL)
    {
      std::cout << "Bound=" << getValue(ampls::Value::MIP_OBJBOUND) << "\n";
      std::cout << "Obj="<< getValue(ampls::Value::OBJ) << "\n";
      nrun++;
      // Add the the cut!
      auto arcs = GraphArc::solutionToArcs(getSolutionVector(), xvar, minXIndex);
      auto sts = Tour::findSubtours(arcs);
      std::cout << "Iteration " << nrun << ": Found " << sts.size() << " subtours. \n";
      int i = 0;
      for(auto st:sts)
        std::cout << "Subtour " << i++ << ": (" << st.numNodes() << " nodes)\n";
      if (sts.size() > 1)
      {
        for (auto st : sts)
        {
          auto stn = st.getNodes();
          auto nstn = setDiff(vertices, stn);
          std::vector<GraphArc> toAdd;
          for (int inside : stn) {
            for (int outside : nstn)
            {
              if (inside < outside)
                toAdd.push_back(GraphArc(inside, outside));
              else
                toAdd.push_back(GraphArc(outside, inside));
            }
          }
          std::vector<int> varsIndexToAdd;
          for (GraphArc a : toAdd)
            varsIndexToAdd.push_back(xinverse[a]);
          std::sort(varsIndexToAdd.begin(), varsIndexToAdd.end());
          std::vector<double> coeffs(varsIndexToAdd.size());
          std::fill(coeffs.begin(), coeffs.end(), 1);
          ampls::Constraint c= addLazyIndices(varsIndexToAdd.size(),
            varsIndexToAdd.data(), coeffs.data(),
            ampls::CutDirection::GE, 2);
        }
        std::cout << "Added cuts..";
      }
      std::cout << "Continue solving.\n";
      return 0;
    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m) 
{
  // Set a (generic) callback
  MyGenericCallbackCut cb;
  cb.setMap(m.getVarMapFiltered("X"));
  m.enableLazyConstraints();
  m.setCallback(&cb);
  
  // Start the optimization process
  m.optimize();

  // Get the objective value
  double obj = m.getObj();
  printf("\nObjective with callback (%s)=%f\n", m.driver(), obj);

  // Get the solution vector
  std::map<int, GraphArc> xvar;
  std::size_t minXIndex = INT_MAX;
  for (auto it : m.getVarMapFiltered("X"))
  {
    GraphArc cur = GraphArc::fromVar(it.first);
    xvar[it.second] = cur;
    if (it.second < minXIndex)
      minXIndex = it.second;
  }
  auto a = GraphArc::solutionToArcs(m.getSolutionVector(), xvar, minXIndex);
  auto sts = Tour::findSubtours(a);

  // Print solution
  std::cout << "Solution has " << sts.size() << " subtours\n";
  int i = 0;
  for (auto st : sts)
    std::cout << "SUBTOUR " << i++ << " (" << st.numNodes() << " nodes): " << st << "\n";
  return obj;
}



#ifdef USE_amplapi

void declareModel(ampl::AMPL& a) {

  a.eval("set NODES ordered; param hpos{ NODES }; param vpos{NODES};");
  a.eval("set PAIRS := {i in NODES, j in NODES : ord(i) < ord(j)};");
  a.eval("param distance{ (i,j) in PAIRS }:= sqrt((hpos[j] - hpos[i]) **2 + (vpos[j] - vpos[i]) **2);");
  a.eval("var X{ PAIRS } binary;");
  a.eval("var Length;");
  a.eval("minimize Tour_Length : Length;");
  a.eval("subject to Visit_All{i in NODES } : sum{ (i, j) in PAIRS } X[i, j] + sum{ (j, i) in PAIRS } X[j, i] = 2;");
  a.eval("c: Length = sum{ (i,j) in PAIRS } distance[i, j] * X[i, j];");
}
#include <sstream>
#include <string>
#include <fstream>

std::string trim(const std::string& str)
{
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first)
  {
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}
#include <tuple>
std::tuple<std::vector<double>,
  std::vector<double>, std::vector<double>> readTSP(const char* path) {

  std::ifstream infile(path);
  std::string line;
  int nitems = 0;
  bool coords = false;
  int node;
  double x, y;
  std::vector<double> nodes;
  std::vector<double> xs, ys;
  while (std::getline(infile, line))
  {
    if (nitems == 0) {
      if (line.find("DIMENSION") != std::string::npos)
      {
        auto ss = trim(line.substr(line.find(":") + 1));
        std::istringstream iss(ss);
        if (!(iss >> nitems))
          break; // errror
      }
      continue;
    }
    if (!coords)
    {
      if (line.find("NODE_COORD_SECTION") != std::string::npos)
      {
        coords = true;
        continue;
      }
    }
    if (coords) {
      std::istringstream iss(trim(line));
      if (iss >> node >> x >> y) {
        nodes.push_back(node);
        xs.push_back(x);
        ys.push_back(y);
      }
    }
  }
  for (auto x : nodes)
  {
    printf("node: %d\n", x);
  }
  return std::make_tuple(nodes, xs, ys);
}


#endif
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "tspg96.nl");

#ifdef USE_amplapi
  ampl::AMPL a;
  try {
    declareModel(a);
  }
  catch (const std::exception& e) {
    std::cout << e.what();
  }
  auto t= readTSP("D:/Development/ampl/ampls-api/python/examples/tsp/gr96.tsp");
  std::vector<double> nodes;
  std::vector<double> xs, ys;
  std::tie(nodes, xs, ys) = t;
  auto df = ampl::DataFrame(1, { "NODES", "hpos", "vpos" });
  df.setColumn("NODES", nodes.data(), nodes.size());
  df.setColumn("hpos", xs.data(), nodes.size());
  df.setColumn("vpos", ys.data(), nodes.size());
  a.setData(df, "NODES");
  a.eval("display NODES, hpos, vpos;");

#ifdef USE_cbcmp
  auto cbcmodel = ampls::AMPLAPIInterface::exportModel<ampls::CbcModel>(a);
  doStuff(cbcmodel);
#endif
#ifdef USE_copt
  auto coptmodel = ampls::AMPLAPIInterface::exportModel<ampls::CoptModel>(a);
  doStuff(coptmodel);
#endif
#ifdef USE_xpress
  auto xpressmodel = ampls::AMPLAPIInterface::exportModel<ampls::XPRESSModel>(a);
  doStuff(xpressmodel);
#endif
#ifdef USE_gurobi
  auto gurobimodel = ampls::AMPLAPIInterface::exportModel<ampls::GurobiModel>(a);
  doStuff(gurobimodel);
#endif
#ifdef USE_cplexmp
  auto cplexmodel = ampls::AMPLAPIInterface::exportModel<ampls::CPLEXModel>(a);
  doStuff(cplexmodel);
#endif
#endif
}
;