macro(createPythonWrapper solvername basesolvername libstargetname modulename)

set(PYTHON_SWIG_API amplpy_${modulename}_swig) # name of swig generated wrapper

# ############ Create SWIG wrapper #############
if(NOT ${basesolvername} MATCHES ampls) 
    get_target_property(libinclude ${basesolvername}-solverlib INTERFACE_INCLUDE_DIRECTORIES)
    set(includeDir  ${libinclude})
    set(linkLibs    ${linkLibs} ${solvername}-drv)
endif()

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON) # enable use of target_include_directories

set(SWIG_PYTHON_MODULE_NAME "amplpy_${solvername}/swig/amplpy_${modulename}_swig")

set(SWIG_PYTHON_WRAPPER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}.py")
set(SWIG_CPP_SOURCE "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.cxx")
set(SWIG_CPP_HEADER "${CMAKE_SWIG_OUTDIR}/${PYTHON_SWIG_API}PYTHON_wrap.h")

set_source_files_properties(${SWIG_PYTHON_MODULE_NAME}.i 
  PROPERTIES CPLUSPLUS ON
             USE_TARGET_INCLUDE_DIRECTORIES ON)

swig_add_library( ${PYTHON_SWIG_API} 
                  LANGUAGE python 
                  TYPE MODULE
                  SOURCES ${SWIG_PYTHON_MODULE_NAME}.i)

target_link_libraries(${PYTHON_SWIG_API} PUBLIC ${Python3_LIBRARIES} ${linkLibs} )

target_include_directories(${PYTHON_SWIG_API} PUBLIC 
    ${Python3_INCLUDE_DIRS} # Python libs
    ${includeDir}           # for solver headers
    ${DIR_CPP_INCLUDE}      # for solver_interface.h
    ${ampls_INCLUDE}        # for ampls.h
)

# From this moment on, PYTHON_SWIG_API has the correct (prefixed) name Can be
# omitted if cmake supports policies CMP0078 and CMP0086
set(PYTHON_SWIG_API ${SWIG_MODULE_${PYTHON_SWIG_API}_REAL_NAME} PARENT_SCOPE)
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

# Copy generated sources (and rename them to remove the PYTHON suffix that
# Cmake introduces)
add_custom_target(amplpy_${solvername}_updatewheel
  DEPENDS ${PYTHON_SWIG_API}
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_PYTHON_WRAPPER} ${wheel_dir}
  COMMAND ${CMAKE_COMMAND} -DINPUT_SOURCE=${SWIG_CPP_SOURCE} 
  -DOUTPUT_SOURCE=${wheel_dir}/${PYTHON_SWIG_API}_wrap.cxx
  -P "${CMAKE_CURRENT_SOURCE_DIR}/../copyAndModifySource.cmake"
  COMMAND ${CMAKE_COMMAND} -E copy ${SWIG_CPP_HEADER}
          ${wheel_dir}/${PYTHON_SWIG_API}_wrap.h)


add_to_folder(${libstargetname}/swig/py ${PYTHON_SWIG_API} amplpy_${solvername}_updatewheel)
if(MSVC)
add_custom_command(TARGET amplpy_${solvername}_updatewheel
    DEPENDS amplpy_${solvername}_updatewheel
    COMMAND bash prepare.sh win64
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Preparing wheel")
  add_custom_command(TARGET amplpy_${solvername}_updatewheel
    DEPENDS amplpy_${solvername}_updatewheel
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/amplpy_${solvername}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/amplpy_${solvername}
    COMMENT "Copying wheel (${wheel_dir}) to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/amplpy_${solvername}")

endif()
endmacro()