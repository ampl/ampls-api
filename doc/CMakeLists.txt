# Check requirements.
find_package(Sphinx)
if(NOT SPHINX_EXECUTABLE)
  message(STATUS "Documentation generation disabled (requires sphinx)")
  return()
endif()

find_program(DOXYGEN_EXECUTABLE doxygen)
if (DOXYGEN_EXECUTABLE)
  add_subdirectory(rst)
else()
  message(STATUS "Cpp documentation disabled (requires doxygen)")
endif ()

             

