cmake_minimum_required(VERSION 3.12)

project(Titan)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)

set(TITAN_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(TITAN_LINK_LIBRARIES "")
set(TITAN_SOURCES "")

add_subdirectory("src")
add_subdirectory("external")


add_executable(${PROJECT_NAME} ${TITAN_SOURCES})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "gcc")
    target_compile_options(${PROJECT_NAME} PUBLIC 
        "-std=c++2a" "-Wall" "-Werror" "-Wno-unused-declarations" "-Wno-unused-variable" "-Wno-unused-function -ffast-math"
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PUBLIC 
        "/std:c++20 /W0 /fp:fast /O2"
    )
    target_link_options(${PROJECT_NAME} PUBLIC
        "/PROFILE"
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${TITAN_INCLUDE_DIRECTORIES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${TITAN_LINK_LIBRARIES})