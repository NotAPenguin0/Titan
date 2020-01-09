add_subdirectory("glfw")

project(glad C)
add_library(glad "glad450/src/glad.c")
target_include_directories(glad PUBLIC "glad450/include")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "gcc")
    target_compile_options(glfw PUBLIC "-w")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(glfw PUBLIC "/W0")
endif()

set(TITAN_LINK_LIBRARIES
    ${TITAN_LINK_LIBRARIES}
    glfw
    glad
    PARENT_SCOPE
)

set(TITAN_INCLUDE_DIRECTORIES
    ${TITAN_INCLUDE_DIRECTORIES}
    "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/glad450/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    PARENT_SCOPE
)