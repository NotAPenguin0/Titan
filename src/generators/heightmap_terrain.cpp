#include "generators/heightmap_terrain.hpp"
#include "generators/noise.hpp"

namespace titan {

HeightmapTerrain create_heightmap_terrain(HeightmapTerrainInfo const& info) {
    HeightmapTerrain terrain;

    // Generate mesh
    terrain.mesh = create_grid_mesh(info.width, info.height, info.resolution, info.texture_mode);
    // Create noise buffer
    PerlinNoise noise(0);
    terrain.height_map = noise.get_buffer(info.noise_size, info.noise_size, info.noise_layers);

    return terrain;
}

}