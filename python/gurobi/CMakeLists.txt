# Names and paths
find_package(PythonLibs REQUIRED)

set(PYTHON_SWIG_API amplpy_gurobi_swig) # name of swig generated wrapper

############# Create SWIG wrapper #############
include_directories(${PYTHON_INCLUDE_PATH}  
  ${GUROBI_INCLUDE_DIR} # for gurobi headers
  ${DIR_CPP_INCLUDE} # for gurobi_interface.h
  ${SIMPLEAPI_INCLUDE}) 

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grbpy_c)

#SET(CMAKE_SWIG_FLAGS "-debug-tmsearch")
set(SWIG_PYTHON_MODULE_NAME "amplpy_gurobi/swig/amplpy_gurobi_swig")

set(swig_python_wrapper "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}.py")
set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i PROPERTIES SWIG_FLAGS "-builtin")

add_swig_library(${PYTHON_SWIG_API} python ${SWIG_PYTHON_MODULE_NAME}.i)
swig_link_libraries(${PYTHON_SWIG_API} gurobi-drv ${PYTHON_LIBRARIES})

add_to_folder(gurobi/swig/py ${PYTHON_SWIG_API})
# For whatever reason, UseSWIG SILENTLY prepends an underscore
# to the target name, so I'm getting the resulting name from
# it. https://cmake.org/cmake/help/v3.0/module/UseSWIG.html
# From this moment on, PYTHON_SWIG_API has the correct (prefixed) name
# Can be omitted if cmake supports policies CMP0078 and CMP0086 
set(PYTHON_SWIG_API ${SWIG_MODULE_${PYTHON_SWIG_API}_REAL_NAME} PARENT_SCOPE)
set(PYTHON_SWIG_API ${SWIG_MODULE_${PYTHON_SWIG_API}_REAL_NAME})
target_compile_definitions(${PYTHON_SWIG_API} PRIVATE SWIG)
# For multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
	set_target_properties(${PYTHON_SWIG_API} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SWIG_BINDIR})
	set_target_properties(${PYTHON_SWIG_API} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SWIG_OUTDIR})
endforeach()

#add_custom_command(
#  TARGET ${PYTHON_SWIG_API}
#  POST_BUILD
#  COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/amplpy_gurobi.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
#  COMMAND ${CMAKE_COMMAND} -E copy ${swig_python_wrapper} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
#  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/grbpy-test/grbpy_test.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

if(MSVC)
  include_external_msproject(amplpy_gurobi_examples
                 ${CMAKE_CURRENT_SOURCE_DIR}/examples/amplpy_gurobi_examples.pyproj
                amplpy_gurobi_examples)
                add_to_folder(gurobi/swig/py amplpy_gurobi_examples)
endif()

