project(gTox)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -rdynamic -g3")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(LIBNOTIFYMM libnotifymm-1.0)
pkg_check_modules(LIBCANBERRA libcanberra)
pkg_check_modules(GSTREAMERMM gstreamermm-1.0)
link_directories(
    ${GTKMM_LIBRARY_DIRS} ${LIBNOTIFYMM_LIBRARY_DIRS} ${LIBCANBERRA_LIBRARY_DIRS} ${GSTREAMERMM_LIBRARY_DIRS})
include_directories(
    ${GTKMM_INCLUDE_DIRS} ${LIBNOTIFYMM_INCLUDE_DIRS} ${LIBCANBERRA_INCLUDE_DIRS} ${GSTREAMERMM_INCLUDE_DIRS})

option(TOXCORE_BUNDLE "TOXCORE_BUNDLE" OFF)
if(TOXCORE_BUNDLE)

else()
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
  find_package(Tox)
  include_directories(${TOX_INCLUDE_DIR})
endif()

option(SQLITECPP_RUN_CPPLINT OFF)
option(SQLITECPP_RUN_CPPCHECK OFF)
option(SQLITECPP_RUN_DOXYGEN OFF)
option(SQLITECPP_BUILD_EXAMPLES OFF)
option(SQLITECPP_BUILD_TESTS OFF)
add_subdirectory(Libs/SQLiteCpp)
include_directories(Libs/SQLiteCpp/include)

add_subdirectory(Tools)

set(LAYOUT
    Layout/dialog_profile.ui
    Layout/list_item_profile.ui
    Layout/dialog_assistant.ui)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/layout.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/text2h" LAYOUT Generated/layout ${LAYOUT}
    DEPENDS text2h ${LAYOUT}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(THEME
    Theme/main.css)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/theme.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/text2h" THEME Generated/theme ${THEME}
    DEPENDS text2h ${THEME}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(DATABASE
    Database/version_1.sql
    Database/version_2.sql
    Database/version_3.sql
    Database/version_4.sql
    Database/version_5.sql
    Database/version_6.sql
    Database/version_7.sql)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/database.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/text2h" DATABASE Generated/database ${DATABASE}
    DEPENDS text2h ${DATABASE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

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
    Icons/clipboard.svg
    Icons/status_message.svg
    Icons/status_away.svg)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/svg2h" ICON Generated/icon ${ICONS}
    DEPENDS svg2h ${ICONS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set(SOURCES
    main.cpp
    Widget/WidgetChat.cpp
    Widget/WidgetContact.cpp
    Widget/WidgetContactListItem.cpp
    Widget/WidgetChatLine.cpp
    Popover/PopoverStatus.cpp
    Popover/PopoverSettings.cpp
    Popover/PopoverAddContact.cpp
    Popover/PopoverContextContact.cpp
    Widget/Chat/WidgetChatLabel.cpp
    Widget/Chat/WidgetChatEntry.cpp
    Widget/Chat/WidgetChatLayout.cpp
    Widget/Chat/WidgetChatBubbleRow.cpp
    Widget/Settings/WidgetAbout.cpp
    Widget/Settings/WidgetProfile.cpp
    Widget/Settings/WidgetNetwork.cpp
    Widget/Settings/WidgetCache.cpp
    Widget/Settings/WidgetAudioVideo.cpp
    Widget/VideoPlayer.cpp
    Dialog/DialogProfileCreate.cpp
    Dialog/DialogChat.cpp
    Dialog/DialogContact.cpp
    Dialog/DialogSettings.cpp
    Dialog/DialogError.cpp
    Dialog/DialogProfile.cpp
    Dialog/Debug/DialogCss.cpp
    Helper/Canberra.cpp
    Helper/ToxEventCallback.cpp
    Tox/Tox.cpp
    Tox/ToxDatabase.cpp
    Tox/ToxEvent.cpp
    Tox/ToxException.cpp)

add_executable(${PROJECT_NAME}
     ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/theme.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/layout.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/database.cpp"
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${LIBNOTIFYMM_LIBRARIES} ${LIBCANBERRA_LIBRARIES} ${GSTREAMERMM_LIBRARIES} ${TOX_LIBRARY} -lpthread -lsqlite3 SQLiteCpp)
add_dependencies(${PROJECT_NAME} SQLiteCpp)
install(PROGRAMS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}"
        DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(FATAL_ERROR "msgfmt not found")
endif()

find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)
if(NOT GETTEXT_MSGMERGE_EXECUTABLE)
    message(FATAL_ERROR "msgmerge not found")
endif()

SET(TRANSLATIONS
    de
    en
    ru
)

foreach(_trans ${TRANSLATIONS})
add_custom_command(
   COMMENT "Generate Locale '${_trans}'"
   TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND mkdir -p Locale/${_trans}/LC_MESSAGES
   COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update --no-fuzzy-matching "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/en.po"
   COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --update --no-fuzzy-matching "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/template.pot"
   COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" -o "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo"
   DEPENDS Locale/${_trans}.po)
   install(DIRECTORY "${PROJECT_BINARY_DIR}/Locale/${_trans}"
           DESTINATION share/locale
           FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()

#Application icon
find_program(RSVG_CONVERT_EXECUTEABLE rsvg-convert)
if(NOT RSVG_CONVERT_EXECUTEABLE)
    message(FATAL_ERROR "rsvg-convert not found")
endif()

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
        TARGET ${PROJECT_NAME}
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
    TARGET ${PROJECT_NAME}
    COMMAND mkdir -p "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable/apps"
    COMMAND cp Icons/icon.svg "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable/apps/gTox.svg"
    POST_BUILD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS Icons/icon.svg)
install(DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable"
    DESTINATION share/icons/hicolor
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

find_program(GETTEXT_XGETTEXT_EXECUTEABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTEABLE)
    message(FATAL_ERROR "xgettext not found")
endif()

find_program(GETTEXT_MSGCAT_EXECUTEABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTEABLE)
    message(FATAL_ERROR "msgcat not found")
endif()

add_custom_command(
   COMMENT "Generate pot file"
   TARGET ${PROJECT_NAME}
   PRE_BUILD
   COMMAND ${GETTEXT_XGETTEXT_EXECUTEABLE} -o "${CMAKE_CURRENT_SOURCE_DIR}/Locale/source.pot" --from-code=UTF-8 --sort-output --c++ --omit-header -k_ -s ${SOURCES}
   COMMAND ${GETTEXT_XGETTEXT_EXECUTEABLE} -o "${CMAKE_CURRENT_SOURCE_DIR}/Locale/layout.pot" --from-code=UTF-8 --sort-output --keyword=translatable -k_ -s ${LAYOUT}
   COMMAND ${GETTEXT_MSGCAT_EXECUTEABLE} -us "${CMAKE_CURRENT_SOURCE_DIR}/Locale/source.pot" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/layout.pot" > "${CMAKE_CURRENT_SOURCE_DIR}/Locale/template.pot"
   DEPENDS ${SOURCES} ${LAYOUT}
   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(
    COMMENT "Copy .desktop"
    TARGET ${PROJECT_NAME}
    COMMAND mkdir -p "${PROJECT_BINARY_DIR}/share/applications"
    COMMAND cp gTox.desktop "${PROJECT_BINARY_DIR}/share/applications/gTox.desktop"
    DEPENDS gTox.desktop
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
install(DIRECTORY "${PROJECT_BINARY_DIR}/share/applications"
    DESTINATION share
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
