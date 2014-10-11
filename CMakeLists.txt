project(gTox)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -rdynamic -g3")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
pkg_check_modules(LIBNOTIFYMM libnotifymm-1.0)
link_directories(
    ${GTKMM_LIBRARY_DIRS} ${LIBNOTIFYMM_LIBRARY_DIRS})
include_directories(
    ${GTKMM_INCLUDE_DIRS} ${LIBNOTIFYMM_INCLUDE_DIRS})

#if(NOT SUBMODULES_INITED)
  execute_process(
    COMMAND git submodule init
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  execute_process(
    COMMAND git submodule update
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  set(SUBMODULES_INITED TRUE CACHE BOOL "Are submodules pulled ?" FORCE)
#endif()

#if(NOT TOXCORE_INITED)
  execute_process(
    COMMAND ./autogen.sh
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
  )
  execute_process(
    COMMAND ./configure
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
  )
  set(TOXCORE_INITED TRUE CACHE BOOL "Is toxcore configured ?" FORCE)
#endif()

add_custom_target(
   toxcore
   COMMAND make
   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/toxcore"
)

add_subdirectory(Tools)

set(ICONS
    Icons/chat_attach.svg
    Icons/chat_detach.svg
    Icons/icon_128.svg
    Icons/settings.svg)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
    COMMAND "${PROJECT_BINARY_DIR}/Tools/svg2h" ICON Generated/icon ${ICONS}
    DEPENDS text2h ${ICONS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(${PROJECT_NAME}
    main.cpp
    Widget/WidgetChat.cpp
    Widget/WidgetContact.cpp
    Widget/WidgetContactListItem.cpp
    Widget/WidgetChatBox.cpp
    Dialog/DialogChat.cpp
    Dialog/DialogContact.cpp
    Widget/WidgetNotification.cpp
    Tox/Tox.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/Generated/icon.cpp"
)
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${LIBNOTIFYMM_LIBRARIES} "${CMAKE_CURRENT_SOURCE_DIR}/toxcore/build/.libs/libtoxcore.a" -lsodium -lpthread)
add_dependencies(${PROJECT_NAME} toxcore)
