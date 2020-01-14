#include "generators/heightmap_terrain.hpp"
#include "generators/noise.hpp"

#include "math.hpp"

namespace titan {

using namespace math;

static vec3 calculate_normal(vec3 const& v1, vec3 const& v2, vec3 const& v3) {
    return cross(v1, v2) + cross(v2, v3) + cross(v3, v1);
}

static float sample_height(HeightmapTerrain const& terrain, float x, float y) {
    size_t const index = index_2d(x * (terrain.heightmap_width - 1), y * (terrain.heightmap_height - 1), terrain.heightmap_width);
    return terrain.height_map[index] / 255.0f;
}

static void calculate_normals(HeightmapTerrain& terrain) {
    // Loop over each face
    auto const& indices = terrain.mesh.indices;
    auto& vertices = terrain.mesh.vertices;
    size_t const vertex_size = terrain.mesh.vertex_size;
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
            vec3{vertices[v3_index], v3_height, vertices[v3_index + 1]}
        );

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
            vec3{vertices[base_index], vertices[base_index + 1], vertices[base_index + 2]}
        );
        vertices[base_index] = normalized.x;
        vertices[base_index + 1] = normalized.y;
        vertices[base_index + 2] = normalized.z;
    }
}

HeightmapTerrain create_heightmap_terrain(HeightmapTerrainInfo const& info) {
    HeightmapTerrain terrain;

    terrain.width = info.width;
    terrain.length = info.length;
    terrain.height_scale = info.height_scale;
    terrain.heightmap_width = info.noise_size;
    terrain.heightmap_height = info.noise_size;

    // Generate mesh
    terrain.mesh = create_grid_mesh(info.width, info.length, info.resolution, info.texture_mode);
    // Create noise buffer
    PerlinNoise noise(info.noise_seed);
    terrain.height_map = noise.get_buffer(info.noise_size, info.noise_size, info.noise_layers);

    // TODO: Make this optional (maybe not) + make it work with other texture modes (see sample_height)
    calculate_normals(terrain);

    return terrain;
}

}