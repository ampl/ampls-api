include("../createCsharpProject.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${xpress_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}/gurobi
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplexmp/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cbcmp/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/xpress/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/copt/include
)

createCsharpProject(ampls ampls ampls)
target_link_libraries(amplssharp_c
cplexmp-drv cplexmp-lib
gurobi-drv gurobi-lib
xpress-drv xpress-lib
copt-drv   copt-lib
)