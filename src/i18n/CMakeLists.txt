#Find necessary programs: msgcat and msgfmt
find_program(GETTEXT_MSGCAT_EXECUTABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTABLE)
    message(FATAL_ERROR "msgcat not found")
endif()

find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(FATAL_ERROR "msgfmt not found")
endif()


set(TRANSLATIONS
    de
    en
    es
    fr
    it
    ru
)


foreach(trans ${TRANSLATIONS})
    add_custom_command(
        COMMENT "Generate Locale '${trans}'"
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${trans}/LC_MESSAGES/gtox.mo
        POST_BUILD
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${trans}/LC_MESSAGES/

        # Create binary file with translations
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/${trans}.po
            -o ${CMAKE_CURRENT_BINARY_DIR}/${trans}/LC_MESSAGES/gtox.mo

        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${trans}.po
    )

    set(mo_files
        ${mo_files} ${CMAKE_CURRENT_BINARY_DIR}/${trans}/LC_MESSAGES/gtox.mo
    )

    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${trans}"
        DESTINATION share/locale
        FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()

# Create a target that's accessible form parent directory
add_custom_target(gtox-locale DEPENDS ${mo_files})
