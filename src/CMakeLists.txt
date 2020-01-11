set(TITAN_SOURCES
    ${TITAN_SOURCES}
    # Misc
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cinematic_camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/example_app.cpp"

    # Terrain renderer
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/util.cpp"

    # Generators module
    "${CMAKE_CURRENT_SOURCE_DIR}/generators/grid_mesh.cpp"

    # stb_image
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/stb_image.cpp"
    PARENT_SCOPE
)