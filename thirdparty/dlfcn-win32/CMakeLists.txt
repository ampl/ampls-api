cmake_minimum_required(VERSION 2.8)

if (NOT DEFINED CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project (dlfcn-win32 C)

option(BUILD_SHARED_LIBS "shared/static libs" ON) 
option(BUILD_TESTS "tests?" OFF)

set(headers dlfcn.h)
set(sources dlfcn.c)

if (BUILD_SHARED_LIBS)
  add_definitions(-DSHARED)
endif (BUILD_SHARED_LIBS)

add_library(dl ${sources})
target_include_directories(dl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
