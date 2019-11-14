#ifndef GUROBI_AMPLIMPORTS_H_INCLUDE_
#define GUROBI_AMPLIMPORTS_H_INCLUDE_
#include <stdlib.h> // for exit


#include "dlfcn.h"
#include "gurobi_c.h"

#ifdef _WIN32
#define ENTRYPOINT __declspec(dllimport)
#define DLLNAME "gurobi-lib.dll"
#else
#define ENTRYPOINT
#define DLLNAME "gurobidrv-lib.so"
#endif

struct ASL;

namespace grb
{
  namespace impl
  {
#define IMPORTFUNC(FNAME) FNAME ## Ptr = (FNAME ## PtrType)dlsym(libHandle, #FNAME); 
    /*
    extern "C" {
      // Imported from the GUROBI driver
      ENTRYPOINT GRBmodel* AMPLloadmodel(int argc, char** argv);
      ENTRYPOINT GRBmodel* AMPLloadmodelNoLic(int argc, char** argv, ASL** asl);
      ENTRYPOINT int AMPLcallmain(int argc, char** argv);
      ENTRYPOINT void AMPLwritesol(GRBmodel* m, ASL* asl, int lastoptimizerun);
      ENTRYPOINT void freeEnvironment();
      ENTRYPOINT void freeASL(ASL** aslp);
    }*/

    typedef GRBmodel* (__cdecl* AMPLloadmodelNoLicPtrType)(int argc, char** argv, ASL** asl);
    typedef void(__cdecl* AMPLwritesolPtrType)(GRBmodel* m, ASL* asl, int lastoptimizerun);
    typedef void(__cdecl* freeEnvironmentPtrType)();
    typedef void(__cdecl* freeASLPtrType)(ASL** aslp);

    AMPLloadmodelNoLicPtrType AMPLloadmodelNoLicPtr;
    AMPLwritesolPtrType AMPLwritesolPtr;
    freeEnvironmentPtrType freeEnvironmentPtr;
    freeASLPtrType freeASLPtr;
    

    void* libHandle;

    void initFunctions() {
      if (libHandle)
        return;
      
      libHandle = dlopen(DLLNAME, RTLD_LAZY);
      if (!libHandle)
      {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
      }
      IMPORTFUNC(AMPLloadmodelNoLic)
      IMPORTFUNC(AMPLwritesol)
      IMPORTFUNC(freeEnvironment)
      IMPORTFUNC(freeASL)
    }
  }
}


#endif // GUROBI_AMPLIMPORTS_H_INCLUDE_