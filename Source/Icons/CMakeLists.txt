cmake_minimum_required(VERSION 2.8)

set(NAME "gtox-icon.gresource")
set(CNAME "gtox_icon")
set(PREFIX "/org/gtox/icon")
set(OPTIONS "compressed=\\\"true\\\"")

set(SRC
    chat-attach-symbolic.svg
    chat-detach-symbolic.svg
    icon_128.svg
    settings-symbolic.svg
    status_online.svg
    status_offline.svg
    status_busy.svg
    avatar.svg
    plus-symbolic.svg
    remove-symbolic.svg
    clipboard-symbolic.svg
    status_message.svg
    notification.svg
    status_away.svg)

find_program(GLIB_COMPILE_RESOURCE_EXECUTEABLE glib-compile-resources)
if(NOT GLIB_COMPILE_RESOURCE_EXECUTEABLE)
    message(FATAL_ERROR "glib-compile-resources not found")
endif()

find_program(BASH_EXECUTEABLE bash)
if(NOT BASH_EXECUTEABLE)
    message(FATAL_ERROR "bash not found")
endif()

#GENERATED ICON LIST
set(ICONS_CONVERTED )
set(ICONS_CONVERTED_N )
foreach(_icon ${SRC})
    get_filename_component(_icon_we "${_icon}" NAME_WE)

    set(ICONS_CONVERTED_N ${ICONS_CONVERTED_N} "${_icon_we}.svg")

    #CONVERT ICONS
    add_custom_command(
        OUTPUT "${_icon_we}.svg"
        COMMAND cp "${_icon}" "${CMAKE_CURRENT_BINARY_DIR}/${_icon_we}.svg"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_icon}
    )
endforeach()

#ADD ICONS TO RESOURCE
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h"
    COMMAND "${BASH_EXECUTEABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${ICONS_CONVERTED_N}
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.c" --generate-source --c-name "${CNAME}" "${NAME}.xml"
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.h" --generate-header --c-name "${CNAME}" "${NAME}.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${ICONS_CONVERTED}
)

set (GRESOURCE ${GRESOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" PARENT_SCOPE)

ADD_CUSTOM_TARGET(gtox-icon-resource ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")

