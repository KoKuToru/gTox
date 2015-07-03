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

option(SQLITECPP_RUN_CPPLINT OFF)
option(SQLITECPP_RUN_CPPCHECK OFF)
option(SQLITECPP_RUN_DOXYGEN OFF)
option(SQLITECPP_BUILD_EXAMPLES OFF)
option(SQLITECPP_BUILD_TESTS OFF)
add_subdirectory(libs/SQLiteCpp)
add_subdirectory(resources/db)

set(SOURCES

    types.cpp
    core.cpp
    exception.cpp
    profile.cpp
    contact/manager.cpp
    contact/contact.cpp

    #Toxmm.cpp
    #ToxDatabase.cpp
    #ToxEvent.cpp
    #ToxException.cpp
    #ToxProfile.cpp
)

add_library(${PROJECT_NAME} STATIC
    ${SOURCES}
)
target_link_libraries(${PROJECT_NAME} ${GLIBMM_LIBRARIES} ${TOX_LIBRARY} -lpthread -lsqlite3 SQLiteCpp ToxmmResourcesDb)
target_include_directories(${PROJECT_NAME} PUBLIC libs/SQLiteCpp/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#add_dependencies(${PROJECT_NAME} ToxmmResourcesDb SQLiteCpp)
