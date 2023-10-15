@echo off
setlocal
if "%1"=="" (
  echo SOLVER argument not provided. Exiting...
  exit /b
)

if "%1"=="all" (
  set "SOLVERS=gurobi cplex xpress scip copt" 
) else (
  set "SOLVERS=%1"
)

for %%s in (%SOLVERS%) do (
  cd  %%s
  bash ./prepare.sh win64
  pip install -e .
  python -m amplpy_%%s.tests
  cd ..
)

endlocal
