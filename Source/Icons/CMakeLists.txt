cmake_minimum_required(VERSION 2.8)

set(NAME "gtox-icon.gresource")
set(CNAME "gtox_icon")
set(PREFIX "/org/gtox/icon")
set(OPTIONS "preprocess=\\\"to-pixdata\\\"")

set(SRC
    chat_attach.svg
    chat_detach.svg
    icon_128.svg
    settings.svg
    status_online.svg
    status_offline.svg
    status_busy.svg
    avatar.svg
    plus.svg
    remove.svg
    clipboard.svg
    status_message.svg
    notification.svg
    status_away.svg)

find_program(RSVG_CONVERT_EXECUTEABLE rsvg-convert)
if(NOT RSVG_CONVERT_EXECUTEABLE)
    message(FATAL_ERROR "rsvg-convert not found")
endif()

#GENERATED ICON LIST
set(ICONS_CONVERTED )
set(ICONS_CONVERTED_N )
foreach(_icon ${SRC})
    get_filename_component(_icon_we "${_icon}" NAME_WE)
    set(ICONS_CONVERTED ${ICONS_CONVERTED} "${CMAKE_CURRENT_BINARY_DIR}/${_icon_we}.png")
    set(ICONS_CONVERTED ${ICONS_CONVERTED} "${CMAKE_CURRENT_BINARY_DIR}/${_icon_we}@2.png")
    
    set(ICONS_CONVERTED_N ${ICONS_CONVERTED_N} "${_icon_we}.png")
    set(ICONS_CONVERTED_N ${ICONS_CONVERTED_N} "${_icon_we}@2.png")
    
    #CONVERT ICONS
    add_custom_command(
        OUTPUT "${_icon_we}.png" "${_icon_we}@2.png"
        COMMAND ${RSVG_CONVERT_EXECUTEABLE} "${_icon}" -f png -o "${CMAKE_CURRENT_BINARY_DIR}/${_icon_we}.png"
        COMMAND ${RSVG_CONVERT_EXECUTEABLE} "${_icon}" -x 2 -y 2 -f png -o "${CMAKE_CURRENT_BINARY_DIR}/${_icon_we}@2.png"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_icon}
    )
endforeach()

#ADD ICONS TO RESOURCE
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.h"
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/../Tools/gresource.sh" "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.xml" "${PREFIX}" "${OPTIONS}" ${ICONS_CONVERTED_N}
    COMMAND glib-compile-resources --target="${NAME}.c" --generate-source --c-name "${CNAME}" "${NAME}.xml"
    COMMAND glib-compile-resources --target="${NAME}.h" --generate-header --c-name "${CNAME}" "${NAME}.xml"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${ICONS_CONVERTED}
)

set (GRESOURCE ${GRESOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" PARENT_SCOPE)

ADD_CUSTOM_TARGET(gtox-icon-resource ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")

