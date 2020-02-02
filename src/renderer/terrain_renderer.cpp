#include "renderer/terrain_renderer.hpp"
#include "renderer/util.hpp"

#include <glad/glad.h>

// debug
#include <iostream>

#include "math.hpp"
// meh
#include <glm/glm.hpp> 

using namespace std::chrono;

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

// Create swap buffers for max LOD specified in lod parameter
static void make_swap_buffers_lod(HeightmapTerrain const& terrain, TerrainRenderInfo::LODBuffer& buffer, 
                                  size_t chunk_id, size_t lod) {
    HeightmapTerrain::Chunk const& chunk = terrain.mesh.chunks[chunk_id];
    // Create VBO swap buffer
    buffer.vbo.create(GL_ARRAY_BUFFER, chunk.meshes[lod].vertices.size() * sizeof(float));
    // Create EBO swap buffer
    buffer.ebo.create(GL_ELEMENT_ARRAY_BUFFER, chunk.meshes[lod].indices.size() * sizeof(unsigned int));
    // Finalize
    buffer.elements = chunk.meshes[lod].indices.size();
}

static void queue_swap_buffer_fill(HeightmapTerrain const& terrain, TerrainRenderInfo::LODBuffer& buffer,
                                   size_t chunk_id, size_t lod) {
    HeightmapTerrain::Chunk const& chunk = terrain.mesh.chunks[chunk_id];
    GridMesh const& mesh = chunk.meshes[lod];
    buffer.vbo.start_data_upload(mesh.vertices.data(), mesh.vertices.size() * sizeof(float));
    buffer.ebo.start_data_upload(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));
    buffer.elements = mesh.indices.size();
    buffer.lod = lod;
}

TerrainRenderInfo make_terrain_render_info(HeightmapTerrain const& terrain, size_t const initial_lod) {
    TerrainRenderInfo info;

    create_vao(info, terrain);
    create_heightmap(info, terrain);

    // Fill chunk vbo's
    size_t const chunk_count = terrain.mesh.chunks.size();
    size_t const lod_count = terrain.max_lod;
    info.chunks.resize(chunk_count);
    for (size_t i = 0; i < chunk_count; ++i) {
        auto& chunk = info.chunks[i];
        auto const& chunk_data = terrain.mesh.chunks[i];
        // Chunk center for distanced based LOD changing
        chunk.center[0] = chunk_data.xoffset + chunk_data.width / 2.0f;
        chunk.center[1] = chunk_data.yoffset + chunk_data.length / 2.0f;
        chunk.center[2] = chunk_data.height_at_center;
        // Create swap buffers with highest LOD level, this is at index 0
        make_swap_buffers_lod(terrain, chunk.higher_lod, i, 0);
        make_swap_buffers_lod(terrain, chunk.current_lod, i, 0);
        make_swap_buffers_lod(terrain, chunk.lower_lod, i, 0);
        // Queue filling the swap buffers for this chunk
        queue_swap_buffer_fill(terrain, chunk.higher_lod, i, initial_lod - 1);
        queue_swap_buffer_fill(terrain, chunk.current_lod, i, initial_lod);
        queue_swap_buffer_fill(terrain, chunk.lower_lod, i, initial_lod + 1);
    }

    // vertex layout stays constant, so we can pick any LOD on any chunk
    info.vertex_size = terrain.mesh.chunks[0].meshes[0].vertex_size;

    return info;
}

void swap_buffers(TerrainRenderInfo::LODBuffer& lhs, TerrainRenderInfo::LODBuffer& rhs) {
    lhs.vbo.swap(rhs.vbo);
    lhs.ebo.swap(rhs.ebo);
    std::swap(lhs.elements, rhs.elements);
    std::swap(lhs.lod, rhs.lod);
}

