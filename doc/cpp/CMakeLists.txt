set(BASEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp)
set(DOXYGEN_INPUT_DIR "${BASEDIR}/ampls/include"
                      "${BASEDIR}/gurobi/include"
                      "${BASEDIR}/cplex/include")

foreach(dir ${DOXYGEN_INPUT_DIR})
    file(GLOB_RECURSE headers ${dir}/*.h)
    list(APPEND DOCHEADERS ${headers})
endforeach()
string (REPLACE ";" " " DOXYGEN_INPUT_DIR "${DOXYGEN_INPUT_DIR}")
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${DOCHEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN} 
                   COMMENT "Generating docs")

add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})


set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(GLOB_RECURSE DOC_SOURCES ${SPHINX_SOURCE}/*.rst)

set(COMMON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../common/definitions.rst)
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                  COMMAND
                  ${SPHINX_EXECUTABLE} -b html
                  # Tell Breathe where to find the Doxygen output
                  -Dbreathe_projects.SolverAPI=${DOXYGEN_OUTPUT_DIR}/doxyxml
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                  DEPENDS
                   # Other docs files you want to track should go here (or in some variable)
                   ${DOC_SOURCES}
                   ${DOXYGEN_INDEX_FILE}
                   ${COMMON_FILES}
                   MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                  COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(sphinx ALL DEPENDS ${SPHINX_INDEX_FILE}
SOURCES ${DOC_SOURCES} ${COMMON_FILES})
add_to_folder(docs/cpp sphinx doxygen)
