macro(createSwigProject solvername)
# C# API Names and paths
set(CSHARP_SWIG_API ${solvername}sharp_c) # name of swig generated wrapper

# Use Csharp
find_package(CSharp)
include(${CSHARP_USE_FILE})

# ############ Create SWIG wrapper #############
include_directories(
  ${${solvername}_INCLUDE_DIR} # for solver headers
  ${DIR_CPP_INCLUDE} # for solver api includes
  ${SIMPLEAPI_INCLUDE})

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/swig)
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${solvername}sharp_c)
set(SWIG_CSHARP_MODULE_NAME "swig/${solvername}sharp_c")
set_source_files_properties(${SWIG_CSHARP_MODULE_NAME}.i PROPERTIES CPLUSPLUS
                                                                    ON)
set(SWIG_DEPENDS ../../cpp/${solvername}/swig/${solvername}-common.i)
list(APPEND CMAKE_SWIG_FLAGS "-namespace;${solvername}sharp")
add_swig_library(${CSHARP_SWIG_API} csharp ${SWIG_CSHARP_MODULE_NAME}.i)
if(NOT ${solvername} STREQUAL "simpleapi")
target_link_libraries(${CSHARP_SWIG_API} ${solvername}-drv ${solvername}-lib)
endif()
add_to_folder(${solvername}/swig/csharp ${CSHARP_SWIG_API})

# For multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(
    ${CSHARP_SWIG_API} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
                                  ${CMAKE_SWIG_BINDIR})
  set_target_properties(
    ${CSHARP_SWIG_API} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}
                                  ${CMAKE_SWIG_OUTDIR})
endforeach()

# ########### C# API Assembly ############
set(CSHARP_API ${solvername}sharp)
set(CSHARP_LIBRARY_NAME ${solvername}sharp)
set(CSHARP_OUTPUT_NAME ${CSHARP_LIBRARY_NAME})
add_prefix(CSHARP_ADDITIONALFILES ${CMAKE_CURRENT_SOURCE_DIR}/api/Utils.cs)
# Set global out directory for all csharp projects to the main bin directory
set(CSHARP_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Configure AssemblyInfo.cs with appropriate versioning
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs.in
               ${CMAKE_CURRENT_SOURCE_DIR}/api/Properties/AssemblyInfo.cs @ONLY)

# Add high level API
csharp_add_library(${CSHARP_API} ${GSHARP_ADDITIONALFILES}
                   ${CMAKE_SWIG_OUTDIR}/*.cs)
add_dependencies(${CSHARP_API} ${CSHARP_SWIG_API})
add_to_folder(${solvername}/swig/csharp ${CSHARP_API})

# Library development
if(MSVC)
  set(CSHARP_API_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
  set(CSHARP_PROJECT_SOURCE ${CSHARP_API_SOURCES_DIR}/${solvername}sharp.csproj.in)
  set(CSHARP_PROJECT_DESTINATION
      ${CSHARP_API_SOURCES_DIR}/dev_${solvername}sharp.csproj)

  set(CSHARP_SWIGFILESLOCATION ${CMAKE_SWIG_OUTDIR})
  configure_file(../ConfigureCSharpLib.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake @ONLY)
  add_custom_command(
    OUTPUT ${CSHARP_PROJECT_DESTINATION}
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake
    DEPENDS ../ConfigureCSharpLib.cmake.in
            ${CSHARP_PROJECT_SOURCE} ${CSHARP_SWIG_API}
    COMMENT "Configuring ${CSHARP_PROJECT_SOURCE}"
    VERBATIM)
  add_custom_target(
    dev_configure_${solvername}sharp_project ALL
    DEPENDS ${CSHARP_PROJECT_DESTINATION}
            ${SWIG_MODULE_csharpapi_TARGET_NAME})

  include_external_msproject(dev-${solvername}sharp-api ${CSHARP_PROJECT_DESTINATION}
                             dev_configure_${solvername}sharp_project)
  add_dependencies(dev-${solvername}sharp-api dev_configure_${solvername}sharp_project)
  add_to_folder(${solvername}/swig/csharp dev_configure_${solvername}sharp_project
                dev-${solvername}sharp-api)
endif()

if(MSVC)
  include_external_msproject(
    ${solvername}sharp-test
    ${CMAKE_CURRENT_SOURCE_DIR}/${solvername}sharp-test/${solvername}sharp-test.csproj
    ${solvername}sharp-test)
  add_to_folder(${solvername}/swig/csharp ${solvername}sharp-test)
endif()
endmacro()
