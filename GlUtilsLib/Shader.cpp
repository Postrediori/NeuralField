#include "stdafx.h"
#include "GlUtils.h"
#include "Shader.h"

namespace utils {

std::string loadFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);
    if (!in) {
        return "";
    }

    std::string line;
    std::ostringstream out;
    while (std::getline(in, line)) {
        out << line << std::endl;
    }
    
    in.close();

    return out.str();
}

namespace gl {

static std::string GetProgramInfo(const GLuint shader) {
    std::stringstream str;
    GLint blen;
    GLsizei slen;

    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &blen); LOGOPENGLERROR();
    if (!blen) {
        str << "no info";
    } else {
        GLchar* log = new GLchar[blen + 1];
        glGetProgramInfoLog(shader, blen, &slen, log); LOGOPENGLERROR();
        str << log;
        str << "[" << slen << " byte(s)]";
        delete[] log;
    }
    
    return str.str();
}

static std::string GetShaderError(const GLuint shader) {
    std::stringstream str;
    GLint blen;
    GLsizei slen;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &blen); LOGOPENGLERROR();
    if (!blen) {
        str << "Unknown error";
    } else {
        GLchar* compiler_log = new GLchar[blen+1];
        glGetInfoLogARB(shader, blen, &slen, compiler_log); LOGOPENGLERROR();
        str << compiler_log;
        delete[] compiler_log;
    }

    return str.str();
}

static std::string GetShaderInfo(const GLuint shader) {
    std::stringstream str;
    GLint blen;
    GLsizei slen;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &blen); LOGOPENGLERROR();
    if (!blen) {
        str << "no info";
    } else {
        GLchar* log = new GLchar[blen+1];
        glGetShaderInfoLog(shader, blen, &slen, log); LOGOPENGLERROR();
        str << log;
        str << "[" << slen << " byte(s)]";
        delete[] log;
    }
    
    return str.str();
}

}

}

ShaderProgram::ShaderProgram()
    : program_(0)
    , vertex_(0)
    , fragment_(0) {
}

ShaderProgram::~ShaderProgram() {
    release();
}

GLuint ShaderProgram::program() const {
    return program_;
}

GLint ShaderProgram::attrib(const std::string& variable) const {
    return glGetAttribLocation(program_, variable.c_str()); LOGOPENGLERROR();
}

GLint ShaderProgram::uniform(const std::string& variable) const {
    return glGetUniformLocation(program_, variable.c_str()); LOGOPENGLERROR(); 
}

bool ShaderProgram::load(const std::string& vertexShader,
                         const std::string& fragmentShader) {
    GLuint glProgram;
    GLuint vShader, fShader;

    program_ = 0;
    vertex_ = 0;
    fragment_ = 0;

    vShader = glCreateShader(GL_VERTEX_SHADER); LOGOPENGLERROR();
    fShader = glCreateShader(GL_FRAGMENT_SHADER); LOGOPENGLERROR();
    
    GLchar* vertexSource = new GLchar[vertexShader.size() + 1];
    strncpy(vertexSource, vertexShader.c_str(), vertexShader.size());
    
    GLchar* fragmentSource = new GLchar[fragmentShader.size() + 1];
    strncpy(fragmentSource, fragmentShader.c_str(), fragmentShader.size());
    
    glShaderSource(vShader, 1, &vertexSource, NULL); LOGOPENGLERROR();
    glShaderSource(fShader, 1, &fragmentSource, NULL); LOGOPENGLERROR();
    
    delete[] vertexSource;
    delete[] fragmentSource;

    GLint result;

    glCompileShader(vShader); LOGOPENGLERROR();
    glGetObjectParameterivARB(vShader, GL_COMPILE_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Vertex Shader Error : " << utils::gl::GetShaderError(vShader);
        goto error;
    }

    glCompileShader(fShader); LOGOPENGLERROR();
    glGetObjectParameterivARB(fShader, GL_COMPILE_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Fragment Shader Error : " << utils::gl::GetShaderError(fShader);
        goto error;
    }

    glProgram = glCreateProgram(); LOGOPENGLERROR();

    glAttachShader(glProgram, vShader); LOGOPENGLERROR();
    glAttachShader(glProgram, fShader); LOGOPENGLERROR();

    glLinkProgram(glProgram); LOGOPENGLERROR();

    glGetProgramiv(glProgram, GL_LINK_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Linking Shader Error : " << utils::gl::GetShaderError(glProgram);
        LOGE << "Program Info : " << utils::gl::GetShaderInfo(glProgram);
        LOGE << "Vertex Info : " << utils::gl::GetShaderInfo(vShader);
        LOGE << "Fragment Info : " << utils::gl::GetShaderInfo(fShader);
        goto error;
    }

    // show shader info
    LOGI << "Program : " << utils::gl::GetProgramInfo(glProgram);
    LOGI << "Vertex : " << utils::gl::GetShaderInfo(vShader);
    LOGI << "Fragment : " << utils::gl::GetShaderInfo(fShader);

    program_ = glProgram;
    vertex_ = vShader;
    fragment_ = fShader;

    return true;
    
error:
    if (vShader != 0) {
        glDetachShader(glProgram, vShader); LOGOPENGLERROR();
        glDeleteShader(vShader); LOGOPENGLERROR();
    }
    if (fShader != 0) {
        glDetachShader(glProgram, fShader); LOGOPENGLERROR();
        glDeleteShader(fShader); LOGOPENGLERROR();
    }
    if (glProgram != 0) {
        glDeleteProgram(glProgram); LOGOPENGLERROR();
    }
    return false;
}

void ShaderProgram::release() {
    glDetachShader(program_, vertex_); LOGOPENGLERROR();
    glDetachShader(program_, fragment_); LOGOPENGLERROR();
    glDeleteShader(vertex_); LOGOPENGLERROR();
    glDeleteShader(fragment_); LOGOPENGLERROR();
    glDeleteProgram(program_); LOGOPENGLERROR();

    program_ = 0;
    vertex_ = 0;
    fragment_ = 0;
}

ShaderFiles::ShaderFiles() {
}

bool ShaderFiles::load(const std::string& vertexFile,
                       const std::string& fragmentFile) {
    LOGI << "Loading Shader from Files: " << vertexFile << " " << fragmentFile;

    std::string strVert = utils::loadFile(vertexFile);
    if (strVert.empty()) {
        LOGE << "Vertex Shader Error : Unable to load file";
        return false;
    }

    std::string strFrag = utils::loadFile(fragmentFile);
    if (strFrag.empty()) {
        LOGE << "Fragment Shader Error : Unable to load file";
        return false;
    }

    const GLchar* sourceVertex = strVert.c_str();
    const GLchar* sourceFragment = strFrag.c_str();

    return ShaderProgram::load(sourceVertex, sourceFragment);
}
