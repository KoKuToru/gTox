cmake_minimum_required(VERSION 2.8)

set(NAME "gtox-flatbuffers.gresource")
set(CNAME "gtox_flatbuffers")
set(PREFIX "/org/gtox/flatbuffers")
set(OPTIONS "compressed=\\\"true\\\"")

set(SRC
    Config.fbs
    ConfigGlobal.fbs)
    
ADD_CUSTOM_TARGET(gtox-flatbuffers-resource ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")

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
    COMMAND "${BASH_EXECUTEABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${SRC}
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.c" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-source --c-name "${CNAME}" "${NAME}.xml"
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.h" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-header --c-name "${CNAME}" "${NAME}.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SRC}
)

find_program(FLAT_C_EXECUTABLE flatc)
if(NOT FLAT_C_EXECUTABLE)
    message(FATAL_ERROR "flatc not found")
endif()

foreach(_item ${SRC})
    GET_FILENAME_COMPONENT(_item_we "${_item}" NAME_WE)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h"
        POST_BUILD
        COMMAND "${FLAT_C_EXECUTABLE}" -c -o ./generated --strict-json "${_item}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_item}
    )
    SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h" PROPERTIES GENERATED 1)
    ADD_CUSTOM_TARGET("gtox-flatbuffers-${_item_we}" ALL DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h")
    add_dependencies(gtox-flatbuffers-resource "gtox-flatbuffers-${_item_we}")
endforeach()

set (GRESOURCE ${GRESOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" PARENT_SCOPE)
SET_SOURCE_FILES_PROPERTIES(${GRESOURCE} PROPERTIES GENERATED 1)
