#include "ampls/ampls.h"
#include "csvReader.h"

#include <fstream>      // std::filebuf
#include <iostream>     // std::ios, std::istream
#include <sstream> // string concatenation
#include <cstring> // for strcpy and strlen

#include <cmath>  // for abs
#include <algorithm> // for min_element, max_element

namespace ampls
{
char** generateArguments(const char* modelName)
{

  // Add exe name, -AMPL and \0
  char** params = new char* [4];
  const char* MYNAME = "my";
  const char* OPTION = "-AMPL";

  params[0] = new char[strlen(MYNAME) + 1];
  strcpy(params[0], MYNAME);
  params[1] = new char[strlen(modelName) + 1];
  strcpy(params[1], modelName);
  params[2] = new char[strlen(OPTION) + 1];
  strcpy(params[2], OPTION);
  params[3] = NULL;
  return params;
}

void deleteParams(char** params)
{
  for (int i = 0; i < 3; i++)
    delete[]params[i];
  delete[] params;
}

std::string getSuffixedFileName(const std::string& nlFileName, const char* ext) {
  size_t lastindex = nlFileName.find_last_of(".");
  std::string basename;
  if (lastindex != std::string::npos)
    basename = nlFileName.substr(0, lastindex);
  else
    basename = nlFileName;
  return impl::string_format("%s.%s", basename.c_str(), ext);
}

std::filebuf openAuxFile(const std::string &nlFileName, const char* ext) {
  std::string name = getSuffixedFileName(nlFileName, ext);
  std::filebuf fb;
  if (fb.open(name.c_str(), std::ios::in))
    return fb;
  else
  {
    fb.close();
    throw ampls::AMPLSolverException("Make sure you export the column file from AMPL.");
  }
}

std::map<int, std::string> getMapInverse(const std::string &filename, const char* suffix) {
  std::filebuf fb = openAuxFile(filename, suffix);
  std::istream is(&fb);
  std::map<int, std::string> map = impl::createMapInverse(is);
  fb.close();
  return map;
}

std::map<std::string, int> getMapFiltered(const std::string& filename, const char* suffix,
  const char* beginWith) {
  std::filebuf fb = openAuxFile(filename, suffix);
  std::istream is(&fb);
  std::map<std::string, int> map = impl::createMap(is, beginWith);
  fb.close();
  return map;
}

std::map<std::string, int> AMPLModel::getVarMapFiltered(const char* beginWith) {
  return getMapFiltered(fileName_, "col", beginWith);
}
std::map<int, std::string> AMPLModel::getVarMapInverse() {
  return getMapInverse(fileName_, "col");
}
std::map<std::string, int> AMPLModel::getConMapFiltered(const char* beginWith) {
  return getMapFiltered(fileName_, "row", beginWith);
}
std::map<int, std::string> AMPLModel::getConMapInverse() {
  return getMapInverse(fileName_, "row");
}

std::vector<double> AMPLModel::getSolutionVector() {
  int len = getNumVars();
  std::vector<double> res(len);
  getSolution(0, len, res.data());
  return res;
}

void AMPLModel::printModelVars(bool onlyNonZero) {
  std::vector<double> sol = getSolutionVector();
  std::map<std::string, int> map = getVarMap();
  for (auto a : map)
    if ( (!onlyNonZero) || (sol[a.second] != 0))
      printf("(%i) %s=%f\n", a.second, a.first.data(), sol[a.second]);
}

std::string impl::Records::getRecordedEntities(bool exportToAMPL) {
  std::stringstream ss;
  auto vmap = parent_->getVarMapInverse();
  auto cmap = parent_->getConMapInverse();
  int begin, end;
  if (vars_.size() > 0)
  {
    getVarIndices(begin, end);
    auto values = parent_->getVarsValueImpl(begin, end - begin+1);
    for (int i = 0; i < values.size(); i++)
      vars_[i].value(values[i]);
  }

  
  if (exportToAMPL) {
    for (auto e : entities_)
    {
      if (!e->exportedToAMPL_)
      {
        ss << e->toAMPLString(vmap, cmap, *this) << std::endl;
        e->exportedToAMPL_ = true;
      }
    }
  }
  else {
    for (auto e : entities_)
     ss << e->toAMPLString(vmap, cmap, *this) << std::endl;
  }
  return ss.str();
}

std::string  Constraint::toString() {
  std::stringstream ss;
  for (int i = 0; i < indices().size(); i++)
  {
    if (coeffs()[i] == 0)
      continue;

    int index = indices()[i];

    std::string name = impl::string_format("X[%d]", index);

    if (coeffs()[i] > 0)
      ss << impl::string_format("+%f*%s", coeffs()[i], name.c_str());
    else
      ss << impl::string_format("%f*%s", coeffs()[i], name.c_str());
  }
  ss << impl::string_format("%s %f;", CutDirection::toString(sense()).c_str(), rhs());
  return ss.str();

}

std::string Constraint::toAMPLString(const std::map<int, std::string>& varMap,
  const std::map<int, std::string>& consMap,
  const impl::Records& records) const {
  std::stringstream ss;
  ss << impl::string_format("%s: to_come+", name().c_str());
  for (int i = 0; i < indices().size(); i++)
  {
    if (coeffs()[i] == 0)
      continue;

    int index = indices()[i];

    std::string name;
    if (index >= varMap.size())
      name = records.vars_[index - varMap.size()].name();
    else
      name = varMap.at(index);

    if (coeffs()[i] > 0)
      ss << impl::string_format("+%f*%s", coeffs()[i], varMap.at(indices()[i]).c_str());
    else
      ss << impl::string_format("%f*%s", coeffs()[i], varMap.at(indices()[i]).c_str());
  }
  ss << impl::string_format("%s %f;", CutDirection::toString(sense()).c_str(), rhs());
  return ss.str();
}

std::string Variable::toAMPLString(const std::map<int, std::string>& varMap, 
  const std::map<int, std::string>& consMap,
  const impl::Records& records) const  {
  std::stringstream ss;
  ss << impl::string_format("var %s ", name().c_str());

  if (type_ == VarType::Binary)
    ss << "binary";
  if (type_ == VarType::Integer)
    ss << "integer";

  if (!std::isinf(lb_))
    ss << impl::string_format(">=%f, ", lb_);
  if (!std::isinf(ub_))
    ss << impl::string_format("<=%f,", ub_);

  if (!std::isnan(obj_))
    ss << impl::string_format("obj %s %f,", consMap.at(static_cast<int>(consMap.size()) - 1).c_str(), obj_);

  for (int i = 0; i < indices().size(); i++)
  {
    if (coeffs()[i] == 0)
      continue;
    int index = indices()[i];
    std::string name;
    if (index >= (consMap.size() - 1))
      name = records.cons_[index - consMap.size() + 1].name();
    else
      name = consMap.at(index);

    ss << impl::string_format("coeff %s %f,", name.c_str(), coeffs()[i]);
  }
  ss << ";";
  if (!std::isnan(value()))
    ss << impl::string_format("let %s:=%f;", name().c_str(), value());
  return ss.str();
}

void handleStatus(const char* name, int status) {
  if (!status)
    return;
  if (status==1)
    throw ampls::AMPLSolverException::format("Unknown option: %s", name);
  else
    throw ampls::AMPLSolverException::format("Problems setting option: %s", name);
}
void AMPLMPModel::setOption(const char* name, int value) {
  handleStatus(name, impl::mp::AMPLSSetIntOption(solver_, name, value));
}
void AMPLMPModel::setOption(const char* name, double value) {
  handleStatus(name, impl::mp::AMPLSSetDblOption(solver_, name, value));
}
void AMPLMPModel::setOption(const char* name, const char* value) {
  handleStatus(name, impl::mp::AMPLSSetStrOption(solver_, name, value));
}

namespace impl {


