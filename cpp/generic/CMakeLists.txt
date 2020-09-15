# Simple API Lib
set(SIMPLEAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SIMPLEAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(SIMPLEAPI_INCLUDE ${SIMPLEAPI_DIR}/include)
set(SIMPLEAPI_INCLUDE ${SIMPLEAPI_DIR}/include PARENT_SCOPE)

add_to_source_group(SIMPLEAPI_SOURCES "Simpleapi" ${SIMPLEAPI_DIR}/
  src/csvReader.h src/csvReader.cpp
  src/simpleApi.cpp
  include/simpleapi/simpleApi.h)

add_library(simpleapi STATIC ${SIMPLEAPI_SOURCES})
target_include_directories(simpleapi PRIVATE ${SIMPLEAPI_DIR}
      PUBLIC ${SIMPLEAPI_INCLUDE})
if(NOT WIN32)
  target_compile_options(simpleapi PRIVATE -fPIC)
endif()


# C++ simpleapi examples
add_prefix(EXAMPLES_SOURCES ${SIMPLEAPI_DIR}/examples/
  getInformation.cpp 
  simpleCuts.cpp
  )
add_executable(simpleapi-examples ${EXAMPLES_SOURCES})
target_include_directories(simpleapi-examples PRIVATE
   ${GENERATED_INCLUDE_DIR} # for test-config
 )
target_link_libraries(simpleapi-examples gurobi-drv cplex-drv)
target_compile_definitions(simpleapi-examples PRIVATE SWIG)




# C++ generic routines
add_prefix(TEST_SOURCES ${SIMPLEAPI_DIR}/test/
  api-test.cpp)
add_executable(simpleapi-test ${TEST_SOURCES})
target_include_directories(simpleapi-test PRIVATE
   ${GENERATED_INCLUDE_DIR} # for test-config
 )
target_link_libraries(simpleapi-test gurobi-drv )
target_compile_definitions(simpleapi-test PRIVATE SWIG)





#install(
    #FILES ${SIMPLEAPI_SOURCES}
    #DESTINATION amplpy_gurobi/amplpy_gurobi/simpleapi/
#)
#install(
    #DIRECTORY ${SIMPLEAPI_INCLUDE}/
    #DESTINATION amplpy_gurobi/amplpy_gurobi/simpleapi/include/
#)
