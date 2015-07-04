find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(FATAL_ERROR "msgfmt not found")
endif()

find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
if(NOT GETTEXT_MSGMERGE_EXECUTABLE)
    message(FATAL_ERROR "msgmerge not found")
endif()

find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib)
if(NOT GETTEXT_MSGATTRIB_EXECUTABLE)
    message(FATAL_ERROR "msgattrib not found")
endif()

SET(TRANSLATIONS
    de
    en
    es
    it
    ru
)

foreach(_trans ${TRANSLATIONS})
add_custom_command(
   COMMENT "Generate Locale '${_trans}'"
   OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po"
   POST_BUILD
   COMMAND mkdir -p Locale/${_trans}/LC_MESSAGES
   COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE} --translated "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" -o "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po"
   COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update --no-fuzzy-matching "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/en.po"
   COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update --no-fuzzy-matching "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/template.pot"
   COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" -o "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo"
   DEPENDS Locale/${_trans}.po)
   install(DIRECTORY "${PROJECT_BINARY_DIR}/Locale/${_trans}"
           DESTINATION share/locale
           FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()
