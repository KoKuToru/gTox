cmake_minimum_required(VERSION 2.8)

set(NAME "gtox-ui.gresource")
set(CNAME "gtox_ui")
set(PREFIX "/org/gtox/ui")
set(OPTIONS "compressed=\\\"true\\\"")

set(SRC
    dialog_profile.ui
    list_item_profile.ui
    list_item_contact.ui
    list_item_notification.ui
    list_item_notification_mini.ui
    dialog_assistant.ui
    dialog_contact.ui
    popover_settings.ui
    dialog_settings.ui
    chat_filerecv.ui
    chat_filepreview.ui
    status_menu.ui
    dialog_chat.ui
    chat_bubble_right.ui)

set(LAYOUT ${SRC} PARENT_SCOPE)

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

set (GRESOURCE ${GRESOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c" PARENT_SCOPE)

ADD_CUSTOM_TARGET(gtox-ui-resource ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.c")


