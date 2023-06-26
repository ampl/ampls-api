#include <functional>
#include <memory> // for unique_ptr

#include "scip_interface.h"
#include "ampls/ampls.h"

/**
 * @brief translates a SCIP_RETCODE into an error string
 *
 * @param[in] retcode SCIP_RETCODE you want to translate
 * @param[in] buffersize size of buffer
 * @param[out] buffer_str buffer to character array to store translated message, this must be at least of size SCIP_MSG_MAX
 * @return buffer_str or NULL, if retcode could not be translated
 */
inline char* SCIPgetErrorString(SCIP_RETCODE retcode, char* buffer_str, int buffersize)
{
   // the following was copied from SCIPprintError
   switch(retcode)
   {
   case SCIP_OKAY:
      (void) SCIPsnprintf(buffer_str, buffersize, "normal termination");
      return buffer_str;
   case SCIP_ERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "unspecified error");
      return buffer_str;
   case SCIP_NOMEMORY:
      (void) SCIPsnprintf(buffer_str, buffersize, "insufficient memory error");
      return buffer_str;
   case SCIP_READERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "file read error");
      return buffer_str;
   case SCIP_WRITEERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "file write error");
      return buffer_str;
   case SCIP_BRANCHERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "branch error");
      return buffer_str;
   case SCIP_NOFILE:
      (void) SCIPsnprintf(buffer_str, buffersize, "file not found error");
      return buffer_str;
   case SCIP_FILECREATEERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "cannot create file");
      return buffer_str;
   case SCIP_LPERROR:
      (void) SCIPsnprintf(buffer_str, buffersize, "error in LP solver");
      return buffer_str;
   case SCIP_NOPROBLEM:
      (void) SCIPsnprintf(buffer_str, buffersize, "no problem exists");
      return buffer_str;
   case SCIP_INVALIDCALL:
      (void) SCIPsnprintf(buffer_str, buffersize, "method cannot be called at this time in solution process");
      return buffer_str;
   case SCIP_INVALIDDATA:
      (void) SCIPsnprintf(buffer_str, buffersize, "method cannot be called with this type of data");
      return buffer_str;
   case SCIP_INVALIDRESULT:
      (void) SCIPsnprintf(buffer_str, buffersize, "method returned an invalid result code");
      return buffer_str;
   case SCIP_PLUGINNOTFOUND:
      (void) SCIPsnprintf(buffer_str, buffersize, "a required plugin was not found");
      return buffer_str;
   case SCIP_PARAMETERUNKNOWN:
      (void) SCIPsnprintf(buffer_str, buffersize, "the parameter with the given name was not found");
      return buffer_str;
   case SCIP_PARAMETERWRONGTYPE:
      (void) SCIPsnprintf(buffer_str, buffersize, "the parameter is not of the expected type");
      return buffer_str;
   case SCIP_PARAMETERWRONGVAL:
      (void) SCIPsnprintf(buffer_str, buffersize, "the value is invalid for the given parameter");
      return buffer_str;
   case SCIP_KEYALREADYEXISTING:
      (void) SCIPsnprintf(buffer_str, buffersize, "the given key is already existing in table");
      return buffer_str;
   case SCIP_MAXDEPTHLEVEL:
      (void) SCIPsnprintf(buffer_str, buffersize, "maximal branching depth level exceeded");
      return buffer_str;
   case SCIP_NOTIMPLEMENTED:
      (void) SCIPsnprintf(buffer_str, buffersize, "function not implemented");
      return buffer_str;
   }
   return NULL;
}

namespace ampls
{

SCIPDrv::~SCIPDrv() {
}

SCIPModel SCIPDrv::loadModelImpl(char** args, const char** options) {
  auto mp = static_cast<impl::mp::AMPLS_MP_Solver*>(impl::scip::AMPLSOpen_scip(3, args));
  auto msg = impl::mp::AMPLSGetMessages(mp);
  if (msg[0] != nullptr)
    throw ampls::AMPLSolverException(msg[0]);
  return SCIPModel(mp, args[1], options);
}

void SCIPModel::writeSolImpl(const char* solFileName) {
  impl::mp::AMPLSReportResults(solver_, solFileName);
}


template <typename T>
struct MsgCallback;

template <typename Ret, typename... Params>
struct MsgCallback<Ret(Params...)> {
  template <typename... Args>
  static Ret callback(Args... args) {
    return func(args...);
  }
  static std::function<Ret(Params...)> func;
};

template <typename Ret, typename... Params>
std::function<Ret(Params...)> MsgCallback<Ret(Params...)>::func;


class MySCIPCallbackBridge : public SCIPCallback {
  GenericCallback* cb_;
public:
  MySCIPCallbackBridge(GenericCallback* cb) {
    cb_ = cb;
  }
  virtual int run() {
    return cb_->run();
  }
};

impl::BaseCallback* SCIPModel::createCallbackImplDerived(GenericCallback* callback) {
  return new MySCIPCallbackBridge(callback);
}

void SCIPModel::optimize() {
  SCIPsolve(model_);
}


SCIPModel::~SCIPModel() {
  if (copied_)
    return;
  impl::scip::AMPLSClose_scip(solver_);
}

std::string SCIPModel::error(int code)
{ 
  char buffer[100];

  if (SCIPgetErrorString((SCIP_RETCODE)code, buffer, 100) != NULL) {
    return std::string(buffer);
  }
  else {
    return "Error code not found.";
  }
}


std::vector<double> SCIPModel::getConstraintsValueImpl(int offset, int length) {
  std::vector<double> pi(length);
  if (SCIPisDualSolAvailable(getSCIPmodel(), TRUE)) {
    for (int i=offset; i < offset+length; i++)
      SCIPgetDualSolVal(getSCIPmodel(), SCIPgetProbData(getSCIPmodel())->linconss[i], pi.data() + i, NULL);
  }
  return pi;
}
std::vector<double> SCIPModel::getVarsValueImpl(int offset, int length) {
  std::vector<double> vars(length);
  for (int i = offset; i < offset+length; i++)
    vars[i] = SCIPgetSolVal(getSCIPmodel(), SCIPgetBestSol(getSCIPmodel()), SCIPgetProbData(getSCIPmodel())->vars[i]);
  return vars;
}


} // namespace