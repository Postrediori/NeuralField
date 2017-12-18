#pragma once

#include "Matrix.h"
#include "Texture.h"
#include "Gauss.h"
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

    void set_blur(double blur);
    void add_blur(double dblur);
    
public:
    unsigned int w, h;
    size_t size;

    TextureGuard_t tex;

    bool use_blur;
    double blur_sigma;

    KernelGuard_t blur_kernel;

    GLuint texture;

    GLuint vbo;

    ShaderProgram program;
    GLint aCoord, aTexCoord;
    GLint uMVP, uResolution, uTex;
};
