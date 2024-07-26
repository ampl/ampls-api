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

target_sources(ampls PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../changes.md)

# Check which libraries are being built and add only the relevant
# examples and link libs
set(PREPROCESSORDEFS "")
foreach(s ${ENABLED_SOLVERS})
    set(LINKLIBS ${LINKLIBS} ${s}-drv)
    set(PREPROCESSORDEFS ${PREPROCESSORDEFS} USE_${s}=1)
 endforeach()
 message("ampls examples linked with ${ENABLED_SOLVERS}")

 # Find amplapi, if present add combined examples
 set(AMPLAPI_VERSION 2.3.8)
if(WIN32)
  set(libdir win64)
  set(libname ampl-${AMPLAPI_VERSION}.lib)
  set(dllname ampl-${AMPLAPI_VERSION}.dll)
elseif(APPLE)
  set(libdir osx64)
  set(libname libampl.${AMPLAPI_VERSION}.dylib)
else()
 set(libdir linux64)
 set(libname libampl.so.${AMPLAPI_VERSION})
endif()
set(AMPLAPIDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/amplapi/${libdir})


if(EXISTS ${AMPLAPIDIR}/lib/${libname})
  set(BUILD_AMPLAPI_EXAMPLES ON)
  add_library(amplapi SHARED IMPORTED)
  if(WIN32)
    set_target_properties(amplapi PROPERTIES
      IMPORTED_IMPLIB ${AMPLAPIDIR}/lib/${libname}
      IMPORTED_LOCATION ${AMPLAPIDIR}/bin/${dllname}
      INTERFACE_INCLUDE_DIRECTORIES ${AMPLAPIDIR}/include)
  else()
    set_target_properties(amplapi PROPERTIES
      IMPORTED_LOCATION ${AMPLAPIDIR}/lib/${libname}
      INTERFACE_INCLUDE_DIRECTORIES ${AMPLAPIDIR}/include)
  endif()
endif()

# Examples
macro(addExample name category src)
    set(tar ${category}-${name}) 
    add_executable(${tar} ${src})
    target_compile_definitions(${tar} PRIVATE ${PREPROCESSORDEFS})
    target_include_directories(${tar} PRIVATE
       ${GENERATED_INCLUDE_DIR} # for test-config
     )
    target_link_libraries(${tar} PRIVATE ${LINKLIBS})
    add_to_folder(${category}/examples ${tar})
endmacro()

if(PREPROCESSORDEFS)
    set(EXAMPLES loadModel getInformation errorDetection stoppingCriteria)
    foreach(e ${EXAMPLES})
        addExample(${e} ampls ${ampls_DIR}/examples/${e}.cpp)
    endforeach()
    
    if(BUILD_AMPLAPI_EXAMPLES)
      set(AMPLAPI_EXAMPLES cuttingStock addEntities tsp multipleModels multipleSolutions setOptions 
                           benders)
      foreach(e ${AMPLAPI_EXAMPLES})
        addExample(${e} ampls ${ampls_DIR}/examples/${e}.cpp)
        target_link_libraries(ampls-${e} PRIVATE amplapi)
        target_compile_definitions(ampls-${e} PRIVATE USE_amplapi)
        add_custom_command ( TARGET ampls-${e} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:amplapi> $<TARGET_FILE_DIR:ampls-${e}>
          COMMENT "Copying amplapi dynamic library to binary directory"
        )
      endforeach()
    endif()
endif()

if(BUILD_AMPLS_TESTS)
    # GoogleTest requires at least C++14
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/06f44bc951046150f1348598854b211afdcf37fc.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_to_folder(ampls/test/gtest gmock gmock_main gtest gtest_main)

    enable_testing()

    set(EXAMPLES_TEST "")
    foreach(s ${EXAMPLES})
        set(EXAMPLES_TEST ${EXAMPLES_TEST} ${s}-test.cpp)
    endforeach()
    set(AMPLAPI_EXAMPLES_TEST "")
    foreach(s ${AMPLAPI_EXAMPLES})
        set(AMPLAPI_EXAMPLES_TEST ${AMPLAPI_EXAMPLES_TEST} ${s}-test.cpp)
    endforeach()
     # Add tests
     # Get sources from examples
    add_prefix(TEST_SOURCES ${ampls_DIR}/test/
          ${EXAMPLES_TEST} ${AMPLAPI_EXAMPLES_TEST})

    add_executable(ampls-test ${TEST_SOURCES} ${ampls_DIR}/test/test-base.h ${ampls_DIR}/test/test-execute.h)
    target_compile_definitions(ampls-test PRIVATE ${PREPROCESSORDEFS})
    target_include_directories(ampls-test PRIVATE 
                     ${ampls_DIR}/examples # to include examples
                    ${GENERATED_INCLUDE_DIR})# for test-config
    
    if(BUILD_AMPLAPI_EXAMPLES)
      target_compile_definitions(ampls-test PRIVATE USE_amplapi)
      target_link_libraries(ampls-test amplapi)
    endif()
    target_link_libraries(ampls-test ${LINKLIBS} GTest::gtest_main)

    add_to_folder(ampls ampls-test)
    include(GoogleTest)
    gtest_discover_tests(ampls-test)
    add_to_folder(ampls/test ampls-test)
endif()