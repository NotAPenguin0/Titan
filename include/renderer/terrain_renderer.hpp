#ifndef TITAN_RENDERER_TERRAIN_RENDERER_HPP_
#define TITAN_RENDERER_TERRAIN_RENDERER_HPP_

#include "generators/heightmap_terrain.hpp"

#include "renderer/swap_buffer.hpp"

#include <vector>
#include <glm/glm.hpp>

namespace titan::renderer {

struct TerrainRenderInfo {
    // We can use one single vao because the vertex format is consistent for each chunk
    unsigned int vao;


    struct LODBuffer {
        SwapBuffer vbo;
        SwapBuffer ebo;
        size_t elements;

        size_t lod;
    };

    struct ChunkRenderInfo {
        // Previous LOD means higher detail, next LOD means lower detail
        LODBuffer current_lod;
        LODBuffer higher_lod;
        LODBuffer lower_lod;  

        float center[3] = {0, 0, 0};
    };

    std::vector<ChunkRenderInfo> chunks;

    // Heightmap texture
    unsigned int height_map;

    // Misc data
    size_t vertex_size;
};

TerrainRenderInfo make_terrain_render_info(HeightmapTerrain const& terrain, size_t const initial_lod);

void higher_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id);
void lower_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id);

// TODO: I don't like having glm::mat4 here but okay
void update_lod_distance(TerrainRenderInfo& info, HeightmapTerrain const& terrain, glm::mat4 terrain_transform, float const* cam_pos);

/**
 * @param chunk_center: Pointer to a float array with 3 values with the chunk's center
 * @param cam_pos: Pointer to a float array with 3 values with the camera position 
 */
void update_lod_distance(TerrainRenderInfo& info, HeightmapTerrain const& terrain, 
                         size_t chunk_id, float const* chunk_center, float const* cam_pos);

void await_all_data_upload(TerrainRenderInfo::ChunkRenderInfo& chunk);

// Before calling this, a shader must be bound
void render_terrain(TerrainRenderInfo const& terrain);
    
}

#endif