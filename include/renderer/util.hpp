#ifndef TITAN_RENDERER_UTIL_HPP_
#define TITAN_RENDERER_UTIL_HPP_

namespace titan {

namespace renderer {

void set_wireframe(bool wireframe);

unsigned int load_shader(const char* vtx_path, const char* frag_path, const char* geom_path = nullptr);

unsigned int load_texture(const char* path);

unsigned int texture_from_buffer(unsigned char* buf, size_t w, size_t h);

}

}

#endif