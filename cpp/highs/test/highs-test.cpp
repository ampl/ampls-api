#include "highs_interface.h"
#include "highs_callback.h"

#include "test-config.h" // for MODELS_DIR

#include <iostream>
#include <vector>

const char* MODELNAME = "queens18.nl";

class MyHighsCutCallback : public ampls::HighsCallback
{
  int run()
  {
    /*
    int ret = 0;
    if (getWhere() == ampls::Where::MSG)
    {
      printf("%s", getMessage());
      return 0;
    }
    if (getWhere() == HIGHS_CBCONTEXT_MIPSOL)
    {
      std::vector<std::string> vars;
      vars.push_back("x[1,2]");
      vars.push_back("x[1,4]");
      double coefs[] = { 1,1};
      std::vector<double> sol = getSolutionVector();
      for (int i = 0; i < sol.size(); i++)
        if(sol[i]!=0)
          printf("x[%d] = %f\n", i, sol[i]);
      auto c= addLazy(vars, coefs, ampls::CutDirection::GE, 2);
    }*/
    return 0;
  }
};
class MyHighsCallback : public ampls::HighsCallback
{
  int nsol = 0;
public:
  int run()
  {
    if (getWhere() == kHighsCallbackLogging || getWhere() == kHighsCallbackMipLogging)
    {
    //  std::cout << getMessage() << std::endl;
      return 0;
    }
    if (getAMPLWhere() == ampls::Where::MIPSOL)
    {
      std::cout << "Where::MIPSOL - " << getWhereString() << std::endl;
      nsol++;
      if (nsol==20) return -1;

      int indices = nsol;
      double value = 1;
      Highs_addRow(_prob, 1, 1, 1, &indices, &value);
      std::cout << model_->getNumCons() << std::endl;
    }
    return 0;
  }
  void* _prob;
  MyHighsCallback(void* prob) : _prob(prob) {}
};

int main(int argc, char** argv) {

  int res = 0;

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  ampls::HighsDrv d;
  ampls::HighsModel m = d.loadModel(buffer);
  
  auto options = m.getOptions();
  for (auto o : options)
    printf("%s\n", o.toString().c_str());

  m.setOption("mip:return_gap", 1);
  m.setOption("outlev", 1);
  printf("Option=%d\n", m.getIntOption("mip:return_gap"));
  MyHighsCallback cb(m.getHighsModel());
  
  res = m.setCallback(&cb);
 

  if (res != 0)
  {
    std::cout << "ERROR!!! " << res << std::endl;
    return res;
  }
  
  m.optimize();

  // Access objective function through generic API
  double obj = m.getObj();
  std::cout << "Objective: " << obj << std::endl;

  // Access objective through HiGHS API
  void* prob = m.getHighsModel();
  obj = Highs_getObjectiveValue(prob);
  std::cout << "Objective from HiGHS: " << obj << std::endl;

 
  // Access info through shortcuts
  int64_t nodes;

  nodes = m.getInt64Attr("mip_node_count");
  std::cout << "MIP nodes from shortcut: " << nodes << std::endl;
  // Use Highs C API to access attributes
  Highs_getInt64InfoValue(prob, "mip_node_count", &nodes);
  std::cout << "MIP nodes from C API: " << nodes << std::endl;
  

  m.writeSol();
  // Get solution vector via generic interface
  std::vector<double> vars = m.getSolutionVector();
  // Get map and display the variables ordered as AMPL stores them
  auto fg = m.getVarMap();
  double value;
  printf("\nSolution vector ordered by AMPL definition\n");
  for (auto r : fg)
  {
    value = vars[r.second];
    if (value != 0)
      std::cout << "Index: " << r.second << " AMPL: " << r.first.data() << "=" << value << std::endl;
  }
  
  // Get solution vector via Highs C API
  int nc =  Highs_getNumCols(prob);
  auto varsFromHighs = std::vector<double>(nc);
  Highs_getSolution(prob, varsFromHighs.data(), NULL, NULL, NULL);

  // Get inverse map and display the variables with solver ordering
  auto gf = m.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      std::cout << "Index: " << i << " AMPL: " << gf[i] << "=" << vars[i] << std::endl;
  }
}
