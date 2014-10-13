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
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/svg2h" ICON Generated/icon ${ICONS}
    DEPENDS svg2h ${ICONS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
