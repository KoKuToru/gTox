set(ICONS
    Icons/chat_attach.svg
    Icons/chat_detach.svg
    Icons/icon_128.svg
    Icons/settings.svg
    Icons/status_online.svg
    Icons/status_offline.svg
    Icons/status_busy.svg
    Icons/avatar.svg
    Icons/plus.svg
    Icons/remove.svg
    Icons/status_away.svg)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../Generated/icon.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/svg2h" ICON "${CMAKE_CURRENT_SOURCE_DIR}/../Generated/icon" ${ICONS}
    DEPENDS svg2h ${ICONS}
)

#Application icon
SET(SIZES
    14x14
    16x16
    22x22
    24x24
    32x32
    36x36
    48x48
    64x64
    72x72
    96x96
    128x128
    192x192
    256x256
    512x512
)
foreach(_size ${SIZES})
add_custom_command(
#   COMMENT "Generate Icon '${_size}'"
    OUTPUT "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}/apps/gTox.png"
    COMMAND mkdir -p "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}/apps"
    COMMAND convert "${CMAKE_CURRENT_SOURCE_DIR}/icon.svg" -resize ${_size}  "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}/apps/gTox.png"
    PRE_BUILD
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icon.svg")
   install(DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}"
           DESTINATION share/icons/hicolor
           FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
   add_custom_target("icon_${_size}" ALL DEPENDS "${PROJECT_BINARY_DIR}/share/icons/hicolor/${_size}/apps/gTox.png")
endforeach()
