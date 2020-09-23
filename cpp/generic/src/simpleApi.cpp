#include "simpleapi/simpleApi.h"

#include <cstring>

#include <fstream>      // std::filebuf
#include <iostream>     // std::ios, std::istream
#include <stdexcept>    // std::runtime_error
#include "csvReader.h"

namespace ampl
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



std::string getColFileName(const char* nlFileName) {
  std::string name(nlFileName);
  size_t lastindex = name.find_last_of(".");
  std::string basename;
  if (lastindex != std::string::npos)
    name = name.substr(0, lastindex);
  name += ".col";
  return name;
}
std::map<int, std::string> AMPLModel::getVarMapInverse() {
  std::string name = getColFileName(fileName_.c_str());
  std::filebuf fb;
  if (fb.open(name.c_str(), std::ios::in))
  {
    std::istream is(&fb);
    std::map<int, std::string> map = impl::createMapInverse(is);
    fb.close();
    return map;
  }
  else
  {
    fb.close();
    throw ampl::AMPLSolverException("Make sure you export the column file from AMPL.");
  }
}
std::map<std::string, int> AMPLModel::getVarMapFiltered(const char* beginWith) {
  std::string name = getColFileName(fileName_.c_str());
  std::filebuf fb;
  if (fb.open(name.c_str(), std::ios::in))
  {
    std::istream is(&fb);
    std::map<std::string, int> map = impl::createMap(is, beginWith);
    fb.close();
    return map;
  }
  else
  {
    fb.close();
    throw ampl::AMPLSolverException("Make sure you export the column file from AMPL.");
  }
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
  const double* coeffs, char direction, double rhs, int lazy) {
  std::size_t length = vars.size();
  std::map<std::string, int> map = getVarMap();
  std::vector<int> indices;
  indices.reserve(length);
  for (size_t i = 0; i < vars.size(); i++)
    indices.push_back(map[vars[i]]);
  return doAddCut((int)length, &indices[0], coeffs, direction, rhs, lazy);
}

double* AMPLModel::getSolutionVector(int* len) {
  *len = getNumVars();
  double* result = new double[*len];
  getSolution(0, *len, result);
  return result;
}



double* impl::BaseCallback::getSolutionVector(int* len) {

  *len = model_->getNumVars();
  double* result = new double[*len];
  int res;
  try {
    res = getSolution(*len, result);
  }
  catch (...)
  {
    delete[] result;
    return NULL;
  }
  return result;
}

} // namespace