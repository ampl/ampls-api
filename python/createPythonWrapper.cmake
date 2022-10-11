macro(createPythonWrapper solvername basesolvername libstargetname modulename)
# Names and paths
find_package(PythonLibs REQUIRED)

set(PYTHON_SWIG_API amplpy_${modulename}_swig) # name of swig generated wrapper

# ############ Create SWIG wrapper #############
# Workaround to bypass licensing routines
set(gurobi_INCLUDE_DIR ${gurobi_INCLUDE_DIR}/gurobi)
set(includeDir ${${basesolvername}_INCLUDE_DIR})

include_directories(
  ${PYTHON_INCLUDE_PATH} ${includeDir} # for solver headers
  ${DIR_CPP_INCLUDE} # for solver_interface.h
  ${ampls_INCLUDE})
  message("include_directories( ${PYTHON_INCLUDE_PATH} ${includeDir} # for solver headers "
  "${DIR_CPP_INCLUDE} "# for solver_interface.h
  "${ampls_INCLUDE})")
# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(SWIG_PYTHON_MODULE_NAME "amplpy_${solvername}/swig/amplpy_${modulename}_swig")

set(SWIG_PYTHON_WRAPPER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}.py")
set(SWIG_CPP_SOURCE "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.cxx")
set(SWIG_CPP_HEADER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.h")

set(CMAKE_SWIG_FLAGS "-builtin")
set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i PROPERTIES CPLUSPLUS
                                                                    ON)
add_swig_library(${PYTHON_SWIG_API} python ${SWIG_PYTHON_MODULE_NAME}.i)
if(NOT ${solvername} STREQUAL "ampls")
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
  COMMAND ${CMAKE_COMMAND} -DINPUT_SOURCE=${SWIG_CPP_SOURCE} 
  -DOUTPUT_SOURCE=${wheel_dir}/${PYTHON_SWIG_API}_wrap.cxx
  -P "${CMAKE_CURRENT_SOURCE_DIR}/../copyAndModifySource.cmake"
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_CPP_HEADER}
          ${wheel_dir}/${PYTHON_SWIG_API}_wrap.h)

add_to_folder(${libstargetname}/swig/py ${PYTHON_SWIG_API} amplpy_${solvername}_updatewheel)
if(MSVC)
  include_external_msproject(
    amplpy_${solvername}_examples
    ${CMAKE_CURRENT_SOURCE_DIR}/../examples/amplpy_${solvername}_examples.pyproj
    amplpy_${solvername}_examples)
  add_to_folder(${libstargetname}/swig/py amplpy_${solvername}_examples)
  add_custom_command(TARGET amplpy_${solvername}_updatewheel
    DEPENDS amplpy_${solvername}_updatewheel
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/amplpy_${solvername}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/amplpy_${solvername}
    
   # COMMAND ${CMAKE_COMMAND} -E copy ${${solvername}_DIST} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}

    COMMENT "Copying wheel (${wheel_dir}) to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/amplpy_${solvername}")


if(NOT ${solvername} STREQUAL "ampls")
# Copy ampl solver libs, if defined
add_custom_command(TARGET amplpy_${solvername}_updatewheel
DEPENDS amplpy_${solvername}_updatewheel
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${libstargetname}-lib> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
     COMMENT "Copying $<TARGET_FILE:${libstargetname}-lib> to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
     foreach(lib ${${basesolvername}_LIBRARY}) # copy dependencies
    add_custom_command(TARGET amplpy_${solvername}_updatewheel
      DEPENDS amplpy_${solvername}_updatewheel
      COMMAND ${CMAKE_COMMAND} -E copy ${lib} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
      COMMENT "Copying ${lib} to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

     endforeach()


endif()
endif()
endmacro()