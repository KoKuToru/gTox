cmake_minimum_required(VERSION 2.8)

set(SRC
    File.fbs)
    
ADD_CUSTOM_TARGET(toxmm-flatbuffers ALL)

find_program(FLAT_C_EXECUTABLE flatc)
if(NOT FLAT_C_EXECUTABLE)
    message(FATAL_ERROR "flatc not found")
endif()

foreach(_item ${SRC})
    GET_FILENAME_COMPONENT(_item_we "${_item}" NAME_WE)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h"
        POST_BUILD
        COMMAND "${FLAT_C_EXECUTABLE}" -c -o ./generated --strict-json "${_item}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_item}
    )
    SET_SOURCE_FILES_PROPERTIES("${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h" PROPERTIES GENERATED 1)
    ADD_CUSTOM_TARGET("toxmm-flatbuffers-${_item_we}" ALL DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${_item_we}_generated.h")
    add_dependencies(toxmm-flatbuffers "toxmm-flatbuffers-${_item_we}")
endforeach()
