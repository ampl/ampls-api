include("../createPythonWrapper.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}/gurobi
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplex/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
)

createPythonWrapper(ampls)
target_link_libraries(amplpy_ampls_swig
cplex-drv cplex-lib
gurobi-drv gurobi-lib)