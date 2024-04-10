#include "highs_callback.h"
#include "highs_interface.h"

namespace ampls
{
const char* HighsCallback::getWhereString()
{
    const char* names[] = {
      "kHighsCallbackLogging",
      "kHighsCallbackSimplexInterrupt",
      "kHighsCallbackIpmInterrupt",
      "kHighsCallbackMipSolution",
      "kHighsCallbackMipImprovingSolution",
      "kHighsCallbackMipLogging",
      "kHighsCallbackMipInterrupt"
    };
    if (where_ < kHighsCallbackLogging ||
      where_> kHighsCallbackMipInterrupt)
      return "Where code not found";
    return names[where_];
}

void* HighsCallback::getHighsModel() {
    return ((HighsModel*)model_)->getHighsModel();
 };

const char* HighsCallback::getMessage()
{
  char* msg;
  if (where_ == kHighsCallbackLogging || where_ == kHighsCallbackMipLogging)
    return msg_;
  else
    throw ampls::AMPLSolverException("Cannot get message outside of a log callback.");
}

int HighsCallback::getSolution(int len, double* sol)
{
  for (std::size_t i = 0; i < len; ++i)
    sol[i] = cbdata_->mip_solution[i];
  return 0;
}

double HighsCallback::getObj()
{
  return cbdata_->objective_function_value;
}


Variant  HighsCallback::getValueImpl(Value::CBValue v) {
  switch (v)
  {
  case Value::OBJ:
    return Variant(getObj());
  case Value::MIP_RELATIVEGAP:
    return cbdata_->mip_gap;
  case Value::MIP_OBJBOUND:
    return cbdata_->mip_dual_bound;
  case Value::N_COLS:
    return model_->getNumVars();
  case Value::N_ROWS:
    return model_->getNumCons();
  default:
    throw AMPLSolverException("Specified value unknown.");
  }
}

std::vector<double> HighsCallback::getValueArray(Value::CBValue v) {

  switch (v)
  {
  case Value::MIP_SOL_RELAXED:
    std::vector<double> d(model_->getNumVars());
    for (int i = 0; i < d.size(); i++)
      d[i] = cbdata_->mip_solution[i];
    return d;
  }
  return std::vector<double>();
}

} // namespace