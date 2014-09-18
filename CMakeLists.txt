project(gTox)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -rdynamic -g3")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(
    ${GTKMM_LIBRARY_DIRS}  )
include_directories(
    ${GTKMM_INCLUDE_DIRS}  )

add_custom_target(
   toxcore
   COMMAND make
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/toxcore
)

add_executable(${PROJECT_NAME}
    main.cpp
    Dialog/DialogChat.cpp
)
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/toxcore/build/.libs/libtoxcore.a -lsodium -lpthread)
add_dependencies(${PROJECT_NAME} toxcore)
