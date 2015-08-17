project(gTox)
cmake_minimum_required(VERSION 2.8)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX1Y)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
endif()

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -rdynamic -g3 -Wno-deprecated")
#-fsanitize=address

find_package(PkgConfig)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0>=3.16)
pkg_check_modules(GSTREAMERMM REQUIRED gstreamermm-1.0)
link_directories(${GTKMM_LIBRARY_DIRS} ${GSTREAMERMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS} ${GSTREAMERMM_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Tox)
include_directories(${TOX_INCLUDE_DIR})

find_package(Flatbuffers 1.10 REQUIRED)
include_directories(${FLATBUFFERS_INCLUDE_DIR})

add_subdirectory(tox)
add_subdirectory(i18n)
add_subdirectory(resources/ui)
add_subdirectory(resources/style)
add_subdirectory(resources/icon)
add_subdirectory(resources/flatbuffers)

set(SOURCES
    main.cpp
    gTox.cpp
    config.cpp

    dialog/error.cpp
    dialog/profile_selection.cpp
    dialog/profile_create.cpp
    dialog/main.cpp
    dialog/chat.cpp
    dialog/settings.cpp

    widget/avatar.cpp
    widget/contact.cpp
    widget/main_menu.cpp
    widget/label.cpp
    widget/chat_input.cpp
    widget/chat_message.cpp
    widget/chat_action.cpp
    widget/chat_bubble.cpp
    widget/chat_file.cpp
    widget/imagescaled.cpp
    widget/videoplayer.cpp

    utils/builder.cpp
    utils/dispatcher.h
    utils/storage.cpp
    utils/gstreamer.cpp
)
SET_SOURCE_FILES_PROPERTIES(${GRESOURCE} PROPERTIES GENERATED 1)
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${GRESOURCE}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${GSTREAMERMM_LIBRARIES} ${TOX_LIBRARY} -lpthread -luuid toxmm)
add_dependencies(${PROJECT_NAME}
     gtox-style-resource
     gtox-icon-resource
     gtox-ui-resource
     gtox-flatbuffers-resource
     gtox-locale
     gtox-icon
)
install(PROGRAMS "${PROJECT_BINARY_DIR}/${PROJECT_NAME}"
        DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

#GENRATE POT FILES
find_program(GETTEXT_XGETTEXT_EXECUTEABLE xgettext)
if(NOT GETTEXT_XGETTEXT_EXECUTEABLE)
    message(FATAL_ERROR "xgettext not found")
endif()

find_program(GETTEXT_MSGCAT_EXECUTEABLE msgcat)
if(NOT GETTEXT_MSGCAT_EXECUTEABLE)
    message(FATAL_ERROR "msgcat not found")
endif()

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(LAYOUT_P "resources/ui" ${LAYOUT})
add_custom_command(
   COMMENT "Generate pot file"
   TARGET ${PROJECT_NAME}
   PRE_BUILD
   COMMAND ${GETTEXT_XGETTEXT_EXECUTEABLE} -o "${CMAKE_CURRENT_SOURCE_DIR}/i18n/source.pot" --add-location=file --from-code=UTF-8 --sort-output --c++ --omit-header -k_ -s ${SOURCES}
   COMMAND ${GETTEXT_XGETTEXT_EXECUTEABLE} -o "${CMAKE_CURRENT_SOURCE_DIR}/i18n/layout.pot" --add-location=file --from-code=UTF-8 --sort-output --keyword=translatable -k_ -s ${LAYOUT_P}
   COMMAND ${GETTEXT_MSGCAT_EXECUTEABLE} -s "${CMAKE_CURRENT_SOURCE_DIR}/i18n/source.pot" "${CMAKE_CURRENT_SOURCE_DIR}/i18n/layout.pot" > "${CMAKE_CURRENT_SOURCE_DIR}/i18n/template.pot"
   DEPENDS ${SOURCES} ${LAYOUT_P}
   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

#.desktop file
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
