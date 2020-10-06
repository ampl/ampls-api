include("../createPythonWrapper.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplex/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
)
createPythonWrapper(simpleapi)
target_link_libraries(amplpy_simpleapi_swig
cplex-drv cplex-lib
gurobi-drv gurobi-lib)