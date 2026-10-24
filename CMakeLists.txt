cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(MOD_ID "cinema-lite")
set(MOD_VERSION "0.1.0")
set(COMPILE_ID ${MOD_ID})
project(${MOD_ID})

set(EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared)

macro(RECURSE_FILES return_list filter)
    file(GLOB_RECURSE new_list ${filter})
    set(file_list "")
    foreach(file_path ${new_list})
        set(file_list ${file_list} ${file_path})
    endforeach()
    list(REMOVE_DUPLICATES file_list)
    set(${return_list} ${file_list})
endmacro()

add_library(${COMPILE_ID} SHARED main.cpp)
target_compile_definitions(${COMPILE_ID} PRIVATE
    MOD_ID="${MOD_ID}" VERSION="${MOD_VERSION}" FMT_HEADER_ONLY)
target_compile_options(${COMPILE_ID} PRIVATE
    -Wno-invalid-offsetof -Wno-deprecated-declarations)
target_include_directories(${COMPILE_ID} PRIVATE .)

include(extern.cmake)
target_link_libraries(${COMPILE_ID} PRIVATE -llog)

# Auto-find headers whose folder layout differs between package versions
set(FIX_INC ${CMAKE_BINARY_DIR}/fix_inc)
file(MAKE_DIRECTORY ${FIX_INC})

function(link_pkg pkg_name marker_glob)
    file(GLOB_RECURSE found "${EXTERN_DIR}/${marker_glob}")
    list(LENGTH found n)
    if(n EQUAL 0)
        file(GLOB_RECURSE all "${EXTERN_DIR}/includes/${pkg_name}/*")
        list(LENGTH all cnt)
        message(FATAL_ERROR "NOT FOUND: ${marker_glob} (files in includes
