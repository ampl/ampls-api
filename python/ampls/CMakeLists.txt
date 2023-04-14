include("../createPythonWrapper.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}/gurobi
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplexmp/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
)

createPythonWrapper(ampls ampls ampls ampls)
target_link_libraries(amplpy_ampls_swig
cplexmp-drv cplexmp-lib
gurobi-drv gurobi-lib
xpress-drv xpress-lib
copt-drv copt-lib
)