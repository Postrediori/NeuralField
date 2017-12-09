#pragma once

#include "GlUtils.h"

class ShaderProgram {
public:
    ShaderProgram();

    void release();

    GLint attrib(const GLchar* name);
    GLint uniform(const GLchar* name);

    bool load_file(const char* vertex_shader,
                  const char* fragment_shader);
    bool load_source(const char* vertex_src,
                    const char* fragment_src);

public:
    GLuint glProgram;
    GLuint glShaderV;
    GLuint glShaderF;
};
