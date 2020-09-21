# Check requirements.
#find_program(DOXYGEN doxygen)
#if (NOT DOXYGEN)
#  message(STATUS "Target doc disabled (requires doxygen)")
#  return ()
#endif ()

             
add_subdirectory(cpp)
