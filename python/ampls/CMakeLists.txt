include("../createPythonWrapper.cmake")

set(SOLVERS_INCLUDE "")
set(LINK_LIBS "")
foreach(s ${AMPLS_SOLVER_NAMES})
    if(ENABLE_${s})
        get_target_property(libinclude ${${s}_SOLVER}-solverlib INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(targetinclude ${${s}_SOLVER}-drv INTERFACE_INCLUDE_DIRECTORIES)
        set(LINK_LIBS ${LINK_LIBS} ${${s}_TARGET}-lib ${${s}_TARGET}-drv)
        set(SOLVERS_INCLUDE ${SOLVERS_INCLUDE} ${libinclude} ${targetinclude})
    endif()
endforeach()
set(DIR_CPP_INCLUDE ${DIR_CPP_INCLUDE} ${SOLVERS_INCLUDE})
createPythonWrapper(ampls ampls ampls ampls)
target_link_libraries(amplpy_ampls_swig ${LINK_LIBS})
target_compile_definitions(amplpy_ampls_swig PRIVATE CPLEXD)

if(MSVC)
  include_external_msproject(
    python-tests
    ${TESTDIR}/python-tests.pyproj
    python-tests)
endif()

add_custom_target(python-tests-run
    COMMAND pytest
    WORKING_DIRECTORY ${TESTDIR}
    COMMENT "Running Python tests.")


add_to_folder(ampls/test python-tests-run python-tests)
