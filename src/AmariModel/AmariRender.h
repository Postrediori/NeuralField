#pragma once

class AmariRender {
public:
    AmariRender();
    ~AmariRender();

    bool init(size_t size);
    void release();

    void render(const Math::mat4f& mvp);
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

    GLuint vao;
    GLuint vbo;

    GLuint program;
    GLint uMVP, uResolution, uTex;
};
