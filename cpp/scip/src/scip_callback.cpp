#include "scip_callback.h"
#include "scip_interface.h"

namespace ampls
{

SCIP* SCIPPlugin::getSCIP() {
    return scip_;
  };

} // namespace

