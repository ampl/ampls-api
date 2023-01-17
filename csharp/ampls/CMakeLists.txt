include("../createCsharpProject.cmake")
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}
${cplex_INCLUDE_DIR}
${gurobi_INCLUDE_DIR}/gurobi
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/cplex/include
${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/gurobi/include
)

createCsharpProject(ampls ampls ampls)
target_link_libraries(amplssharp_c
#cplex-drv cplex-lib
gurobi-drv gurobi-lib)