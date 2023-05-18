%module(directors = "1") ampls_c

%include <arrays_csharp.i>
CSHARP_ARRAYS(char*, string)
CSHARP_ARRAYS(double, double)
CSHARP_ARRAYS(int, int)
CSHARP_ARRAYS(const double, double)
CSHARP_ARRAYS(const int, int)

%apply double OUTPUT[]{ double* sol }
%apply double INPUT[]{ const double* coeffs }
%apply int INPUT[]{ const int* vars }

%include "../../../cpp/ampls/swig/ampls-common.i"
%include "../../../cpp/cplex/swig/cplex-common.i"
%include "../../../cpp/gurobi/swig/gurobi-common.i"
//%include "../../../cpp/xpress/swig/xpress-common.i"