  double calculateRelMIPGAP(double obj, double bbound)
  {
    double absmipgap = std::abs(obj - bbound);
    obj = std::abs(obj);
    return absmipgap / (1e-10 + obj);
  }


  std::vector<double> BaseCallback::getSolutionVector() {
    int len = model_->getNumVars();
    std::vector<double> res(len);
    int s;
    try {
      s = getSolution(len, res.data());
    }
    catch (...)
    {
      return res;
    }
    return res;
  }

  std::map<std::string, int>& BaseCallback::getVarMap() {
    model_->getVarMapsInternal();
    return model_->varMap_;
  }

  std::map<int, std::string>& BaseCallback::getVarMapInverse() {
    model_->getVarMapsInternal();
    return model_->varMapInverse_;
  }


  ampls::Constraint BaseCallback::callDoAddCut(int length, const int* indices,
    const double* coeffs, CutDirection::Direction direction, double rhs,
    int type) {
    ampls::Constraint c(model_, NULL, length, indices, coeffs, direction, rhs);
    int status = doAddCut(c, type);
    if (status)
      throw ampls::AMPLSolverException::format("Error while adding cut!");
    c.solverIndex(model_->getNumCons() - 1);
    return c;
  }

  ampls::Constraint BaseCallback::callAddCut(std::vector<std::string>& vars,
    const double* coeffs, CutDirection::Direction direction, double rhs, int lazy) {
    std::size_t length = vars.size();
    std::map<std::string, int> map = getVarMap();
    std::vector<int> indices;
    indices.reserve(length);
    for (size_t i = 0; i < vars.size(); i++)
    {
      std::map<std::string, int>::iterator it = map.find(vars[i]);
      if (it == map.end())
        throw AMPLSolverException::format("Variable %s not found in variable map", vars[i].c_str());
      else
        indices.push_back(map[vars[i]]);
    }
    if (cutDebug_)
      printCut(ampls::Constraint(model_, NULL, (int)length, &indices[0], coeffs, direction, rhs),
        cutDebugIntCoefficients_, cutDebugPrintVarNames_);
   
    return callDoAddCut((int)length, indices.data(), coeffs, direction, rhs, lazy);
  }


  void BaseCallback::record(const ampls::Variable& v) {
    model_->record(v);
  }
  void BaseCallback::record(const ampls::Constraint& c) {
    model_->record(c);
  }
  ampls::Variable BaseCallback::addVariable(int nnz, const int* cons,
    const double* coefficients, double lb, double ub, double objCoefficient,
    VarType::Type type, const char* name) {
    return model_->addVariable(nnz, cons, coefficients, lb, ub, objCoefficient, type, name);
  }

} // namespace impl

void Constraint::assignName() {
  name_ = impl::string_format("ampls_con_%d", parent_->numConsAdded());
}

void Variable::assignName() {
  name_ = impl::string_format("ampls_var_%d", parent_->numVarsAdded());
}

} // namespace ampls