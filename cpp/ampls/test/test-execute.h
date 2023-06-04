#include "test-base.h"
#ifdef USE_gurobi
  ampls_test(TESTNAME, GurobiModel)
#endif

#ifdef USE_xpress
    ampls_test(TESTNAME, XPRESSModel)
#endif

#ifdef USE_cplex
    ampls_test(TESTNAME, CPLEXModel)
#endif

#ifdef USE_copt
    ampls_test(TESTNAME, CoptModel)
#endif

#ifdef USE_cbcmp
    ampls_test(TESTNAME, CbcModel)
#endif