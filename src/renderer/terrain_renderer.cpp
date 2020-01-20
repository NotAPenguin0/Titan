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

static void fill_vbo(TerrainRenderInfo::ChunkRenderInfo& info, HeightmapTerrain::Chunk const& chunk, size_t lod) {
    auto const& mesh = chunk.meshes[lod];
    glBindBuffer(GL_ARRAY_BUFFER, info.vbos[lod]);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.ebos[lod]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);
    info.element_count[lod] = mesh.indices.size();
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
        info.chunks[i].vbos.resize(lod_count);
        info.chunks[i].ebos.resize(lod_count);
        info.chunks[i].element_count.resize(lod_count);
        glGenBuffers(lod_count, info.chunks[i].vbos.data());
        glGenBuffers(lod_count, info.chunks[i].ebos.data());
        for (size_t lod = 0; lod < lod_count; ++lod) {
            fill_vbo(info.chunks[i], terrain.mesh.chunks[i], lod);
        }

        auto const& mesh = terrain.mesh.chunks[i].meshes[0];
        info.chunks[i].vbo.create(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float));
        info.chunks[i].ebo.create(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int));
        info.chunks[i].elements = mesh.indices.size();

        info.chunks[i].vbo.start_data_upload(mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
        info.chunks[i].ebo.start_data_upload(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));
    }

    // Misc data

    // vertex_size stays constant
    info.vertex_size = terrain.mesh.chunks[0].meshes[0].vertex_size;

    return info;
}

void await_data_upload(TerrainRenderInfo& terrain) {
    for (auto& chunk : terrain.chunks) {
        chunk.vbo.wait_for_upload();
        chunk.ebo.wait_for_upload();
    }
}
    
void render_terrain(TerrainRenderInfo const& terrain, size_t const lod) {
    // Bind noisemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain.height_map);
    // Render all chunks
    size_t const chunk_count = terrain.chunks.size();
    for (size_t i = 0; i < chunk_count; ++i) {
        auto const& chunk = terrain.chunks[i];
        unsigned int vbo = chunk.vbos[lod];
        unsigned int ebo = chunk.ebos[lod];

        vbo = chunk.vbo.get();
        ebo = chunk.ebo.get();

        // Update buffers for VAO
        glBindVertexBuffer(0, vbo, 0, terrain.vertex_size * sizeof(float));
        glBindVertexBuffer(1, vbo, 0, terrain.vertex_size * sizeof(float));
        glBindVertexBuffer(2, vbo, 0, terrain.vertex_size * sizeof(float));
        glVertexArrayElementBuffer(terrain.vao, ebo);
//        glDrawElements(GL_TRIANGLES, chunk.element_count[lod], GL_UNSIGNED_INT, nullptr);
        glDrawElements(GL_TRIANGLES, chunk.elements, GL_UNSIGNED_INT, nullptr);
    }
}

}