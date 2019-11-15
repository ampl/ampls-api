#### Taken from http://www.openflipper.org/svnrepo/CoMISo/trunk/CoMISo/cmake/FindGUROBI.cmake


# - Try to find GUROBI
# Use GUROBI_ROOT as path hint
# Once done this will define
#  GUROBI_FOUND - System has Gurobi
#  GUROBI_INCLUDE_DIRS - The Gurobi include directories
#  GUROBI_LIBRARIES - The libraries needed to use Gurobi
if (GUROBI_INCLUDE_DIR)
  # in cache already
  set(GUROBI_FOUND TRUE)
  set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}" )
  set(GUROBI_LIBRARIES "${GUROBI_LIBRARY}" )
else (GUROBI_INCLUDE_DIR)

find_path(GUROBI_INCLUDE_DIR 
          NAMES gurobi_c.h
          PATHS  "$ENV{GUROBI_INCLUDE_DIR}"
                 "$ENV{GUROBI_HOME}/include"
                 "/Library/gurobi810/mac64/include"
                 "C:\\libs\\gurobi810\\include"
                 "C:\\solvers\\gurobi811\\win64\\include"
                 "${GUROBI_ROOT}/include"
          )
set(GRBLIBRARYNAMES
gurobi gurobi80 gurobi81)

find_library( GUROBI_LIBRARY 
              NAMES ${GRBLIBRARYNAMES} NAMES_PER_DIR
              PATHS "$ENV{GUROBI_LIBRARY}"
                    "$ENV{GUROBI_HOME}/lib"
                    "/Library/gurobi810/mac64/lib"
                    "C:\\libs\\gurobi810\\lib"
                    "C:\\solvers\\gurobi811\\win64\\lib"
                    "${GUROBI_ROOT}/lib"
              )

if(NOT GUROBI_LIBRARY)
  message(FATAL_ERROR "GUROBI_LIBRARY is set to '$ENV{GUROBI_HOME}', but did not find any file matching $ENV{GUROBI_HOME}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}{${GRBLIBRARYNAMES}}${CMAKE_FIND_LIBRARY_SUFFIXES}")
endif()


set(CPPLIBNAME gurobi_c++)

if(WIN32)
    if(NOT (MSVC_VERSION LESS 1910)) 
      set(LIB_SUFFIX md2017)
      set(LIB_SUFFIX_DEBUG mdd2017)
    else()
      set(LIB_SUFFIX md2015)
      set(LIB_SUFFIX_DEBUG mdd2015)
    endif()
  set(CPPLIBNAME ${CPPLIBNAME}${LIB_SUFFIX})
endif()


set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}" )
set(GUROBI_LIBRARIES "${GUROBI_LIBRARY}" )

# use c++ headers as default
# set(GUROBI_COMPILER_FLAGS "-DIL_STD" CACHE STRING "Gurobi Compiler Flags")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBCPLEX_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GUROBI  DEFAULT_MSG
                                  GUROBI_LIBRARY GUROBI_INCLUDE_DIR)

mark_as_advanced(GUROBI_INCLUDE_DIR GUROBI_LIBRARY)

endif(GUROBI_INCLUDE_DIR)

