project(gTox)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -rdynamic -g3")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(LIBNOTIFYMM libnotifymm-1.0)
pkg_check_modules(LIBCANBERRA libcanberra)
link_directories(
    ${GTKMM_LIBRARY_DIRS} ${LIBNOTIFYMM_LIBRARY_DIRS} ${LIBCANBERRA_LIBRARY_DIRS})
include_directories(
    ${GTKMM_INCLUDE_DIRS} ${LIBNOTIFYMM_INCLUDE_DIRS} ${LIBCANBERRA_INCLUDE_DIRS})

if(NOT SUBMODULES_INITED)
  execute_process(
    COMMAND git submodule init
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  execute_process(
    COMMAND git submodule update
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  set(SUBMODULES_INITED TRUE CACHE BOOL "Are submodules pulled ?" FORCE)
endif()

if(NOT TOXCORE_INITED)
  execute_process(
    COMMAND ./autogen.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
  )
  execute_process(
    COMMAND ./configure
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
  )
  set(TOXCORE_INITED TRUE CACHE BOOL "Is toxcore configured ?" FORCE)
endif()

add_custom_target(
   toxcore
   COMMAND make
   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
)

add_subdirectory(SQLiteCpp)
include_directories(SQLiteCpp/include)

add_subdirectory(Tools)

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
    Database/version_6.sql)
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
    Widget/Assistant/WelcomePage.cpp
    Widget/Assistant/AccountWidget.cpp
    Widget/Assistant/NewAccountWidget.cpp
    Widget/Chat/WidgetChatLabel.cpp
    Widget/Chat/WidgetChatEntry.cpp
    Widget/Chat/WidgetChatLayout.cpp
    Widget/Settings/WidgetAbout.cpp
    Widget/Settings/WidgetProfile.cpp
    Widget/Settings/WidgetNetwork.cpp
    Widget/Settings/WidgetCache.cpp
    Dialog/FirstStartAssistant.cpp
    Dialog/DialogChat.cpp
    Dialog/DialogContact.cpp
    Dialog/DialogSettings.cpp
    Dialog/DialogError.cpp
    Widget/WidgetNotification.cpp
    Dialog/Debug/DialogCss.cpp
    Helper/Canberra.cpp
    Helper/ToxEventCallback.cpp
    Tox/Tox.cpp
    Tox/ToxDatabase.cpp
    Tox/ToxException.cpp)

add_executable(${PROJECT_NAME}
     ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/theme.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/database.cpp"
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${LIBNOTIFYMM_LIBRARIES} ${LIBCANBERRA_LIBRARIES} "${CMAKE_CURRENT_SOURCE_DIR}/toxcore/build/.libs/libtoxcore.a" -lsodium -lpthread -lsqlite3 SQLiteCpp)
add_dependencies(${PROJECT_NAME} toxcore SQLiteCpp)
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
)

foreach(_trans ${TRANSLATIONS})
add_custom_command(
   COMMENT "Generate Locale '${_trans}'"
   TARGET ${PROJECT_NAME}
   PRE_BUILD
   COMMAND mkdir -p Locale/${_trans}/LC_MESSAGES
   COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --no-fuzzy-matching "${CMAKE_CURRENT_SOURCE_DIR}/Locale/${_trans}.po" "${CMAKE_CURRENT_SOURCE_DIR}/Locale/en.po" -o "${CMAKE_CURRENT_SOURCE_DIR}/Generated/${_trans}.po"
   COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Generated/${_trans}.po" -o "${PROJECT_BINARY_DIR}/Locale/${_trans}/LC_MESSAGES/gTox.mo"
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
        PRE_BUILD
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
    PRE_BUILD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS Icons/icon.svg)
install(DIRECTORY "${PROJECT_BINARY_DIR}/share/icons/hicolor/scalable"
    DESTINATION share/icons/hicolor
    FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)

find_program(GETTEXT_XGETTEXT_EXECUTEABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTEABLE)
    message(FATAL_ERROR "xgettext not found")
endif()

add_custom_command(
   COMMENT "Generate pot file"
   TARGET ${PROJECT_NAME}
   PRE_BUILD
   COMMAND ${GETTEXT_XGETTEXT_EXECUTEABLE} -o "${CMAKE_CURRENT_SOURCE_DIR}/Locale/template.pot" --from-code=UTF-8 --c++ --omit-header -k_ -s ${SOURCES}
   DEPENDS ${SOURCES}
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
