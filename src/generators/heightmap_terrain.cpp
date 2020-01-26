#include "generators/heightmap_terrain.hpp"
#include "generators/noise.hpp"

#include "math.hpp"

#include <thread>
#include <iostream>

namespace titan {

using namespace math;

static vec3 calculate_normal(vec3 const v1, vec3 const v2, vec3 const v3) {
    return cross(v1, v2) + cross(v2, v3) + cross(v3, v1);
}

static float sample_height(HeightmapTerrain const& terrain, float x, float y) {
    x = std::min(1.0f, x);
    y = std::min(1.0f, y);
    size_t const index = index_2d(x * (terrain.heightmap_width - 1), y * (terrain.heightmap_height - 1), terrain.heightmap_width);
    return terrain.height_map[index];
}

static void calculate_normals(HeightmapTerrain& terrain, GridMesh& mesh) {
    // Loop over each face
    auto const& indices = mesh.indices;
    auto& vertices = mesh.vertices;
    size_t const vertex_size = mesh.vertex_size;
    for (size_t face = 0; face < indices.size(); face += 3) {
        size_t const v1_index = vertex_size * indices[face];
        size_t const v2_index = vertex_size * indices[face + 1];
        size_t const v3_index = vertex_size * indices[face + 2];

        // Texcoords are at position 2 and 3 of a vertex
        float const v1_height = terrain.height_scale * sample_height(terrain, vertices[v1_index + 2], vertices[v1_index + 3]);
        float const v2_height = terrain.height_scale * sample_height(terrain, vertices[v2_index + 2], vertices[v2_index + 3]);
        float const v3_height = terrain.height_scale * sample_height(terrain, vertices[v3_index + 2], vertices[v3_index + 3]);

        vec3 normal = calculate_normal(
            vec3{vertices[v1_index], v1_height, vertices[v1_index + 1]},
            vec3{vertices[v2_index], v2_height, vertices[v2_index + 1]},
            vec3{vertices[v3_index], v3_height, vertices[v3_index + 1]});

        // Add calculated normals to vertex data

        auto add_normal = [&vertices, &normal](size_t const index) {
            vertices[index + 4] += normal.x;
            vertices[index + 5] += normal.y;
            vertices[index + 6] += normal.z;
        };

        add_normal(v1_index);
        add_normal(v2_index);
        add_normal(v3_index);
    }

    // Normalize all normals
    for (size_t vertex = 0; vertex < vertices.size(); vertex += vertex_size) {
        // Normal data starts at position 4
        size_t const base_index = vertex + 4;
        vec3 normalized = normalize(
            vec3{vertices[base_index], vertices[base_index + 1], vertices[base_index + 2]});
        vertices[base_index] = normalized.x;
        vertices[base_index + 1] = normalized.y;
        vertices[base_index + 2] = normalized.z;
    }
}

static void generate_chunk_lod(HeightmapTerrain& terrain, HeightmapTerrain::Chunk& chunk, HeightmapTerrainInfo const& info, 
                               size_t const lod_index, size_t const lod) {
    
    GridMeshOptions options;
    options.tex_w = terrain.width;
    options.tex_h = terrain.length;
    options.xoffset = chunk.xoffset;
    options.yoffset = chunk.yoffset;
    chunk.meshes[lod_index] = create_grid_mesh(chunk.width, chunk.length, lod, options);
    calculate_normals(terrain, chunk.meshes[lod_index]);
}

static void generate_lod(HeightmapTerrain& terrain, HeightmapTerrainInfo const& info, size_t const lod_index, size_t const lod) {
    for (size_t x = 0; x < terrain.chunks_x; ++x) {
        for (size_t y = 0; y < terrain.chunks_y; ++y) {
            size_t const chunk_id = index_2d(x, y, terrain.chunks_x);
            generate_chunk_lod(terrain, terrain.mesh.chunks[chunk_id], info, lod_index, lod);
        }
    }
}


HeightmapTerrain create_heightmap_terrain(HeightmapTerrainInfo const& info) {
    HeightmapTerrain terrain;

    terrain.width = info.width;
    terrain.length = info.length;
    terrain.height_scale = info.height_scale;
    terrain.heightmap_width = info.noise_size;
    terrain.heightmap_height = info.noise_size;

    // Create noise buffer
    PerlinNoise noise(info.noise_seed);
    terrain.height_map = noise.get_buffer_float(info.noise_size, info.noise_layers);

    size_t resolution = info.max_lod;

    constexpr size_t min_lod = 2;
    size_t const lod_count = (size_t)std::log2(info.max_lod) - min_lod;
    terrain.max_lod = lod_count;

    // Calculate the amount of chunks in the terrain

    constexpr float chunk_size = 8.0f;

    size_t const chunks_x = info.width / chunk_size + 1;
    size_t const chunks_y = info.length / chunk_size + 1;    

    size_t const chunk_count = chunks_x * chunks_y;
    terrain.mesh.chunks.resize(chunk_count);

    // Write info to output data
    terrain.chunks_x = chunks_x;
    terrain.chunks_y = chunks_y;
    terrain.chunk_size = chunk_size;

    // Write basic chunk data
    for (size_t x = 0; x < chunks_x; ++x) {
        for (size_t y = 0; y < chunks_y; ++y) {
            size_t const chunk_id = index_2d(x, y, chunks_x);
            auto& chunk = terrain.mesh.chunks[chunk_id];
            chunk.xoffset = x * terrain.chunk_size;
            chunk.yoffset = y * terrain.chunk_size;
            chunk.width = terrain.chunk_size;
            chunk.length = terrain.chunk_size;

            if (x == terrain.chunks_x - 1) {
                chunk.width = std::fmod(terrain.width, terrain.chunk_size);
            }
            if (y == terrain.chunks_y - 1) {
                chunk.length = std::fmod(terrain.length, terrain.chunk_size);
            }

            chunk.meshes.resize(terrain.max_lod);

            chunk.height_at_center = terrain.height_scale * 
                                     sample_height(terrain, chunk.xoffset + chunk.width / 2.0f, chunk.yoffset + chunk.length / 2.0f);
        }
    }

    std::vector<std::thread> threads(lod_count);

    for (size_t lod_index = 0; lod_index < lod_count; ++lod_index) {
        threads[lod_index] = std::thread(
            generate_lod,
            std::ref(terrain), std::ref(info), lod_index, resolution
        );
        resolution /= 2;
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return terrain;
}

} // namespace titan