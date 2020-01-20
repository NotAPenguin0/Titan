#include "renderer/terrain_renderer.hpp"
#include "renderer/util.hpp"

#include <glad/glad.h>

// debug
#include <iostream>

namespace titan::renderer {

static void create_vao(TerrainRenderInfo& info, HeightmapTerrain const& terrain) {
    glGenVertexArrays(1, &info.vao);
    glBindVertexArray(info.vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);

    // TexCoords
    glEnableVertexAttribArray(1);
    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
    glVertexAttribBinding(1, 1);

    // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float));
    glVertexAttribBinding(2, 2);
}

static void create_heightmap(TerrainRenderInfo& info, HeightmapTerrain const& terrain) {
    info.height_map = texture_from_buffer(terrain.height_map.data(), terrain.heightmap_width, terrain.heightmap_height);
}


TerrainRenderInfo make_terrain_render_info(HeightmapTerrain const& terrain) {
    TerrainRenderInfo info;

    create_vao(info, terrain);
    create_heightmap(info, terrain);

    // Fill chunk vbo's
    size_t const chunk_count = terrain.mesh.chunks.size();
    size_t const lod_count = terrain.max_lod;
    info.chunks.resize(chunk_count);
    for (size_t i = 0; i < chunk_count; ++i) {
        
    }

    // Misc data

    // vertex_size stays constant
    info.vertex_size = terrain.mesh.chunks[0].meshes[0].vertex_size;

    return info;
}


void render_terrain(TerrainRenderInfo const& terrain, size_t const lod) {
    // Bind noisemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain.height_map);
    // Render all chunks
    size_t const chunk_count = terrain.chunks.size();
    for (size_t i = 0; i < chunk_count; ++i) {
        auto const& chunk = terrain.chunks[i];
        auto const& buf = chunk.current_lod;
        unsigned int vbo = buf.vbo.get();
        unsigned int ebo = buf.ebo.get();
        // Update buffers for VAO
        glBindVertexBuffer(0, vbo, 0, terrain.vertex_size * sizeof(float));
        glBindVertexBuffer(1, vbo, 0, terrain.vertex_size * sizeof(float));
        glBindVertexBuffer(2, vbo, 0, terrain.vertex_size * sizeof(float));
        glVertexArrayElementBuffer(terrain.vao, ebo);
        glDrawElements(GL_TRIANGLES, buf.elements, GL_UNSIGNED_INT, nullptr);
    }
}

}