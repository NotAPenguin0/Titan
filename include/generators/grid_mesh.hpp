#ifndef TITAN_GRID_MESH_HPP_
#define TITAN_GRID_MESH_HPP_

#include <vector>

// TODO:
/*
    User-provided buffer/allocation function
*/

namespace titan {

namespace generators {

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
 * @return The generated mesh
 */
GridMesh generate_grid_mesh(float width, float height, size_t resolution);


}

}

#endif