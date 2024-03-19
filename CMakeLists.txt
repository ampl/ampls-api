cmake_minimum_required(VERSION 3.3)
project(ampls)
set(ampls_VERSION 0.1.8)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to CMake modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/support/cmake)

set(AMPLS_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_SWIG_WRAPPERS "Build swig wrappers" ON)
option(BUILD_AMPLS_DOC "Build AMPLS documentation" OFF)
option(BUILD_AMPLS_TESTS "Build cpp tests" OFF)

# Set solvers to build.
function(defSolver name solvername libtargetname)
  cmake_parse_arguments(DEFSOLVER "" "" "ADDITIONALINCLUDES" ${ARGN})
  list(APPEND AMPLS_SOLVER_NAMES ${name})
  set(AMPLS_SOLVER_NAMES ${AMPLS_SOLVER_NAMES} PARENT_SCOPE)
  set(${name}_SOLVER ${solvername} PARENT_SCOPE)
  set(${name}_TARGET ${libtargetname} PARENT_SCOPE)
  set(${name}_ADDITIONALINCLUDES ${DEFSOLVER_ADDITIONALINCLUDES} PARENT_SCOPE)
endfunction()

defSolver(gurobi gurobi gurobi)
defSolver(cplex cplex cplexmp)
defSolver(xpress xpress xpress)
defSolver(cbcmp cbc cbcmp)
defSolver(copt copt copt)
defSolver(scip scip scip)

set(BUILD_AMPLS "" CACHE STRING "Comma-separated list of solver drivers to build,  set to \"all\" to build all")


if (BUILD AND NOT BUILD_AMPLS)
    set(BUILD_AMPLS ${BUILD})
endif()

set(ENABLED_SOLVERS "")
if (BUILD_AMPLS)
  if (BUILD_AMPLS STREQUAL all)
    message(STATUS "Building all solvers")
    foreach(s ${AMPLS_SOLVER_NAMES})
      set(ENABLE_${s} on)
      set(ENABLED_SOLVERS ${ENABLED_SOLVERS} ${s})
    endforeach()
  else()
    string(REGEX MATCHALL "[^,]+" SOLVERS_TOBUILD "${BUILD_AMPLS}")
    # Disable all
    foreach(s ${AMPLS_SOLVER_NAMES})
      set(ENABLE_${s} OFF)
    endforeach()
    # Enable only the ones specified
    foreach(s ${SOLVERS_TOBUILD})
      if (NOT ${s} IN_LIST AMPLS_SOLVER_NAMES)
        message(WARNING "${s} is not a valid solver library name.")
      endif()
      set(ENABLE_${s} ON)
      set(ENABLED_SOLVERS ${ENABLED_SOLVERS} ${s})
      message(STATUS "Building library for solver ${s}")
    endforeach()
  endif()
else()
  message(WARNING "Specify the solvers to build with -DBUILD_AMPLS=solvername1,..,solvernamen or -DBUILD=all")
endif()

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(addPrefix) # for add_prefix and add_to_folder

# Set global compiler settings
if(MSVC)
  get_directory_property(MYDEFS COMPILE_DEFINITIONS)
  if(NOT MYDEFS MATCHES "_CRT_SECURE_NO_WARNINGS")
    # Disable useless MSVC warnings suggesting nonportable "secure"
    # alternatives.
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Disable warning about osx file format
    add_compile_options(/wd4335)
  endif()
endif()

# Set output directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set paths
set(BASESRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src/solvers) # Base dir for all
# Swig
if(BUILD_SWIG_WRAPPERS)
  find_package(SWIG 3.0)
  if(SWIG_FOUND)
    if(POLICY CMP0078)
      cmake_policy(SET CMP0078 NEW)
    endif()
    if(POLICY CMP0086)
      cmake_policy(SET CMP0086 NEW)
    endif()
    if(POLICY CMP0122)
      cmake_policy(SET CMP0122 NEW)
    endif()
    include(UseSWIG)
  else()
    message(WARNING "SWIG not found, Python and C# bindings will not be generated")
  endif()
else()
  message(WARNING "SWIG bindings not generated. Set option BUILD_SWIG_WRAPPERS to ON")
endif()
set(GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include) # For generated files
file(WRITE ${GENERATED_INCLUDE_DIR}/test-config.h
     "#ifndef _TEST_CONFIG_\n#define _TEST_CONFIG_\nconst char MODELS_DIR[] = \"${PROJECT_SOURCE_DIR}/test/models/\";\n#endif")

