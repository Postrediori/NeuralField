#pragma once

#include "GlUtils.h"

class ShaderProgram {
public:
    ShaderProgram();
    virtual ~ShaderProgram();

    GLuint program() const;
    GLint attrib(const std::string& variable) const;
    GLint uniform(const std::string& variable) const;
    
    virtual bool load(const std::string& vertexShader,
                      const std::string& fragmentShader);

private:
    void release();

private:
    GLuint program_;
    GLuint vertex_;
    GLuint fragment_;
};

class ShaderFiles : public ShaderProgram {
public:
    ShaderFiles();
    bool load(const std::string& vertexFile,
              const std::string& fragmentFile);
};
