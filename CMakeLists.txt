cmake_minimum_required(VERSION 3.3)
project(ampls)
set(ampls_VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to CMake modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/support/cmake)

set(AMPLS_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Set solvers to build.
set(SOLVER_NAMES cplex gurobi xpress)
set(BUILD "" CACHE STRING "Comma-separated list of solver drivers to build,  set to \"all\" to build all")
if (BUILD)
  if (BUILD STREQUAL all)
    message(STATUS "Building all solvers")
    foreach(s ${SOLVER_NAMES})
      set(ENABLE_${s} on)
    endforeach()
  else()
    string(REGEX MATCHALL "[^,]+" SOLVERS_TOBUILD "${BUILD}")
    # Disable all
    foreach(s ${SOLVER_NAMES})
      set(ENABLE_${s} OFF)
    endforeach()
    # Enable only the ones specified
    foreach(s ${SOLVERS_TOBUILD})
      if (NOT ${s} IN_LIST SOLVER_NAMES)
        message(WARNING "${s} is not a valid solver library name.")
      endif()
      set(ENABLE_${s} ON)
      message(STATUS "Building library for solver ${s}")
    endforeach()
  endif()
else()
  message(WARNING "Specify the solvers to build with -DBUILD=solvername1,..,solvernamen or -DBUILD=all")
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
find_package(SWIG 3.0)
if(SWIG_FOUND)
  if(POLICY CMP0078)
    cmake_policy(SET CMP0078 NEW)
  endif()
  if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
  endif()
  include(UseSWIG)
else()
  message(WARNING "SWIG not found, Python and C# bindings will not be generated")
endif()

set(GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include) # For generated files
file(WRITE ${GENERATED_INCLUDE_DIR}/test-config.h
     "const char MODELS_DIR[] = \"${PROJECT_SOURCE_DIR}/test/models/\";\n")

# Solver vendors libraries and headers locations
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
  set(SOLVERLIBS ${CMAKE_CURRENT_SOURCE_DIR}/../solver-libraries)
  set(gurobi_BASE_DIR ${SOLVERLIBS}/gurobi/912)
  set(cplex_BASE_DIR ${SOLVERLIBS}/cplex/2010)
  set(xpress_BASE_DIR ${SOLVERLIBS}/xpress/37.01.03)
else()
  set(SOLVERLIBS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
  set(gurobi_BASE_DIR ${SOLVERLIBS}/gurobi)
  set(cplex_BASE_DIR ${SOLVERLIBS}/cplex)
  set(xpress_BASE_DIR ${SOLVERLIBS}/xpress)
endif()

foreach(s ${SOLVER_NAMES})
  set(${s}_INCLUDE_DIR ${${s}_BASE_DIR}/include)
endforeach()

if(WIN32)
    set(gurobi_LIBRARY "${gurobi_BASE_DIR}/lib/win64/gurobi91.lib"
                       "${gurobi_BASE_DIR}/lib/win64/gurobi91.dll")
    set(cplex_LIBRARY "${cplex_BASE_DIR}/lib/win64/cplex2010.lib"
                      "${cplex_BASE_DIR}/lib/win64/cplex2010.dll")
    set(xpress_LIBRARY ${xpress_BASE_DIR}/lib/win64/xprs.lib)
    set(AMPLS_DIR ${SOLVERLIBS}/ampls/win64) # AMPL base driver library locations
elseif(APPLE)
    set(gurobi_LIBRARY ${gurobi_BASE_DIR}/lib/osx64/libgurobi91.dylib)
    set(cplex_LIBRARY ${cplex_BASE_DIR}/lib/osx64/libcplex2010.dylib)
    set(xpress_LIBRARY ${xpress_BASE_DIR}/lib/osx64/libxprs.dylib
                      ${xpress_BASE_DIR}/lib/osx64/libxprl.dylib)
    set(AMPLS_DIR ${SOLVERLIBS}/ampls/osx64) # AMPL base driver library locations
else()
    set(gurobi_LIBRARY ${gurobi_BASE_DIR}/lib/linux64/libgurobi91.so)
    set(cplex_LIBRARY ${cplex_BASE_DIR}/lib/linux64/libcplex2010.so)
    set(xpress_LIBRARY ${xpress_BASE_DIR}/lib/linux64/libxprs.so.37.01
                        ${xpress_BASE_DIR}/lib/linux64/libxprl.so.x8.11 )
    set(AMPLS_DIR ${SOLVERLIBS}/ampls/linux64) # AMPL base driver library locations
endif()


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


function(AddSolverStub solver includes libs)
  set(TARGETNAME ${solver}-drv)
  set(DIR_CPP ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver})
  set(DIR_CPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/src)
  set(DIR_CPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/include)
  set(DIR_SWIG ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/swig)
  set(DIR_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/python/${solver})
  set(DIR_CSHARP ${CMAKE_CURRENT_SOURCE_DIR}/csharp/${solver})
  # On windows I find both import lib and dll file on other systems
  # i'll need just one

  set(IMPORTLIB ${solver}-lib)
  if(NOT TARGET ${IMPORTLIB})
    set(LIBFILENAME ${solver}-lib)
    find_library(${solver}LIBFILE ${LIBFILENAME} PATHS ${AMPLS_DIR} REQUIRED NO_DEFAULT_PATH) 
    if(NOT ${solver}LIBFILE)
      message(FATAL_ERROR "AMPL solver library ${solver}-lib not found in ${AMPLS_DIR}")
    endif()
    if(WIN32)
      find_file(${solver}DLLFILE ${solver}-lib.dll PATHS ${AMPLS_DIR} REQUIRED NO_DEFAULT_PATH) 
    endif()
    add_library(${IMPORTLIB} SHARED IMPORTED)
    if(WIN32)
      set_property(TARGET  ${IMPORTLIB} PROPERTY IMPORTED_LOCATION ${${solver}DLLFILE})
      set_property(TARGET  ${IMPORTLIB} PROPERTY IMPORTED_IMPLIB ${${solver}LIBFILE})
    else()
      set_property(TARGET ${IMPORTLIB} PROPERTY IMPORTED_LOCATION ${${solver}LIBFILE})
    endif()
  endif()

  add_prefix(
    DRV_SOURCES ${DIR_CPP}/ include/${solver}_interface.h
    src/${solver}_interface.cpp include/${solver}_callback.h
    src/${solver}_callback.cpp)
  add_library(${TARGETNAME} STATIC ${DRV_SOURCES})
  target_link_libraries(${TARGETNAME} ampls ${libs} ${IMPORTLIB})
  target_include_directories(
    ${TARGETNAME} PUBLIC ${includes} ${DIR_CPP_INCLUDE} ${ampls_INCLUDE})
  add_to_folder(${solver} ${TARGETNAME})
  if(NOT WIN32)
    target_compile_options(${TARGETNAME} PRIVATE -fPIC)
  endif()
  
  

  set(EXAMPLES loadModel getInformation simpleCuts)
  foreach(exname ${EXAMPLES})
      set(tar ${solver}-${exname}) 
      set(src ${DIR_CPP}/examples/${exname}.cpp)
      if(EXISTS ${src})
        add_executable(${tar} ${src})
        target_include_directories(${tar} PRIVATE
          ${GENERATED_INCLUDE_DIR}) # for test-config
        target_link_libraries(${tar} ampls ${TARGETNAME})
        add_to_folder(${solver}/examples ${tar})
      endif()
  endforeach()


  # Tests
  add_prefix(TEST_SOURCES ${DIR_CPP}/test/ ${solver}-test.cpp)
  add_executable(${TARGETNAME}-test ${TEST_SOURCES})
  target_link_libraries(${TARGETNAME}-test ${libs} ${TARGETNAME})
  target_include_directories(
    ${TARGETNAME}-test
    PRIVATE ${GENERATED_INCLUDE_DIR} # for test-config
            ${DIR_CPP_INCLUDE} ${ampls_INCLUDE} ${includes})
  add_to_folder(${solver} ${TARGETNAME}-test)
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

  if (EXISTS ${DIR_SWIG})
    if(SWIG_FOUND)
        # Add a custom target just to show  the following common sources in some IDEs
        add_custom_target(${solver}-common SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/ampls/swig/ampls-common.i
                                                   ${DIR_SWIG}/${solver}-common.i
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/python/common-python.i
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/python/common-python-extensions.i
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/python/createPythonWrapper.cmake
                                                   ${CMAKE_CURRENT_SOURCE_DIR}/csharp/createCsharpProject.cmake
                                                   )
        add_to_folder(${solver}/swig ${solver}-common)
        add_subdirectory(${DIR_PYTHON})
        if(WIN32)
          add_subdirectory(${DIR_CSHARP})
        endif()
    endif()
  endif()
endfunction()

# Add the enabled modules
foreach(s ${SOLVER_NAMES})
if(ENABLE_${s})
  addsolverstub(${s} ${${s}_INCLUDE_DIR} ${${s}_LIBRARY})
endif()
endforeach()

# Add ampls swig bindings (must do after the targets below, as )
if(SWIG_FOUND)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/python/ampls)
  if(WIN32)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/csharp/ampls)
  endif()
endif()


# Add the documentation, check for the required programs is implemented
# in the inner CMake file
add_subdirectory("doc")


