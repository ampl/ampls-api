%module(directors = "1") cpxsharp_c

%include <arrays_csharp.i>
CSHARP_ARRAYS(char*, string)
CSHARP_ARRAYS(char const*, string)
CSHARP_ARRAYS(double, double)
CSHARP_ARRAYS(int, int)
CSHARP_ARRAYS(const double, double)
CSHARP_ARRAYS(const int, int)

%apply double OUTPUT[]{ double* sol }
%apply double INPUT[]{ const double* coeffs }
%apply int INPUT[]{ const int* vars }

%apply double INPUT[]{ double const* rhs } 
%apply int INPUT[] { int const* rm }
%apply int INPUT[] { int const* rmatind }
%apply int INPUT[]{ int const* rmatbeg }
%apply double INPUT[] { double const* rmatval }
%apply char* INPUT[] { char** rowname }

%apply int INPUT[]{ int const* type }
%apply int INPUT[]{ int const* indvar }
%apply int INPUT[]{ int const* complemented }
%apply int INPUT[]{ int const* linbeg }
%apply int INPUT[]{ int const* linind }
%apply double INPUT[]{ double const* linval }
%apply char* INPUT[]{ char** indname }

%include "../../../cpp/generic/swig/generic-common.i"
%include "../../../cpp/cplex/swig/cplex-common.i"
