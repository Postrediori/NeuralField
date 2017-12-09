#pragma once

#include "Shader.h"

class AmariRender {
public:
    AmariRender();
    ~AmariRender();

    bool init(size_t size);
    void release();

    void render(const glm::mat4& mvp);
    void resize(unsigned int w, unsigned int h);

    void update_texture(const float data[], const size_t size);
    
public:
    unsigned int w, h;

    size_t tex_size;
    size_t tex_data_size;
    GLubyte * tex_data;

    bool use_blur;
    float blur_sigma;

    GLuint texture;

    GLuint vbo;

    ShaderProgram program;
    GLint aCoord, aTexCoord;
    GLint uMVP, uResolution, uTex;
};
