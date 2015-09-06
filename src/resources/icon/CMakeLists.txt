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

#ADD ICONS TO RESOURCE
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h"
    COMMAND "${BASH_EXECUTEABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${SRC}
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.c" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-source --c-name "${CNAME}" "${NAME}.xml"
    COMMAND "${GLIB_COMPILE_RESOURCE_EXECUTEABLE}" --target="${NAME}.h" --sourcedir="${CMAKE_CURRENT_SOURCE_DIR}" --generate-header --c-name "${CNAME}" "${NAME}.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SRC}
)

set (GRESOURCE ${GRESOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" PARENT_SCOPE)

ADD_CUSTOM_TARGET(gtox-icon-resource ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")

find_program(RSVG_CONVERT_EXECUTEABLE rsvg-convert)
if(NOT RSVG_CONVERT_EXECUTEABLE)
    message(FATAL_ERROR "rsvg-convert not found")
endif()

#Application icon
SET(SIZES
    14
    16
    22
    24
    32
    36
    48
    64
    72
    96
    128
    192
    256
    512
)
foreach(size ${SIZES})
    set(icon_path
        "${CMAKE_CURRENT_BINARY_DIR}/hicolor/${size}x${size}/apps"
    )

    #convert svg to png with rsvg
    add_custom_command(
        COMMENT "Generate Icon '${size}x${size}'"
        OUTPUT "${icon_path}/gtox.png"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        POST_BUILD

        #make sure folder exists
        COMMAND mkdir -p "${icon_path}"

        # convert
        COMMAND ${RSVG_CONVERT_EXECUTEABLE} "icon.svg" -w ${size} -h ${size} -f png -o "${icon_path}/gtox.png"

        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icon.svg"
    )

    set(icons
        ${icons} "${icon_path}/gtox.png"
    )
endforeach()

add_custom_command(
    COMMENT "Generate Icon 'scalable'"
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/hicolor/scalable/apps/gtox.svg"
    COMMAND mkdir -p "${CMAKE_CURRENT_BINARY_DIR}/hicolor/scalable/apps"
    COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/icon.svg" "${CMAKE_CURRENT_BINARY_DIR}/hicolor/scalable/apps/gtox.svg"
    POST_BUILD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/icon.svg)

add_custom_target(gtox-icon DEPENDS ${icons} ${CMAKE_CURRENT_BINARY_DIR}/hicolor/scalable/apps/gtox.svg)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/hicolor"
    DESTINATION share/icons
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

