include("../createCsharpProject.cmake")
message("DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE}")
get_target_property(cbcINCLUDES cbcmp-lib INCLUDE_DIRECTORIES)
set(DIR_CPP_INCLUDE "${DIR_CPP_INCLUDE} ${cbcINCLUDES}")
get_target_property(cbcINCLUDES cbcmp-lib INCLUDE_DIRECTORIES)
createCsharpProject(cbcmp cbcmp cbcmp)
target_link_libraries(cbcmpsharp_c CbcSolver
    Cbc
    Cgl
    OsiClp
    Clp
    Osi
    CoinUtils
    zlibd)