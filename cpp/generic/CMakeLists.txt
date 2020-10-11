# Simple API Lib
set(SIMPLEAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SIMPLEAPI_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(SIMPLEAPI_INCLUDE ${SIMPLEAPI_DIR}/include)
set(SIMPLEAPI_INCLUDE ${SIMPLEAPI_DIR}/include PARENT_SCOPE)

add_prefix(SIMPLEAPI_SOURCES ${SIMPLEAPI_DIR}/
  src/csvReader.h 
  src/csvReader.cpp
  src/simpleApi.cpp
  include/simpleapi/simpleApi.h)

set(LINKLIBS "")
if(CMAKE_DL_LIBS)
  append(LINKLIBS ${CMAKE_DL_LIBS})
endif()
if(LINUX)
  append(LINKLIBS rt)
endif()

add_library(simpleapi STATIC ${SIMPLEAPI_SOURCES})
target_include_directories(simpleapi PRIVATE ${SIMPLEAPI_DIR}
      PUBLIC ${SIMPLEAPI_INCLUDE})
if(NOT WIN32)
  target_compile_options(simpleapi PRIVATE -fPIC)
endif()
target_link_libraries(simpleapi ${LINKLIBS})
target_compile_features(simpleapi PUBLIC cxx_std_11)
add_to_folder(simpleapi simpleapi)

# Check which libraries are being built and add only the relevant
# examples and link libs
set(PREPROCESSORDEFS "")
foreach(s ${SOLVER_NAMES})
 if(${ENABLE_${s}})
    set(LINKLIBS ${LINKLIBS} ${s}-drv)
    set(PREPROCESSORDEFS ${PREPROCESSORDEFS} USE_${s}=1)
 endif()
 endforeach()

# Examples
macro(addExample name category src)
    set(tar ${category}-${name}) 
    add_executable(${tar} ${src})
    target_compile_definitions(${tar} PRIVATE ${PREPROCESSORDEFS})
    target_include_directories(${tar} PRIVATE
       ${GENERATED_INCLUDE_DIR} # for test-config
     )
    target_link_libraries(${tar} ${LINKLIBS})
    add_to_folder(${category}/examples ${tar})
endmacro()

if(PREPROCESSORDEFS)
    set(EXAMPLES loadModel getInformation simpleCuts)
    foreach(e ${EXAMPLES})
        addExample(${e} simpleapi ${SIMPLEAPI_DIR}/examples/${e}.cpp)
    endforeach()
    add_prefix(TEST_SOURCES ${SIMPLEAPI_DIR}/test/
      api-test.cpp)
    add_executable(simpleapi-test ${TEST_SOURCES})
    target_compile_definitions(simpleapi-test PRIVATE ${PREPROCESSORDEFS})
    target_include_directories(simpleapi-test PRIVATE
       ${GENERATED_INCLUDE_DIR} # for test-config
     )
    target_link_libraries(simpleapi-test ${LINKLIBS})
    add_to_folder(simpleapi simpleapi-test)
endif()
#install(
    #FILES ${SIMPLEAPI_SOURCES}
    #DESTINATION amplpy_gurobi/amplpy_gurobi/simpleapi/
#)
#install(
    #DIRECTORY ${SIMPLEAPI_INCLUDE}/
    #DESTINATION amplpy_gurobi/amplpy_gurobi/simpleapi/include/
#)
