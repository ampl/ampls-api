# Simple API Lib
set(ampls_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ampls_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(ampls_INCLUDE ${ampls_DIR}/include)
set(ampls_INCLUDE ${ampls_DIR}/include PARENT_SCOPE)

add_prefix(ampls_SOURCES ${ampls_DIR}/
  src/csvReader.h 
  src/csvReader.cpp
  src/ampls.cpp
  include/ampls/ampls.h)

set(LINKLIBS "")
if(CMAKE_DL_LIBS)
  append(LINKLIBS ${CMAKE_DL_LIBS})
endif()
if(LINUX)
  append(LINKLIBS rt)
endif()

add_library(ampls STATIC ${ampls_SOURCES})
target_include_directories(ampls PRIVATE ${ampls_DIR}
      PUBLIC ${ampls_INCLUDE})
if(NOT WIN32)
  target_compile_options(ampls PRIVATE -fPIC)
  find_package(Threads REQUIRED)
  append(LINKLIBS Threads::Threads)
endif()
target_link_libraries(ampls ${LINKLIBS})
target_compile_features(ampls PUBLIC cxx_std_11)
add_to_folder(ampls ampls)

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
        addExample(${e} ampls ${ampls_DIR}/examples/${e}.cpp)
    endforeach()
    add_prefix(TEST_SOURCES ${ampls_DIR}/test/
      api-test.cpp)
    add_executable(ampls-test ${TEST_SOURCES})
    target_compile_definitions(ampls-test PRIVATE ${PREPROCESSORDEFS})
    target_include_directories(ampls-test PRIVATE
       ${GENERATED_INCLUDE_DIR} # for test-config
     )
    target_link_libraries(ampls-test ${LINKLIBS})
    add_to_folder(ampls ampls-test)
endif()
#install(
    #FILES ${ampls_SOURCES}
    #DESTINATION amplpy_gurobi/amplpy_gurobi/ampls/
#)
#install(
    #DIRECTORY ${ampls_INCLUDE}/
    #DESTINATION amplpy_gurobi/amplpy_gurobi/ampls/include/
#)
