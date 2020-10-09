macro(createPythonWrapper solvername)
# Names and paths
find_package(PythonLibs REQUIRED)

set(PYTHON_SWIG_API amplpy_${solvername}_swig) # name of swig generated wrapper

# ############ Create SWIG wrapper #############
include_directories(
  ${PYTHON_INCLUDE_PATH} ${${solvername}_INCLUDE_DIR} # for solver headers
  ${DIR_CPP_INCLUDE} # for solver_interface.h
  ${SIMPLEAPI_INCLUDE})

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(SWIG_PYTHON_MODULE_NAME "amplpy_${solvername}/swig/amplpy_${solvername}_swig")

set(SWIG_PYTHON_WRAPPER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}.py")
set(SWIG_CPP_SOURCE "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.cxx")
set(SWIG_CPP_HEADER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.h")

set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i PROPERTIES CPLUSPLUS
                                                                    ON)
#set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i PROPERTIES SWIG_FLAGS
#                                                                    "-builtin")

add_swig_library(${PYTHON_SWIG_API} python ${SWIG_PYTHON_MODULE_NAME}.i)
if(NOT ${solvername} STREQUAL "simpleapi")
swig_link_libraries(${PYTHON_SWIG_API} ${solvername}-drv                    
                    ${PYTHON_LIBRARIES})
else()
swig_link_libraries(${PYTHON_SWIG_API} ${PYTHON_LIBRARIES})
endif()

# From this moment on, PYTHON_SWIG_API has the correct (prefixed) name Can be
# omitted if cmake supports policies CMP0078 and CMP0086
set(PYTHON_SWIG_API
    ${SWIG_MODULE_${PYTHON_SWIG_API}_REAL_NAME}
    PARENT_SCOPE)
set(PYTHON_SWIG_API ${SWIG_MODULE_${PYTHON_SWIG_API}_REAL_NAME})

# For multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(
    ${PYTHON_SWIG_API} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
                                  ${CMAKE_SWIG_BINDIR})
  set_target_properties(
    ${PYTHON_SWIG_API} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
                                  ${CMAKE_SWIG_OUTDIR})
endforeach()

set(wheel_dir ${CMAKE_CURRENT_SOURCE_DIR}/amplpy_${solvername}/swig)

add_custom_target(
  amplpy_${solvername}_updatewheel
  DEPENDS ${PYTHON_SWIG_API}
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_PYTHON_WRAPPER} ${wheel_dir}
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_CPP_SOURCE}
          ${wheel_dir}/${PYTHON_SWIG_API}_wrap.cxx
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_CPP_HEADER}
          ${wheel_dir}/${PYTHON_SWIG_API}_wrap.h)

add_to_folder(${solvername}/swig/py ${PYTHON_SWIG_API} amplpy_${solvername}_updatewheel)

if(MSVC)
  include_external_msproject(
    amplpy_${solvername}_examples
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/amplpy_${solvername}_examples.pyproj
    amplpy_${solvername}_examples)
  add_to_folder(${solvername}/swig/py amplpy_${solvername}_examples)
endif()

endmacro()