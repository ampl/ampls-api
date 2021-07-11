#ifdef USE_cplex
#include "cplex_interface.h"
#endif
#ifdef USE_gurobi
#include "gurobi_interface.h"
#endif
#ifdef USE_xpress
#include "xpress_interface.h"
#endif
#include <list>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <regex>
#include <iostream>
#include "ampls/ampls.h"
#include "test-config.h" // for MODELS_DIR

#include <exception>

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

struct Arc {
  int from;
  int to;
  Arc() : from(0), to(0) {}
  Arc(int from, int to)
  {
    this->from = from;
    this->to = to;
  }
  

  static Arc fromVar(const std::string& var)
  {
    auto s = split(var, "[A-Za-z0-9]*\\['([A-Za-z0-9]*)','([A-Za-z0-9]*)'\\]");
    return Arc(std::stoi(s[1]), std::stoi(s[2]));
  }

  friend std::ostream& operator<<(std::ostream& out, const Arc& c)
  {
    out << '(' << c.from << "->" << c.to << ')';
    return out;
  }

  /**
  * Find arc connected to the passed one.
  * If it finds one, it changes the arc parameter to the next arc
  * in the sequence and returns true. Returns false otherwise.
  */
  static bool findArcFromTo(const std::list<Arc>& arcs, Arc& arc)
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
};

inline bool operator<(const Arc& c1, const Arc& c2)
{
  if (c1.from < c2.from)
    return true;
  if (c1.from > c2.from)
    return false;
  return (c1.to < c2.to);
}
inline bool operator== (const Arc& c1, const Arc& c2)
{
  return (c1.from == c2.from) && (c1.to == c2.to);
}

class Tour
{
  std::vector<Arc> arcs;
public:
  void add(const Arc& arc)
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
    int lastTo = t.arcs[0].to;
    for(int i=1; i< t.arcs.size(); i++)
    {
      if (t.arcs[i].from == lastTo)
      {
        if(i==1)
          out << t.arcs[0].from << "-" << t.arcs[0].to;
        lastTo = t.arcs[i].to;
        out << "-" << t.arcs[i].from;
      }
      else
      {
        if (i == 1)
          out << t.arcs[0].to << "-" << t.arcs[0].from;
        lastTo = t.arcs[i].from;
        out << "-" << t.arcs[i].to;
      }
    }
    Arc lastArc = t.arcs[t.arcs.size() - 1];
    out << "-" << (lastTo == lastArc.from ? lastArc.from : lastArc.to);
    return out;
  }

  static std::vector<Tour> findSubtours(std::list<Arc> arcs)
  {
    std::vector<Tour> subTours;

    while (arcs.size() > 0)
    {
      Tour t;
      Arc start = arcs.front();
      t.add(start);
      arcs.remove(start);
      while (Arc::findArcFromTo(arcs, start))
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
  std::map<std::string, int> _map;
  std::map<int, std::string> _inversemap;
  std::map<int, Arc> xvar;
  std::map<Arc, int> xinverse;
  std::set<int> vertices;
  int nrun;
  
  
public:
  MyGenericCallbackCut() : nrun(0) {}

  void setMap(std::map<std::string, int> map, std::map<int, std::string> inversemap)
  {
    _map = map;
    _inversemap = inversemap;
     for (auto it : _map)
     {
        Arc cur = Arc::fromVar(it.first);
        xvar[it.second] = cur;
        xinverse[cur] = it.second;
        vertices.insert(cur.from);
        vertices.insert(cur.to);
     }
  }

  std::list<Arc> solutionToArcs(const std::vector<double>& sol)
  {
    std::list<Arc> res;
    for (int i = 0; i < sol.size(); i++)
    {
      if (sol[i] != 0)
        res.push_back(xvar[i]);
    }
    return res;
  }
  virtual int run()
  {
    if (getAMPLWhere() == ampls::Where::MSG)
      std::cout << getMessage();
    // Get the generic mapping
    if (getAMPLWhere() == ampls::Where::MIPSOL)
    {
      nrun++;
      // Add the the cut!
      auto arcs = solutionToArcs(getSolutionVector());
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
          std::vector<Arc> toAdd;
          for (int inside : stn) {
            for (int outside : nstn)
            {
              if (inside < outside)
                toAdd.push_back(Arc(inside, outside));
              else
                toAdd.push_back(Arc(outside, inside));
            }
          }
          std::vector<int> varsIndexToAdd;
          std::vector<std::string> varsToAdd;
          for (Arc a : toAdd)
            varsIndexToAdd.push_back(xinverse[a]);
          std::vector<double> coeffs(varsIndexToAdd.size());
          std::fill(coeffs.begin(), coeffs.end(), 1);
          int status = addLazyIndices(varsIndexToAdd.size(),
            varsIndexToAdd.data(), coeffs.data(),
            ampls::CutDirection::GE, 2);
          if (status != 0)
            printf("status != 0: %d\n", status);
          //if (sts.size() == 2)
          //  break;
        }
        std::cout << "Added cuts. ";
      }
      std::cout << "Continue solving.\n";
      return 0;
    }
    return 0;
  }

};

double doStuff(ampls::AMPLModel& m, const char *name) 
{
  // Set a (generic) callback
  MyGenericCallbackCut cb;
  cb.setMap(m.getVarMap(), m.getVarMapInverse());
//  cb.setDebugCuts(true, true, true);
  m.setCallback(&cb);
  
  // Start the optimization process
  m.optimize();

  // Get the objective value
  double obj = m.getObj();
  printf("\nObjective with callback (%s)=%f\n", name, obj);

  // Get the solution vector
  auto a = cb.solutionToArcs(m.getSolutionVector());
  auto sts = Tour::findSubtours(a);

  // Print solution
  std::cout << "Solution has " << sts.size() << " subtours\n";
  int i = 0;
  for (auto st : sts)
    std::cout << "SUBTOUR " << i++ << " (" << st.numNodes() << " nodes): " << st << "\n";
  std::stringstream ss;
  ss << "d:/tspcpp-" << name << ".sol";
  m.writeSol(ss.str().c_str());
  return obj;
}
int main(int argc, char** argv) {

  char buffer[255];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, "tspg96.nl");
#ifdef USE_cplex
  // Load a model using CPLEX driver
  ampls::CPLEXDrv cplex;
  cplex.setOptions({ "mipgap=1e-9" });
  ampls::CPLEXModel c = cplex.loadModel(buffer);
  // Use it as generic model
  doStuff(c, "cplex");
#endif/*
#ifdef USE_xpress
  // Load a model using CPLEX driver
  ampls::XPRESSDrv xpress;
  ampls::XPRESSModel x = xpress.loadModel(buffer);
  // Use it as generic model
  doStuff(x, "xpress");
#endif

*/
#ifdef USE_gurobi
  // Load a model using gurobi driver
  ampls::GurobiDrv gurobi;
  gurobi.setOptions({ "mipgap=1e-9" });
  ampls::GurobiModel g = gurobi.loadModel(buffer);
  g.enableLazyConstraints();
  // Use it as generic model
  doStuff(g, "gurobi");
#endif
  

  

 
}
;