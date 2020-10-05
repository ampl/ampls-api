include("../createSwigProject.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplex/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
)

createSwigProject(simpleapi)
target_link_libraries(simpleapisharp_c
cplex-drv cplex-lib
gurobi-drv gurobi-lib)