#include "ampls/ampls.h"
#include "csvReader.h"

#include <fstream>      // std::filebuf
#include <iostream>     // std::ios, std::istream
#include <sstream> // string concatenation
#include <cstring> // for strcpy and strlen

namespace ampls
{
char** generateArguments(const char* modelName, std::vector<std::string> options)
{
  
  // Add exe name, -AMPL and \0
  char** params = new char* [options.size()+4];
  const char* MYNAME = "my";
  const char* OPTION = "-AMPL";

  params[0] = new char[strlen(MYNAME) + 1];
  strcpy(params[0], MYNAME);
  params[1] = new char[strlen(modelName) + 1];
  strcpy(params[1], modelName);
  params[2] = new char[strlen(OPTION) + 1];
  strcpy(params[2], OPTION);
  for (size_t i = 0; i < options.size(); i++)
  {
    params[i + 3] = new char[strlen(options[i].data()) + 1];
    strcpy(params[i+3], options[i].data());
  }
  params[3+ options.size()] = NULL;
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
std::map<std::string, int> AMPLModel::getConsMapFiltered(const char* beginWith) {
  return getMapFiltered(fileName_, "row", beginWith);
}
std::map<int, std::string> AMPLModel::getConsMapInverse() {
  return getMapInverse(fileName_, "row");
}


std::vector<double> AMPLModel::getSolutionVector() {
  int len = getNumVars();
  std::vector<double> res;
  res.resize(len);
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



namespace impl {

  std::vector<double> BaseCallback::getSolutionVector() {

    int len = model_->getNumVars();
    std::vector<double> res;
    res.resize(len);
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


  std::string Constraint::toAMPLString(const std::map<int, std::string>& varMap,
    const Records& records) {
    std::stringstream ss;
    ss << string_format("%s: to_come+", name().c_str());
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
        ss << string_format("+%f*%s", coeffs()[i], varMap.at(indices()[i]).c_str());
      else
        ss << string_format("%f*%s", coeffs()[i], varMap.at(indices()[i]).c_str());
    }
    ss << string_format("%s %f;", CutDirection::toString(sense()).c_str(), rhs());
    return ss.str();
  }

  std::string Variable::toAMPLString(const std::map<int, std::string>& consMap, 
    const Records& records) {
    std::stringstream ss;
    ss << string_format("var %s ", name().c_str());

    if (type_ == VarType::Binary)
      ss << "binary";
    if (type_ == VarType::Integer)
      ss << "integer";

    if (!std::isinf(lb_))
      ss << string_format(">=%f, ", lb_);
    if (!std::isinf(ub_))
      ss << string_format("<=%f,", ub_);

    if (!std::isnan(obj_))
      ss << string_format("obj %s %f,", consMap.at(consMap.size() - 1).c_str(), obj_);

    for (int i = 0; i < indices().size(); i++)
    {
      if (coeffs()[i] == 0)
        continue;
      int index = indices()[i];
      std::string name;
      if (index >= (consMap.size()-1))
        name = records.cons_[index - consMap.size()+1].name();
      else
        name = consMap.at(index);

      ss << string_format("coeff %s %f,", name.c_str(), coeffs()[i]);
    }
    ss << ";";
    return ss.str();
  }


  std::map<std::string, int>& BaseCallback::getVarMap() {
    model_->getVarMapsInternal();
    return model_->varMap_;
  }

  std::map<int, std::string>& BaseCallback::getVarMapInverse() {
    model_->getVarMapsInternal();
    return model_->varMapInverse_;
  }

  int BaseCallback::callAddCut(std::vector<std::string>& vars,
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
      printCut((int)length, &indices[0], coeffs, direction, rhs,
        cutDebugIntCoefficients_, cutDebugPrintVarNames_);
    return doAddCut((int)length, &indices[0], coeffs, direction, rhs, lazy);
  }

  void BaseCallback::recordConstraint(const char* name, const std::vector<int>& vars,
    const std::vector<double>& coefficients, CutDirection::Direction sense, double rhs) {
    auto c = Constraint(name, vars, coefficients, sense, rhs);
    c.solverIndex(model_->getNumCons() + model_->records_.getNumConstraints());
    model_->records_.addConstraint(c);
  }


}


} // namespace