void higher_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id) {
    // Make sure all data was uploaded before starting to swap buffers
    auto& chunk = info.chunks[chunk_id];
    await_all_data_upload(chunk);

    size_t const current_lod = chunk.current_lod.lod;
   
    // Previous LOD becomes current LOD
    swap_buffers(chunk.higher_lod, chunk.current_lod);
    // The old current LOD becomes the next LOD
    swap_buffers(chunk.higher_lod, chunk.lower_lod);
    // The old next LOD is now unused, fill it with the new previous LOD
    size_t new_previous_lod = chunk.current_lod.lod - 1;
    // Don't load LOD out of bounds
    if (chunk.current_lod.lod == 0) { return; }
    queue_swap_buffer_fill(terrain, chunk.higher_lod, chunk_id, new_previous_lod);
}

void lower_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id) {
    // Make sure all data was uploaded before starting to swap buffers
    auto& chunk = info.chunks[chunk_id];
    await_all_data_upload(chunk);

    size_t const current_lod = chunk.current_lod.lod;

    // Next LOD becomes current LOD
    swap_buffers(chunk.lower_lod, chunk.current_lod);
    // The old current LOD becomes the new previous LOD
    swap_buffers(chunk.lower_lod, chunk.higher_lod);

    // The old previous LOD is now unused, fill it with the new next LOD
    size_t const new_next_lod = chunk.current_lod.lod + 1;
    // Don't load LOD out of bounds
    if (chunk.current_lod.lod >= terrain.max_lod - 1) { return; }
    queue_swap_buffer_fill(terrain, chunk.lower_lod, chunk_id, new_next_lod);
}

void update_lod_distance(TerrainRenderInfo& info, HeightmapTerrain const& terrain, glm::mat4 terrain_transform, float const* cam_pos) {
    for (size_t chunk_id = 0; chunk_id < info.chunks.size(); ++chunk_id) {
        auto const& center_raw = info.chunks[chunk_id].center;
        glm::vec4 center = glm::vec4(center_raw[0], center_raw[1], center_raw[2], 1);
        // Transform center with model matrix
        center = terrain_transform * center;
        update_lod_distance(info, terrain, chunk_id, &center.x, cam_pos);
    }
}

void update_lod_distance(TerrainRenderInfo& info, HeightmapTerrain const& terrain, 
                         size_t chunk_id, float const* chunk_center, float const* cam_pos) {
    auto const& chunk = info.chunks[chunk_id];
    math::vec3 cam = {cam_pos[0], cam_pos[1], cam_pos[2]};
    math::vec3 center = {chunk_center[0], chunk_center[1], chunk_center[2]};

    float distance = math::magnitude(center - cam);  
    // Very basic distance-based LOD
    // Treshold for maximum LOD
    constexpr float max_lod_distance = 5.0f;
    // Treshold for minimum LOD
    constexpr float min_lod_distance = 200.0f;
    constexpr float distance_range = min_lod_distance - max_lod_distance;
    distance -= max_lod_distance;
    float distance_pct = distance / distance_range;
    distance_pct = std::clamp(distance_pct, 0.0f, 1.0f);
    distance_pct *= terrain.max_lod;
    if ((size_t)distance_pct > (chunk.current_lod.lod)) {
        if (chunk.current_lod.lod >= terrain.max_lod - 1) { return; }
        lower_lod(info, terrain, chunk_id);
    } else if ((size_t)distance_pct < (chunk.current_lod.lod)) {
        if (chunk.current_lod.lod ==  0) { return; }
        higher_lod(info, terrain, chunk_id);
    }
}

void await_all_data_upload(TerrainRenderInfo::ChunkRenderInfo& chunk) {
    chunk.current_lod.vbo.wait_for_upload();
    chunk.current_lod.ebo.wait_for_upload();

    chunk.higher_lod.vbo.wait_for_upload();
    chunk.higher_lod.ebo.wait_for_upload();
    
    chunk.lower_lod.vbo.wait_for_upload();
    chunk.lower_lod.ebo.wait_for_upload();
}

void render_terrain(TerrainRenderInfo const& terrain) {
    // Bind noisemap
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrain.height_map);
    glBindVertexArray(terrain.vao);
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