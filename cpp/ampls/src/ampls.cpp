#include "ampls/ampls.h"

#include <cstring>

#include <fstream>      // std::filebuf
#include <iostream>     // std::ios, std::istream
#include <stdexcept>    // std::runtime_error
#include "csvReader.h"

namespace ampls
{
char** generateArguments(const char* modelName)
{
  // Add exe name, -AMPL and \0
  char** params = new char* [4];
  const char* MYNAME = "my";
  const char* OPTION = "-AMPL";

  params[0] = new char[strlen(MYNAME) + 1];
  params[1] = new char[strlen(modelName) + 1];
  params[2] = new char[strlen(OPTION) + 1];
  strcpy(params[0], MYNAME);
  strcpy(params[2], OPTION);
  strcpy(params[1], modelName);
  params[3] = NULL;
  return params;
}

void deleteParams(char** params)
{
  for (int i = 0; i < 3; i++)
    delete[]params[i];
  delete[] params;
}



std::string getColFileName(const std::string& nlFileName) {
  size_t lastindex = nlFileName.find_last_of(".");
  std::string basename;
  if (lastindex != std::string::npos)
    basename = nlFileName.substr(0, lastindex);
  else
    basename = nlFileName;
  basename += ".col";
  return basename;
}
std::filebuf openColFile(const std::string &nlFileName) {
  std::string name = getColFileName(nlFileName);
  std::filebuf fb;
  if (fb.open(name.c_str(), std::ios::in))
    return fb;
  else
  {
    fb.close();
    throw ampls::AMPLSolverException("Make sure you export the column file from AMPL.");
  }

}
std::map<int, std::string> AMPLModel::getVarMapInverse() {
  std::filebuf fb = openColFile(fileName_);
  std::istream is(&fb);
  std::map<int, std::string> map = impl::createMapInverse(is);
  fb.close();
  return map;
}
std::map<std::string, int> AMPLModel::getVarMapFiltered(const char* beginWith) {
  std::filebuf fb = openColFile(fileName_);
  std::istream is(&fb);
  std::map<std::string, int> map = impl::createMap(is, beginWith);
  fb.close();
  return map;
}


std::map<std::string, int>& impl::BaseCallback::getVarMap() {
  
  model_->getVarMapsInternal();
  return model_->varMap_;
}

std::map<int, std::string>& impl::BaseCallback::getVarMapInverse() {

  model_->getVarMapsInternal();
  return model_->varMapInverse_;
}


int impl::BaseCallback::callAddCut(std::vector<std::string>& vars,
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
  return doAddCut((int)length, &indices[0], coeffs, direction, rhs, lazy);
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

std::vector<double> impl::BaseCallback::getSolutionVector() {

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



} // namespace