set(TITAN_SOURCES
    ${TITAN_SOURCES}
    # Misc
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/cinematic_camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/example_app.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/math.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/input.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp"

    # Terrain renderer
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/util.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/chunk_renderer.cpp"

    # Generators module
    "${CMAKE_CURRENT_SOURCE_DIR}/generators/grid_mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generators/noise.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/generators/heightmap_terrain.cpp"

    # stb_image
    "${CMAKE_CURRENT_SOURCE_DIR}/renderer/stb_image.cpp"
    PARENT_SCOPE
)