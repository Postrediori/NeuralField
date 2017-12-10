#pragma once

#include "Matrix.h"
#include "Texture.h"
#include "Shader.h"

class AmariRender {
public:
    AmariRender();
    ~AmariRender();

    bool init(size_t size);
    void release();

    void render(const glm::mat4& mvp);
    void resize(unsigned int w, unsigned int h);

    void update_texture(matrix_t* m);
    
public:
    unsigned int w, h;
    size_t size;

    TextureGuard_t tex;

    bool use_blur;
    double blur_sigma;

    GLuint texture;

    GLuint vbo;

    ShaderProgram program;
    GLint aCoord, aTexCoord;
    GLint uMVP, uResolution, uTex;
};
