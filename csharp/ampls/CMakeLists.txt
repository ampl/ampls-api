include("../createCsharpProject.cmake")


set(SOLVERS_INCLUDE "")
set(LINK_LIBS "")
foreach(s ${AMPLS_SOLVER_NAMES})
    if(ENABLE_${s})
        get_target_property(libinclude ${${s}_SOLVER}-solverlib INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(targetinclude ${${s}_TARGET}-drv INTERFACE_INCLUDE_DIRECTORIES)
        set(LINK_LIBS ${LINK_LIBS} ${${s}_TARGET}-lib ${${s}_TARGET}-drv)
        set(SOLVERS_INCLUDE ${SOLVERS_INCLUDE} ${libinclude} ${targetinclude})
    endif()
endforeach()

set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE} ${SOLVERS_INCLUDE})
createCsharpProject(ampls ampls ampls)
target_link_libraries(amplssharp_c ${LINK_LIBS})