#include "generators/grid_mesh.hpp"

namespace titan {

static size_t index_2d(size_t const x, size_t const y, size_t const w) {
    return y * w + x;
}

GridMesh create_grid_mesh(float const width, float const height, size_t const resolution, TextureMode tex_mode) {
    GridMesh mesh;

    // Calculate cell size
    float const cell_w = width / resolution;
    float const cell_h = height / resolution;

    // We will allocate the grid 1 cell larger than requested. Otherwise, we can't complete the final row/column.
    size_t const vertex_count = (resolution + 1) * (resolution + 1);
    // A vertex consists of one vec2 for the position and one vec2 for the texture coordinates, which
    // makes 4 elements
    size_t const vertex_size = 4;
    size_t const vertices_per_quad = 6;

    // Allocate memory
    mesh.vertices.resize(vertex_count * vertex_size);
    mesh.indices.resize(resolution * resolution * vertices_per_quad);

    // Fill vertex buffer
    for (size_t y = 0; y < resolution + 1; ++y) {
        for (size_t x = 0; x < resolution + 1; ++x) {
            size_t const base_index = vertex_size * index_2d(x, y, resolution + 1);
            // Position
            float const x_pos = x * cell_w;
            float const y_pos = y * cell_h;
            mesh.vertices[base_index] = x_pos;
            mesh.vertices[base_index + 1] = y_pos;
            // TexCoords
            mesh.vertices[base_index + 2] = x_pos / (tex_mode == TextureMode::Stretch ? width : 1);
            mesh.vertices[base_index + 3] = y_pos / (tex_mode == TextureMode::Stretch ? height : 1);
        }
    }

    // Fill index buffer

    // For each quad, fill it's indices
    for (size_t y = 0; y < resolution; ++y) {
        for (size_t x = 0; x < resolution; ++x) {
            size_t const base_index = vertices_per_quad * index_2d(x, y, resolution);
            // First triangle
            mesh.indices[base_index] = index_2d(x, y, resolution + 1);
            mesh.indices[base_index + 1] = index_2d(x, y + 1, resolution + 1);
            mesh.indices[base_index + 2] = index_2d(x + 1, y + 1, resolution + 1);
            // Second triangle
            mesh.indices[base_index + 3] = index_2d(x, y, resolution + 1);
            mesh.indices[base_index + 4] = index_2d(x + 1, y, resolution + 1);
            mesh.indices[base_index + 5] = index_2d(x + 1, y + 1, resolution + 1);
        }
    }

    return mesh;
}

}