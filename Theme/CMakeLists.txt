set(THEME
    main.css)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../Generated/theme.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/text2h" THEME "${CMAKE_CURRENT_SOURCE_DIR}/../Generated/theme" ${THEME}
    DEPENDS text2h ${THEME}
)
