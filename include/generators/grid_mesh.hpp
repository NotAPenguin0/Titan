#ifndef TITAN_GRID_MESH_HPP_
#define TITAN_GRID_MESH_HPP_

#include <vector>

#include "config.hpp"

// TODO:
/*
    User-provided buffer/allocation function
*/

namespace titan {

/* GridMesh vertex layout:
Vertex 0                Vertex 1
Position    TexCoords   Position    TexCoords    
x   y       x   y       x   y       x   y

The GridMesh is indexed since otherwise we have a lot of duplicate vertices
*/
struct GridMesh {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

/**
 * @param width: The width of the mesh in worldspace units
 * @param height: The height of the mesh in worldspace units
 * @param resolution: The amount of grid cells in each row/column
 * @param tex_mode: Whether to strech the texture so it spans the entire grid or to repeat it every worldspace unit. Default value is Stretch
 * @return The generated mesh
 */
GridMesh create_grid_mesh(float width, float height, size_t resolution, TextureMode tex_mode = TextureMode::Stretch);


}

#endif