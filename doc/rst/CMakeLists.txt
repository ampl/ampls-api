set(BASEDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../cpp)
set(HEADERDIRS "${BASEDIR}/ampls/include"
                      "${BASEDIR}/gurobi/include"
                      "${BASEDIR}/cplex/include"
                      "${BASEDIR}/xpress/include")

foreach(dir ${HEADERDIRS})
    file(GLOB_RECURSE headers ${dir}/*.h)
    list(APPEND DOC_HEADERS ${headers})
endforeach()

set(DOXYFILE ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(GLOB_RECURSE DOC_SOURCES ${SPHINX_SOURCE}/*.rst)

set(COMMON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/common/definitions.rst
                 ${CMAKE_SOURCE_DIR}/changes.rst)
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                  COMMAND
                  ${SPHINX_EXECUTABLE} -b html
                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  DEPENDS
                   ${DOC_HEADERS} ${DOXYFILE}
                   ${COMMON_FILES} ${DOC_SOURCES}
                   MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                  COMMENT "Generating documentation with Sphinx")
# Nice named target so we can run the job easily
add_custom_target(ampls-doc ALL DEPENDS ${SPHINX_INDEX_FILE}
SOURCES ${DOC_SOURCES} ${COMMON_FILES} ${DOC_HEADERS} ${DOXYFILE})


add_to_folder(docs ampls-doc)
