# Check requirements.
find_package(Sphinx)
if(NOT SPHINX_EXECUTABLE)
  message(STATUS "Documentation generation disabled (requires sphinx)")
  return()
endif()

find_program(DOXYGEN doxygen)
if (DOXYGEN)
  add_subdirectory(cpp)
else()
  message(STATUS "Cpp documentation disabled (requires doxygen)")
endif ()

             

