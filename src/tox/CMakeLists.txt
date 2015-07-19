project(Toxmm)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig)
pkg_check_modules(GLIBMM glibmm-2.4)
link_directories(${GLIBMM_LIBRARY_DIRS})
include_directories(${GLIBMM_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
find_package(Tox)
include_directories(${TOX_INCLUDE_DIR})

set(SOURCES

    types.cpp
    core.cpp
    exception.cpp
    profile.cpp
    storage.cpp
    contact/manager.cpp
    contact/contact.cpp
    contact/receipt.cpp
    contact/file/manager.cpp
    contact/file/file.cpp
    contact/file/file_recv.cpp
    contact/file/file_send.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${SOURCES}
)
target_link_libraries(${PROJECT_NAME} ${GLIBMM_LIBRARIES} ${TOX_LIBRARY})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#add_dependencies(${PROJECT_NAME})
