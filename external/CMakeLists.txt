add_subdirectory("glfw")


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "gcc")
    target_compile_options(glfw PUBLIC "-w")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(glfw PUBLIC "/W0")
endif()

set(TITAN_LINK_LIBRARIES
    ${TITAN_LINK_LIBRARIES}
    glfw
    PARENT_SCOPE
)

set(TITAN_INCLUDE_DIRECTORIES
    ${TITAN_INCLUDE_DIRECTORIES}
    "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include"
    PARENT_SCOPE
)