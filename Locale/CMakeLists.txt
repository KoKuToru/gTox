SET(TRANSLATIONS
    de
)

foreach(_trans ${TRANSLATIONS})
add_custom_command(
   COMMENT "Generate Locale '${_trans}'"
#  TARGET ${PROJECT_NAME}
   OUTPUT "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo"
   PRE_BUILD
   COMMAND mkdir -p Locale/${_trans}/LC_MESSAGES
   COMMAND msgfmt "${CMAKE_CURRENT_SOURCE_DIR}/${_trans}.po" -o "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo"
   DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_trans}.po")
   install(DIRECTORY "${PROJECT_BINARY_DIR}/Locale/${_trans}"
           DESTINATION share/locale
           FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
   add_custom_target("lang_${_trans}" ALL DEPENDS "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo")
endforeach()
