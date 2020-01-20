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
        LODBuffer previous_lod;
        LODBuffer next_lod;  
    };

    std::vector<ChunkRenderInfo> chunks;

    // Heightmap texture
    unsigned int height_map;

    // Misc data
    size_t vertex_size;
};

TerrainRenderInfo make_terrain_render_info(HeightmapTerrain const& terrain, size_t const initial_lod);

void previous_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id);
void next_lod(TerrainRenderInfo& info, HeightmapTerrain const& terrain, size_t chunk_id);

void await_data_upload(TerrainRenderInfo::ChunkRenderInfo& chunk);

// Before calling this, a shader must be bound
void render_terrain(TerrainRenderInfo const& terrain);
    
}

#endif