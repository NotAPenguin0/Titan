#include "generators/grid_mesh.hpp"

#include "math.hpp"

namespace titan {

using namespace math;

GridMesh create_grid_mesh(float const width, float const height, size_t const resolution, GridMeshOptions options) {
    GridMesh mesh;

    mesh.resolution = resolution;

    float const cells_size = resolution - 1;

    // Calculate cell size
    float const cell_w = width / cells_size;
    float const cell_h = height / cells_size;

    // We will allocate the grid 1 cell larger than requested. Otherwise, we can't complete the final row/column.
    size_t const vertex_count = (resolution) * (resolution);

    // Position + TexCoord + Normal
    size_t const vertex_size = 2 + 2 + 3;
    size_t const vertices_per_quad = 6;

    // Allocate memory, make sure to zero initialize the vertices
    mesh.vertices = std::vector<float>(vertex_count * vertex_size, 0);
    mesh.indices.resize(cells_size * cells_size * vertices_per_quad);

    mesh.vertex_size = vertex_size;

    // Fill vertex buffer
    for (size_t y = 0; y < resolution; ++y) {
        for (size_t x = 0; x < resolution; ++x) {
            size_t const base_index = vertex_size * index_2d(x, y, resolution);
            // Position
            float const x_pos = options.xoffset + x * cell_w;
            float const y_pos = options.yoffset + y * cell_h;
            mesh.vertices[base_index] = x_pos;
            mesh.vertices[base_index + 1] = y_pos;
            // TexCoords
            mesh.vertices[base_index + 2] = x_pos / options.tex_w;
            mesh.vertices[base_index + 3] = y_pos / options.tex_h;
        }
    }

    // Fill index buffer

    // For each quad, fill it's indices
    for (size_t y = 0; y < cells_size; ++y) {
        for (size_t x = 0; x < cells_size; ++x) {
            size_t const base_index = vertices_per_quad * index_2d(x, y, cells_size);
            // First triangle
            mesh.indices[base_index] = index_2d(x, y, resolution);
            mesh.indices[base_index + 1] = index_2d(x, y + 1, resolution);
            mesh.indices[base_index + 2] = index_2d(x + 1, y + 1, resolution);
            // Second triangle
            mesh.indices[base_index + 3] = index_2d(x, y, resolution);
            mesh.indices[base_index + 4] = index_2d(x + 1, y + 1, resolution);
            mesh.indices[base_index + 5] = index_2d(x + 1, y, resolution);
        }
    }
    return mesh;
}

}