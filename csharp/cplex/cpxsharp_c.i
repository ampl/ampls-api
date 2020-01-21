%module(directors = "1") cpxsharp_c

%include <arrays_csharp.i>
CSHARP_ARRAYS(char*, string)
CSHARP_ARRAYS(double, double)
CSHARP_ARRAYS(int, int)

%apply double OUTPUT[]{ double* sol }
%apply double INPUT[]{ const double* coeffs }
%apply int INPUT[]{ const int* vars }

%include "../common/common.i"