# Solver vendors libraries and headers locations
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
  set(SOLVERLIBS ${BASE_SOLVER_LIBS})
  set(gurobi_BASE_DIR ${SOLVERLIBS}/gurobi/1101)
  set(cplex_BASE_DIR ${SOLVERLIBS}/cplex/2211)
  set(copt_BASE_DIR ${SOLVERLIBS}/copt/7.0.3)
  set(xpress_BASE_DIR ${SOLVERLIBS}/xpress/42.01.05)
  set(scip_BASE_DIR ${SOLVERLIBS}/scip)
else()
  set(SOLVERLIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
  set(gurobi_BASE_DIR ${SOLVERLIBS}/gurobi)
  set(cplex_BASE_DIR ${SOLVERLIBS}/cplex)
  set(copt_BASE_DIR ${SOLVERLIBS}/copt)
  set(xpress_BASE_DIR ${SOLVERLIBS}/xpress)
  set(scip_BASE_DIR ${SOLVERLIBS}/scip)
endif()

function(CreateSolverLib name includedirs lib)
    cmake_parse_arguments(CSLIB "" "IMPORTLIB" "" ${ARGN})
    add_library(${name} SHARED IMPORTED)
    if(WIN32)
      set_property(TARGET ${name} PROPERTY IMPORTED_LOCATION ${lib})
      if(NOT CSLIB_IMPORTLIB)
        message(ERROR "Import lib not specified")
      endif()
      set_property(TARGET ${name} PROPERTY IMPORTED_IMPLIB ${CSLIB_IMPORTLIB})
    else()
      set_property(TARGET ${name} PROPERTY IMPORTED_LOCATION ${lib})
    endif()
    set_target_properties(${name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${includedirs})
endfunction()


function(FindSolverLib OUTPUTTARGETNAME BASELIBFILENAME)
    if(WIN32)
        string(TOLOWER ${BASELIBFILENAME} LIBFILENAME)
        string(REPLACE ".dll" ".lib" DLLFILENAME ${LIBFILENAME})
    else()
        set(LIBFILENAME ${BASELIBFILENAME})
    endif()
    find_library(LIB_LIBFILE_${OUTPUTTARGETNAME} ${LIBFILENAME} NAMES ${LIBFILENAME}-lib PATHS ${AMPLS_DIR} REQUIRED NO_DEFAULT_PATH)
    if(NOT LIB_LIBFILE_${OUTPUTTARGETNAME})
      message(FATAL_ERROR "Library ${LIBFILENAME} not found in ${AMPLS_DIR}")
    endif()
    if(WIN32)
      find_file(LIB_DLLFILE_${OUTPUTTARGETNAME} ${BASELIBFILENAME}-lib.dll PATHS ${AMPLS_DIR} REQUIRED NO_DEFAULT_PATH) 
    endif()
    add_library(${OUTPUTTARGETNAME} SHARED IMPORTED)
    if(WIN32)
      set_property(TARGET ${OUTPUTTARGETNAME} PROPERTY IMPORTED_LOCATION ${LIB_DLLFILE_${OUTPUTTARGETNAME}})
      set_property(TARGET ${OUTPUTTARGETNAME} PROPERTY IMPORTED_IMPLIB ${LIB_LIBFILE_${OUTPUTTARGETNAME}})
    else()
      set_property(TARGET ${OUTPUTTARGETNAME} PROPERTY IMPORTED_LOCATION ${LIB_LIBFILE_${OUTPUTTARGETNAME}})
    endif()
endfunction()
set(GRBINC ${gurobi_BASE_DIR}/include)
if(hasParent)
    set(GRBINC ${GRBINC}/gurobi)
endif()
message("Set GRBINC = ${GRBINC}")
if(WIN32)
    set(sys "lib/win64")
    
    CreateSolverLib(gurobi-solverlib ${GRBINC}
    ${gurobi_BASE_DIR}/${sys}/gurobi110.dll 
    IMPORTLIB ${gurobi_BASE_DIR}/${sys}/gurobi110.lib)
    
    CreateSolverLib(cplex-solverlib ${cplex_BASE_DIR}/include
      ${cplex_BASE_DIR}/${sys}/cplex2211.dll
      IMPORTLIB ${cplex_BASE_DIR}/${sys}/cplex2211.lib)
      
     CreateSolverLib(copt-solverlib ${copt_BASE_DIR}/include
      ${copt_BASE_DIR}/${sys}/copt.dll
      IMPORTLIB ${copt_BASE_DIR}/${sys}/copt.lib)
    
    CreateSolverLib(xpress-solverlib ${xpress_BASE_DIR}/include
      ${xpress_BASE_DIR}/${sys}/xprs.dll
      IMPORTLIB ${xpress_BASE_DIR}/${sys}/xprs.lib)

    CreateSolverLib(scip-solverlib ${scip_BASE_DIR}/include
      ${scip_BASE_DIR}/${sys}/libscip.dll
      IMPORTLIB ${scip_BASE_DIR}/${sys}/libscip.lib)

    set(AMPLS_DIR ${SOLVERLIBS}/ampls/win64) # AMPL base driver library locations
elseif(APPLE)
    set(sys "lib/osx64")
    CreateSolverLib(gurobi-solverlib ${GRBINC} 
                      ${gurobi_BASE_DIR}/${sys}/libgurobi100.dylib)

    CreateSolverLib(cplex-solverlib ${cplex_BASE_DIR}/include
      ${cplex_BASE_DIR}/${sys}/libcplex2211.dylib)
      
   CreateSolverLib(copt-solverlib ${copt_BASE_DIR}/include
      ${copt_BASE_DIR}/${sys}/libcopt.dylib)
    
   CreateSolverLib(xpress-solverlib ${xpress_BASE_DIR}/include
      ${xpress_BASE_DIR}/${sys}/libxprs.dylib)

    CreateSolverLib(scip-solverlib ${scip_BASE_DIR}/include
      ${scip_BASE_DIR}/${sys}/libscip.8.0.3.0.dylib)

    set(AMPLS_DIR ${SOLVERLIBS}/ampls/osx64) # AMPL base driver library locations
else()
    set(sys "lib/linux64")
    CreateSolverLib(gurobi-solverlib ${GRBINC}
                      ${gurobi_BASE_DIR}/${sys}/libgurobi100.so)

    CreateSolverLib(cplex-solverlib ${cplex_BASE_DIR}/include
      ${cplex_BASE_DIR}/${sys}/libcplex2211.so)
      
    CreateSolverLib(copt-solverlib ${copt_BASE_DIR}/include
      ${copt_BASE_DIR}/${sys}/libcopt.so)
    
    CreateSolverLib(xpress-solverlib ${xpress_BASE_DIR}/include
      ${xpress_BASE_DIR}/${sys}/libxprs.so.41)

    CreateSolverLib(scip-solverlib ${scip_BASE_DIR}/include
      ${scip_BASE_DIR}/${sys}/libscip.8.0.3.0.so)

    set(AMPLS_DIR ${SOLVERLIBS}/ampls/linux64) # AMPL base driver library locations
endif()

set(gurobi-solver-deps gurobi-solverlib)
set(cplex-solver-deps cplex-solverlib)
set(copt-solver-deps copt-solverlib)
set(xpress-solver-deps xpress-solverlib)
set(cbc-solver-deps Coin::CbcSolver)
set(scip-solver-deps scip-solverlib)

 # Get rid of warning in newer versions of cmake
macro(ADD_SWIG_LIBRARY name language)
  if(CMAKE_VERSION VERSION_LESS 3.8)
    swig_add_module(${name} ${language} ${ARGN})
  else()
    swig_add_library(${name} LANGUAGE ${language}
        TYPE MODULE SOURCES ${ARGN})
  endif()
endmacro()

# Add ampls (generic) project
add_subdirectory(cpp/ampls)


function(AddSolverStub solver targetbase libname)
  cmake_parse_arguments(ADDSOLVER "" "LIBS" "ADDITIONALINCLUDES" ${ARGN})

  set(TARGETNAME ${libname}-drv)
  set(DIR_CPP ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${libname})
  set(DIR_CPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${libname}/src)
  set(DIR_CPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${libname}/include)
  set(DIR_SWIG ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${libname}/swig)
  set(DIR_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/python/${libname})
  set(DIR_CSHARP ${CMAKE_CURRENT_SOURCE_DIR}/csharp/${libname})
  
  # On windows I find both import lib and dll file on other systems
  # i'll need just one
  set(IMPORTLIB ${targetbase}-lib)
  if(NOT TARGET ${IMPORTLIB})
      set(IMPORTLIB ${targetbase}mp-lib)
      if(NOT TARGET ${IMPORTLIB})
        set(IMPORTLIB ${targetbase}-lib)
        FindSolverLib(${IMPORTLIB} ${targetbase})
      endif()
  endif()

  add_prefix(
    DRV_SOURCES ${DIR_CPP}/ include/${libname}_interface.h
    src/${libname}_interface.cpp include/${libname}_callback.h
    src/${libname}_callback.cpp)
  set(libs ${ADDSOLVER_LIBS})
  set(additionalincludes ${ADDSOLVER_ADDITIONALINCLUDES})
  add_library(${TARGETNAME} STATIC ${DRV_SOURCES})
  target_link_libraries(${TARGETNAME} PUBLIC ampls ${libs} ${IMPORTLIB})
  target_include_directories(
    ${TARGETNAME} PUBLIC ${DIR_CPP_INCLUDE} ${ampls_INCLUDE} ${additionalincludes})
  message("add_library(${TARGETNAME} STATIC ${DRV_SOURCES})
          target_link_libraries(${TARGETNAME} PUBLIC ampls ${libs} ${IMPORTLIB})
          target_include_directories(${TARGETNAME} PUBLIC ${DIR_CPP_INCLUDE} ${ampls_INCLUDE} ${additionalincludes})")

  add_to_folder(${targetbase} ${TARGETNAME})
  if(NOT WIN32)
    target_compile_options(${TARGETNAME} PRIVATE -fPIC)
  endif()
  
  set(EXAMPLES loadModel getInformation simpleCuts genericbranch-cplex genericbranch-ampls)
  foreach(exname ${EXAMPLES})
      set(tar ${libname}-${exname}) 
      set(src ${DIR_CPP}/examples/${exname}.cpp)
      if(EXISTS ${src})
        add_executable(${tar} ${src})
        target_include_directories(${tar} PRIVATE
          ${GENERATED_INCLUDE_DIR}) # for test-config
        target_link_libraries(${tar} ampls ${TARGETNAME})
        add_to_folder(${targetbase}/examples ${tar})
      endif()
  endforeach()


  # Tests
  add_prefix(TEST_SOURCES ${DIR_CPP}/test/ ${libname}-test.cpp)
  add_executable(${TARGETNAME}-test ${TEST_SOURCES})
  target_link_libraries(${TARGETNAME}-test ${TARGETNAME})
  target_include_directories(
    ${TARGETNAME}-test
    PRIVATE ${GENERATED_INCLUDE_DIR} # for test-config
            ${DIR_CPP_INCLUDE} ${ampls_INCLUDE} ${includes})
  add_to_folder(${targetbase} ${TARGETNAME}-test)
  target_compile_definitions(${TARGETNAME}-test PRIVATE SWIG)

  # Copy runtime dependencies to binary directory
  foreach(s ${${TARGETNAME}_LIBRARY} $<TARGET_FILE:${IMPORTLIB}>)
  add_custom_command(
            TARGET ${TARGETNAME}-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${s}"
                    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
            COMMENT "Copying ${solvername}-lib to python directory")
  endforeach()

  if(BUILD_SWIG_WRAPPERS AND SWIG_FOUND)
    if (EXISTS ${DIR_SWIG})
          # Add a custom target just to show  the following common sources in some IDEs
          add_custom_target(${solver}-common SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/ampls/swig/ampls-common.i
                                                      ${DIR_SWIG}/${libname}-common.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/python/common-python.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/python/exceptions-python.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/python/common-python-extensions.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/python/createPythonWrapper.cmake
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/csharp/createCsharpProject.cmake
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/csharp/common-csharp.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/python/common-python-overrides.i
                                                      ${CMAKE_CURRENT_SOURCE_DIR}/csharp/exceptions-csharp.i
                                                      )
          add_to_folder(${targetbase}/swig ${solver}-common)
          add_subdirectory(${DIR_PYTHON})
          if(WIN32)
          #  add_subdirectory(${DIR_CSHARP})
          endif()
      endif()
  endif()
endfunction()

# Add the enabled modules
foreach(s ${AMPLS_SOLVER_NAMES})
if(ENABLE_${s})
  set(SOLVERNAME ${${s}_SOLVER})
  addsolverstub(${SOLVERNAME} ${${s}_TARGET} ${s} LIBS ${${SOLVERNAME}-solver-deps}
                ADDITIONALINCLUDES ${${s}_ADDITIONALINCLUDES})
endif()
endforeach()

# Add ampls swig bindings (must do after the targets below, as )
  if(BUILD_SWIG_WRAPPERS AND SWIG_FOUND)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/ampls)
  if(WIN32)
    #add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/csharp/ampls)
  endif()
endif()


# Add the documentation, check for the required programs is implemented
# in the inner CMake file
if(BUILD_AMPLS_DOC)
  add_subdirectory("doc")
endif()

