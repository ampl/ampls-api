cmake_minimum_required(VERSION 3.3)
project(solvers-public)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to CMake modules.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/support/cmake)

# Initialize the architecture
# Must be done first thing, as it initalizes the
# variable ARCH below
#include(initArchitecture)
#include(createMainSupportProject)
#include(createSolverSupportProject)

# Initial values for solvers builds. If set to 0, the targets are ignored
set(ENABLE_cplex 0)
set(ENABLE_gurobi 1)
set(ENABLE_knitro 0)
set(ENABLE_xpress 0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(addPrefix) # for add_prefix and add_to_folder

# Set global compiler settings
if(MSVC)
  # Disable useless MSVC warnings suggesting nonportable "secure" alternatives.
  add_compile_definitions(-D_CRT_SECURE_NO_WARNINGS)
  # Disable warning about osx file format
  add_compile_options(/wd4335)
endif ()

# Set output directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set paths
set(BASESRCDIR ${CMAKE_CURRENT_SOURCE_DIR}/src/solvers) # Base dir for all solvers

set(SOLVERS gurobi cplex)


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
endif()


set(GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include) # For generated files
file(WRITE ${GENERATED_INCLUDE_DIR}/test-config.h
     "const char MODELS_DIR[] = \"${PROJECT_SOURCE_DIR}/test/models/\";\n")

add_subdirectory(cpp/generic)

set(LIBSDIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
if(WIN32)
  set(LIBSDIR ${LIBSDIR}/win64)
endif()
function(AddSolverStub solver includes libs)
  set(TARGETNAME ${solver}-drv)
  set(DIR_CPP ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver})
  set(DIR_CPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/src)
  set(DIR_CPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/include)
  set(DIR_SWIG ${CMAKE_CURRENT_SOURCE_DIR}/cpp/${solver}/swig)
  set(DIR_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/python/${solver})
  set(DIR_CSHARP ${CMAKE_CURRENT_SOURCE_DIR}/csharp/${solver})

  # Temporarily, on windows I find both import lib and dll file
  # on other systems i'll need just one
  set(IMPORTLIB ${solver}-lib-import)
  set(LIBFILENAME ${solver}-lib)
  find_library(LIBFILE ${LIBFILENAME} 
    PATHS ${LIBSDIR})
  find_file (DLLFILE ${LIBFILENAME}.dll 
    PATHS ${LIBSDIR})
  message("LIBFILE ${LIBFILE}")
  message("DLLFILE ${DLLFILE}")
  add_library(${IMPORTLIB} STATIC IMPORTED)
  set_property(TARGET ${IMPORTLIB}
    PROPERTY IMPORTED_LOCATION ${LIBFILE})


  add_prefix(DRV_SOURCES ${DIR_CPP}/
    include/${solver}_interface.h src/${solver}_interface.cpp
    include/${solver}_callback.h src/${solver}_callback.cpp)
  add_library(${TARGETNAME} STATIC ${DRV_SOURCES})
  target_link_libraries(${TARGETNAME} simpleapi ${libs})
  target_include_directories(${TARGETNAME} 
      PUBLIC ${includes} ${DIR_CPP_INCLUDE} ${SIMPLEAPI_INCLUDE})
  add_to_folder(${solver} ${TARGETNAME})
  if(NOT WIN32)
   target_compile_options(${TARGETNAME} PRIVATE -fPIC)
  endif()
  # Tests
  add_prefix(TEST_SOURCES ${DIR_CPP}/test/
    ${solver}-test.cpp)
  add_executable(${TARGETNAME}-test ${TEST_SOURCES})
  target_link_libraries(${TARGETNAME}-test ${libs} ${TARGETNAME} ${IMPORTLIB})
  target_include_directories(${TARGETNAME}-test PRIVATE
    ${GENERATED_INCLUDE_DIR} # for test-config
     ${DIR_CPP_INCLUDE} ${SIMPLEAPI_INCLUDE}
     ${includes})
  add_to_folder(${solver} ${TARGETNAME}-test)
  target_compile_definitions(${TARGETNAME}-test PRIVATE SWIG)


    if(SWIG_FOUND)
      include(UseSWIG)
        # Get rid of warning in newer versions of cmake
      macro(ADD_SWIG_LIBRARY name language)
        if(CMAKE_VERSION VERSION_LESS 3.8)
          swig_add_module(${name} ${language} ${ARGN})
        else()
          swig_add_library(${name} 
            LANGUAGE ${language} TYPE MODULE
            SOURCES ${ARGN})
        endif()
      endmacro()

      # Create custom target for common swig files,
      # couldn't find a way to add them as headers
      add_custom_target(${solver}-common 
        SOURCES ${SWIGCOMMONSRC} ${DIR_SWIG}/common.i)
      add_to_folder(${solver}/swig ${solver}-common)
      add_subdirectory(${DIR_PYTHON})
      if(WIN32)
        add_subdirectory(${DIR_CSHARP})
      endif()
    endif()
endfunction()

find_package(GUROBI)

if(ENABLE_gurobi)
  AddSolverStub(gurobi ${GUROBI_INCLUDE_DIR} ${GUROBI_LIBRARIES})
endif()

#if(ENABLE_CPLEX)
#  find_package(CPLEX)
#  if (CPLEX_FOUND)#
#	  add_subdirectory(solvers/cplex)
	 # add_subdirectory(solvers/ilogcp)
#  endif()
#endif()

#if(ENABLE_GUROBI)
 # find_package(GUROBI)
 # if(GUROBI_FOUND)
#	  add_subdirectory(solvers/gurobi)
#  endif()
#endif()
#if(ENABLE_KNITRO)
#  Require(KNITRO KNITRO_LIB KNITRO_INCLUDE)
#  if(KNITRO_FOUND)#
	#  add_subdirectory(solvers/knitro)
 #endif()
#endif()
if(ENABLE_XPRESS)
  Require(XPRESS XPRESS_INCLUDE XPRESS_LIB)
  if(XPRESS_FOUND)
    add_subdirectory(solvers/xpressmp)
  endif()
endif()

# Install python package structure
#install(
 #   DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/amplpy_gurobi/
 #   DESTINATION amplpy_gurobi/
 #   COMPONENT ${PYTHON_SWIG_API}
#)

#set(CPACK_GENERATOR ZIP)
#set(CPACK_PACKAGE_VERSION ${AMPL_VERSION})
#set(CPACK_PACKAGE_FILE_NAME solvers)
#include(CPack)
