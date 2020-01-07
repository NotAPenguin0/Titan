cmake_minimum_required(VERSION 3.12)

project(Titan)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

set(TITAN_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(TITAN_LINK_LIBRARIES "")
set(TITAN_SOURCES "")

add_subdirectory("src")

add_executable(${PROJECT_NAME} ${TITAN_SOURCES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PUBLIC 
        "-std=c++17 -Wall -Werror -Wno-unused-declarations -Wno-unused-variable -Wno-unused-function"
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PUBLIC 
        "/std:c++17 /W0"
    )
endif()