#ifndef TITAN_RENDERER_UTIL_HPP_
#define TITAN_RENDERER_UTIL_HPP_

namespace titan {

namespace renderer {

unsigned int load_shader(const char* vtx_path, const char* frag_path);

unsigned int load_texture(const char* path);

}

}

#endif