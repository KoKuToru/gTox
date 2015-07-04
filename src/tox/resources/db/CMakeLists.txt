cmake_minimum_required(VERSION 2.8)

project(ToxmmResourcesDb)

set(NAME "toxmm-db.gresource")
set(CNAME "toxmm_db")
set(PREFIX "/org/toxmm/db")
set(OPTIONS "compressed=\\\"true\\\"")

set(SRC
    version_001.sql
    version_002.sql
    version_003.sql
    version_004.sql
    version_005.sql
    version_006.sql
    version_007.sql
    version_008.sql
    version_009.sql
    version_010.sql
    version_011.sql
    version_012.sql
    version_013.sql)

find_program(GLIB_COMPILE_RESOURCE_EXECUTEABLE glib-compile-resources)
if(NOT GLIB_COMPILE_RESOURCE_EXECUTEABLE)
    message(FATAL_ERROR "glib-compile-resources not found")
endif()

find_program(BASH_EXECUTEABLE bash)
if(NOT BASH_EXECUTEABLE)
    message(FATAL_ERROR "bash not found")
endif()

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h"
#   COMMAND "${BASH_EXECUTEABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${SRC}
    COMMAND "${BASH_EXECUTEABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${SRC}
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.c" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-source --c-name "${CNAME}"  --manual-register "${NAME}.xml"
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.h" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-header --c-name "${CNAME}"  --manual-register "${NAME}.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SRC}
)

set (GRESOURCE_TOXMM_DB ${GRESOURCE_TOXMM_DB} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")

SET_SOURCE_FILES_PROPERTIES(${GRESOURCE_TOXMM_DB} PROPERTIES GENERATED 1)

add_library(${PROJECT_NAME} STATIC
    ${GRESOURCE_TOXMM_DB}
)
