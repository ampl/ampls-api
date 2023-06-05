#include "copt_interface.h"
#include "copt_callback.h"

#include "test-config.h" // for MODELS_DIR
#include <cstring>
#include <vector>

#include <cmath>

const char* MODELNAME = "queens18.nl";

class MyCoptCutCallback : public ampls::CoptCallback
{
  int run()
  {
    int ret = 0;
    if (getWhere() == ampls::Where::MSG)
    {
      printf("%s", getMessage());
      return 0;
    }
    if (getWhere() == COPT_CBCONTEXT_MIPSOL)
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
    }
    return 0;
  }
};
class MyCoptCallback : public ampls::CoptCallback
{
  int lastIter = 0;
public:
  int run()
  {
    if (getWhere() == ampls::Where::MSG)
    {
      std::string s = getMessage();
      printf("%s", s.data());
      return 0;
    }
    printf("\n** Called callback with where=%s ** \n", getWhereString());
    
    if (getWhere() == COPT_CBCONTEXT_MIPRELAX)
      printf("COPT_CBINFO_RELAXSOLOBJ %f\n", get(COPT_CBINFO_RELAXSOLOBJ).dbl);
    else if (getWhere() == COPT_CBCONTEXT_MIPSOL) {

      double objBest = getDouble(COPT_CBINFO_BESTOBJ);
      double objBnd = getDouble(COPT_CBINFO_BESTBND);
      printf("COPT_CBINFO_BESTOBJ %f\n", objBest);
      printf("COPT_CBINFO_BESTBND %f\n", objBnd);

      
      printf("REL GAP=%f%%\n", 100*getValue(ampls::Value::MIP_RELATIVEGAP).dbl);
      if (fabs(objBest - objBnd) < 0.1 * (1.0 + fabs(objBest))) {
        printf("Stop early - 10%% gap achieved\n");
        return -1;
      }
    }
    printf("** End of callback handler **\n\n");
    return 0;
  }
  copt_prob* _prob;
  MyCoptCallback(copt_prob* prob) : _prob(prob) {}
};

int main(int argc, char** argv) {

  int res = 0;

  char buffer[80];
  strcpy(buffer, MODELS_DIR);
  strcat(buffer, MODELNAME);

  ampls::CoptDrv d;
  ampls::CoptModel m = d.loadModel(buffer);
  
  auto options = m.getOptions();
  for (auto o : options)
    printf("%s\n", o.toString().c_str());

  m.setOption("mip:return_gap", 1);
  printf("Option=%d\n", m.getIntOption("mip:return_gap"));
  m.enableLazyConstraints();
  MyCoptCallback cb(m.getCOPTmodel());
  res = m.setCallback(&cb);
  if (res != 0)
  {
    printf("ERROR!!! %i\n", res);
    return res;
  }
  
  m.optimize();

  // Access objective function through generic API
  double obj = m.getObj();
  printf("Objective: %f\n", obj);

  copt_prob* prob = m.getCOPTmodel();
  int isMIP;
  COPT_GetIntAttr(prob, COPT_INTATTR_ISMIP, &isMIP);
  const char* objAttrName = isMIP ? COPT_DBLATTR_BESTOBJ : COPT_DBLATTR_LPOBJVAL;

  // Access copt attribute with shortcut function getDoubleAttr
  obj = m.getDoubleAttr(objAttrName);
  printf("Objective from shortcut: %f\n", obj);

  // Use Copt C API to access attributes
  COPT_GetDblAttr(prob, objAttrName, &obj);
  printf("Objective from copt: %f\n", obj);
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
      printf("Index: %i AMPL: %s=%f\n", r.second, r.first.data(), value);
  }
  
  // Get solution vector via Copt C API
  int nc;
  COPT_GetIntAttr(prob, COPT_INTATTR_COLS, &nc);
  double* varsFromCopt = new double[nc];
  COPT_GetSolution(prob, varsFromCopt);

  // Get inverse map and display the variables with solver ordering
  auto gf = m.getVarMapInverse();
  printf("\nSolution vector ordered by solver\n");
  for (int i = 0; i < nc; i++)
  {
    if (vars[i] != 0)
      printf("Index: %i AMPL: %s=%f\n", i, gf[i].c_str(), vars[i]);
  }
  delete[] varsFromCopt;
}
