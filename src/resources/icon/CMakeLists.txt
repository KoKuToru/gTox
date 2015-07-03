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
foreach(_size ${SIZES})
    add_custom_command(
        COMMENT "Generate Icon '${_size}x${_size}'"
        OUTPUT "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}x${_size}/apps/gTox.png"
        COMMAND mkdir -p "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}x${_size}/apps"
        #COMMAND convert -density 1024 Icons/icon.svg -resize "${_size}^" "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}x${_size}/apps/gTox.png"
        COMMAND ${RSVG_CONVERT_EXECUTEABLE} Icons/icon.svg -w ${_size} -h ${_size} -f png -o "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}x${_size}/apps/gTox.png"
        POST_BUILD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS Icons/icon.svg)
    install(DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}x${_size}"
           DESTINATION share/icons/hicolor
           FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()

add_custom_command(
    COMMENT "Generate Icon 'scalable'"
    OUTPUT "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable/apps/gTox.svg"
    COMMAND mkdir -p "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable/apps"
    COMMAND cp Icons/icon.svg "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable/apps/gTox.svg"
    POST_BUILD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS Icons/icon.svg)
install(DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable"
    DESTINATION share/icons/hicolor
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

