# C# API
# Names and paths
set(CSHARP_SWIG_API cpxsharp_c) # name of swig generated wrapper


find_package(CSharp)
# Use Csharp
include(${CSHARP_USE_FILE})


############# Create SWIG wrapper #############
include_directories(
   ${CPLEX_INCLUDE_DIR}/ilcplex # for cplex headers
  ${CPLEX_API_DIR} # for gurobi api includes
 ${SIMPLEAPI_INCLUDE}
 ) 

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/swig)
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cpxsharp_c)
set(SWIG_CSHARP_MODULE_NAME "cpxsharp_c")
set_source_files_properties(${SWIG_CSHARP_MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
SET(SWIG_DEPENDS ../common/common.i)
list(APPEND CMAKE_SWIG_FLAGS "-namespace;cpxsharp")
add_swig_library(${CSHARP_SWIG_API} csharp ${SWIG_CSHARP_MODULE_NAME}.i)

target_link_libraries(${CSHARP_SWIG_API} cplexdrv-lib)
add_to_folder(cplex/swig/csharp ${CSHARP_SWIG_API})

# For multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set_target_properties(${CSHARP_SWIG_API} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SWIG_BINDIR})
	set_target_properties(${CSHARP_SWIG_API} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SWIG_OUTDIR})
endforeach()


############ C# API Assembly ############
set(CSHARP_API cpxsharp)
set(CSHARP_LIBRARY_NAME cpxsharp)
set(CSHARP_OUTPUT_NAME ${CSHARP_LIBRARY_NAME})
add_prefix(CPXSHARP_ADDITIONALFILES ${CMAKE_CURRENT_SOURCE_DIR}/api/
Utils.cs)
# Set global out directory for all csharp projects to the main bin directory
set(CSHARP_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Configure AssemblyInfo.cs with appropriate versioning
 configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs.in
    ${CMAKE_CURRENT_SOURCE_DIR}/api/Properties/AssemblyInfo.cs @ONLY)

# Add high level API
CSHARP_ADD_LIBRARY(${CSHARP_API}
   ${GSHARP_ADDITIONALFILES}
    ${CMAKE_SWIG_OUTDIR}/*.cs)
add_dependencies(${CSHARP_API} ${CSHARP_SWIG_API})
add_to_folder(cplex/swig/csharp ${CSHARP_API})


# Library development
if(MSVC)
  set(CPXSHARP_API_SOURCES_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/api)
  set(CPXSHARP_PROJECT_SOURCE ${CPXSHARP_API_SOURCES_DIR}/cpxsharp.csproj.in)
  set(CPXSHARP_PROJECT_DESTINATION ${CPXSHARP_API_SOURCES_DIR}/dev_cpxsharp.csproj)

  set(CPXSHARP_SWIGFILESLOCATION ${CMAKE_SWIG_OUTDIR})
  configure_file(ConfigureCSharpLib.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake @ONLY)
  add_custom_command(OUTPUT ${CPXSHARP_PROJECT_DESTINATION}
    COMMAND ${CMAKE_COMMAND} -P
      ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake
    DEPENDS
      ConfigureCSharpLib.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake
      ${CPXSHARP_PROJECT_SOURCE}
      ${CSHARP_SWIG_API}
    COMMENT "Configuring ${CPXSHARP_PROJECT_SOURCE}"
    VERBATIM)
  add_custom_target(dev_configure_cpxsharp_project ALL
    DEPENDS ${CPXSHARP_PROJECT_DESTINATION}
    ${SWIG_MODULE_csharpapi_TARGET_NAME})

  include_external_msproject(dev-cpxsharp-api
                      ${CPXSHARP_PROJECT_DESTINATION}
                      dev_configure_cpxsharp_project)
  add_dependencies(dev-cpxsharp-api dev_configure_cpxsharp_project)
    add_to_folder(cplex/swig/csharp dev_configure_cpxsharp_project dev-cpxsharp-api)
endif()


if(MSVC)
  include_external_msproject(cpxsharp-test
                 ${CMAKE_CURRENT_SOURCE_DIR}/cpxsharp-test/cpxsharp-test.csproj
                cpxsharp-test)
                add_to_folder(cplex/swig/csharp cpxsharp-test)

endif()



