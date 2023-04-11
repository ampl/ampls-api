%module(directors = "1") gdsharp_c

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
%include "../../../cpp/x-gurobi/swig/x-gurobi-common.i"
