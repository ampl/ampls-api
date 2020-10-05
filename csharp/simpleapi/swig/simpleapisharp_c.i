%module(directors = "1") simpleapi_c

%include <arrays_csharp.i>
CSHARP_ARRAYS(char*, string)
CSHARP_ARRAYS(double, double)
CSHARP_ARRAYS(int, int)
CSHARP_ARRAYS(const double, double)
CSHARP_ARRAYS(const int, int)

%apply double OUTPUT[]{ double* sol }
%apply double INPUT[]{ const double* coeffs }
%apply int INPUT[]{ const int* vars }

%include "../../../cpp/generic/swig/generic-common.i"
%include "../../../cpp/cplex/swig/cplex-common.i"
%include "../../../cpp/gurobi/swig/gurobi-common.i"
