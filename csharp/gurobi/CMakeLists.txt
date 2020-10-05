# C# API Names and paths
set(CSHARP_SWIG_API gsharp_c) # name of swig generated wrapper

# Use Csharp
find_package(CSharp)
include(${CSHARP_USE_FILE})

# ############ Create SWIG wrapper #############
include_directories(
  ${gurobi_INCLUDE_DIR} # for gurobi header
  ${DIR_CPP_INCLUDE} # for gurobi api includes
  ${SIMPLEAPI_INCLUDE})

# Setting output directories
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/swig)
set(CMAKE_SWIG_BINDIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gsharp_c)
set(SWIG_CSHARP_MODULE_NAME "swig/gsharp_c")
set_source_files_properties(${SWIG_CSHARP_MODULE_NAME}.i PROPERTIES CPLUSPLUS
                                                                    ON)

list(APPEND CMAKE_SWIG_FLAGS "-namespace;gsharp")
add_swig_library(${CSHARP_SWIG_API} csharp ${SWIG_CSHARP_MODULE_NAME}.i)
target_compile_definitions(${CSHARP_SWIG_API} PRIVATE SWIG)
target_link_libraries(${CSHARP_SWIG_API} gurobi-drv)
add_to_folder(gurobi/swig/csharp ${CSHARP_SWIG_API})

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
set(CSHARP_API gsharp)
set(CSHARP_LIBRARY_NAME gsharp)
set(CSHARP_OUTPUT_NAME ${CSHARP_LIBRARY_NAME})
add_prefix(GSHARP_ADDITIONALFILES ${CMAKE_CURRENT_SOURCE_DIR}/api/ Utils.cs)
# Set global out directory for all csharp projects to the main bin directory
set(CSHARP_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# Configure AssemblyInfo.cs with appropriate versioning
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs.in
               ${GSHARP_PROJECT_SOURCES_DIR}/Properties/AssemblyInfo.cs @ONLY)

# Add high level API
csharp_add_library(${CSHARP_API} ${GSHARP_ADDITIONALFILES}
                   ${CMAKE_SWIG_OUTDIR}/*.cs)
add_dependencies(${CSHARP_API} ${CSHARP_SWIG_API})
add_to_folder(gurobi/swig/csharp ${CSHARP_API})

# Library development
if(MSVC)
  set(GSHARP_API_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
  set(GSHARP_PROJECT_SOURCE ${GSHARP_API_SOURCES_DIR}/gsharp.csproj.in)
  set(GSHARP_PROJECT_DESTINATION ${GSHARP_API_SOURCES_DIR}/dev_gsharp.csproj)

  set(GSHARP_SWIGFILESLOCATION ${CMAKE_SWIG_OUTDIR})
  configure_file(ConfigureCSharpLib.cmake.in
                 ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake @ONLY)
  add_custom_command(
    OUTPUT ${GSHARP_PROJECT_DESTINATION}
    COMMAND ${CMAKE_COMMAND} -P
            ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake
    DEPENDS ConfigureCSharpLib.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/ConfigureCSharpLib.cmake
            ${GSHARP_PROJECT_SOURCE} ${CSHARP_SWIG_API}
    COMMENT "Configuring ${GSHARP_PROJECT_SOURCE}"
    VERBATIM)
  add_custom_target(
    dev_configure_gsharp_project ALL
    DEPENDS ${GSHARP_PROJECT_DESTINATION} ${SWIG_MODULE_csharpapi_TARGET_NAME})

  include_external_msproject(dev-grb-sharp-api ${GSHARP_PROJECT_DESTINATION}
                             dev_configure_gsharp_project)
  add_dependencies(dev-grb-sharp-api dev_configure_gsharp_project)
  add_to_folder(gurobi/swig/csharp dev_configure_gsharp_project
                dev-grb-sharp-api)
endif()

if(MSVC)
  include_external_msproject(
    gsharp-examples ${CMAKE_CURRENT_SOURCE_DIR}/examples/gsharp-examples.csproj
    gsharp-examples)
  add_to_folder(gurobi/swig/csharp gsharp-examples)
endif()
