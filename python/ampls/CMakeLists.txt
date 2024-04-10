set(TESTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../test)
set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../examples)
if(MSVC)
  include_external_msproject(
    python-tests
    ${TESTDIR}/python-tests.pyproj
    python-tests)

    include_external_msproject(
    python-examples
    ${EXAMPLES_DIR}/python-examples.pyproj
    python-examples)
endif()


set(PYTHON_DEPS)
foreach(s ${AMPLS_SOLVER_NAMES})
    if(ENABLE_${s})
      set(PYTHON_DEPS ${PYTHON_DEPS} amplpy_${s}_swig)
    endif()
endforeach()

add_custom_target(python-tests-run
    COMMAND pytest
    WORKING_DIRECTORY ${TESTDIR}
    COMMENT "Running Python tests."
    DEPENDS ${PYTHON_DEPS})


add_to_folder(ampls/test python-tests-run python-tests python-examples)
