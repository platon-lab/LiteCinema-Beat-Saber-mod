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

set(FIX_INC ${CMAKE_BINARY_DIR}/fix_inc)
file(MAKE_DIRECTORY ${FIX_INC})

file(GLOB_RECURSE H1 "${EXTERN_DIR}/includes/*hooking.hpp")
list(FILTER H1 INCLUDE REGEX "/shared/utils/hooking.hpp$")
list(GET H1 0 H1F)
message(STATUS "HOOK: ${H1F}")
string(REGEX REPLACE "/shared/utils/hooking.hpp$" "" H1R "${H1F}")
file(CREATE_LINK "${H1R}" "${FIX_INC}/beatsaber-hook" SYMBOLIC)

file(GLOB_RECURSE H2 "${EXTERN_DIR}/includes/*modloader.h")
list(FILTER H2 INCLUDE REGEX "/shared/modloader.h$")
list(GET H2 0 H2F)
message(STATUS "SL2: ${H2F}")
string(REGEX REPLACE "/shared/modloader.h$" "" H2R "${H2F}")
file(CREATE_LINK "${H2R}" "${FIX_INC}/scotland2" SYMBOLIC)

target_include_directories(${COMPILE_ID} PRIVATE ${FIX_INC})
