#include "stdafx.h"
#include "GlUtils.h"
#include "Shader.h"

static std::string loadFile(const char* filename) {
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

ShaderProgram::ShaderProgram()
    : glProgram(0)
    , glShaderV(0)
    , glShaderF(0) {
}

bool ShaderProgram::load_file(const char* vertex_shader, const char* fragment_shader) {
    LOGI << "Loading Shader from Files: " << vertex_shader << " " << fragment_shader;

    std::string strVert = loadFile(vertex_shader);
    if (strVert.empty()) {
        LOGE << "Vertex Shader Error : Unable to load file";
        return false;
    }

    std::string strFrag = loadFile(fragment_shader);
    if (strFrag.empty()) {
        LOGE << "Fragment Shader Error : Unable to load file";
        return false;
    }

    const GLchar* sourceVertex = strVert.c_str();
    const GLchar* sourceFragment = strFrag.c_str();

    return load_source(sourceVertex, sourceFragment);
}

bool ShaderProgram::load_source(const char* vertex_shader, const char* fragment_shader) {
    GLuint program;
    GLuint vShader, fShader;

    glProgram = 0;
    glShaderV = 0;
    glShaderF = 0;

    vShader = glCreateShader(GL_VERTEX_SHADER); LOGOPENGLERROR();
    fShader = glCreateShader(GL_FRAGMENT_SHADER); LOGOPENGLERROR();
    glShaderSource(vShader, 1, &vertex_shader, NULL); LOGOPENGLERROR();
    glShaderSource(fShader, 1, &fragment_shader, NULL); LOGOPENGLERROR();

    GLint result;

    glCompileShader(vShader); LOGOPENGLERROR();
    glGetObjectParameterivARB(vShader, GL_COMPILE_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Vertex Shader Error : " << GetShaderError(vShader);
        goto error;
    }

    glCompileShader(fShader); LOGOPENGLERROR();
    glGetObjectParameterivARB(fShader, GL_COMPILE_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Fragment Shader Error : " << GetShaderError(fShader);
        goto error;
    }

    program = glCreateProgram(); LOGOPENGLERROR();

    glAttachShader(program, vShader); LOGOPENGLERROR();
    glAttachShader(program, fShader); LOGOPENGLERROR();

    glLinkProgram(program); LOGOPENGLERROR();

    glGetProgramiv(program, GL_LINK_STATUS, &result); LOGOPENGLERROR();
    if (!result) {
        LOGE << "Linking Shader Error : " << GetShaderError(program);
        LOGE << "Program Info :" << GetShaderInfo(program);
        LOGE << "Vertex Info :" << GetShaderInfo(vShader);
        LOGE << "Fragment Info :" << GetShaderInfo(fShader);
        goto error;
    }

    // show shader info
    LOGI << "Program :" << GetProgramInfo(program);
    LOGI << "Vertex :" << GetShaderInfo(vShader);
    LOGI << "Fragment :" << GetShaderInfo(fShader);

    glProgram = program;
    glShaderV = vShader;
    glShaderF = fShader;

    return true;
    
error:
    if (vShader != 0) {
        glDetachShader(program, vShader); LOGOPENGLERROR();
        glDeleteShader(vShader); LOGOPENGLERROR();
    }
    if (fShader != 0) {
        glDetachShader(program, fShader); LOGOPENGLERROR();
        glDeleteShader(fShader); LOGOPENGLERROR();
    }
    if (program != 0) {
        glDeleteProgram(program); LOGOPENGLERROR();
    }
    return false;
}

void ShaderProgram::release() {
    glDetachShader(glProgram, glShaderF); LOGOPENGLERROR();
    glDetachShader(glProgram, glShaderV); LOGOPENGLERROR();
    glDeleteShader(glShaderF); LOGOPENGLERROR();
    glDeleteShader(glShaderV); LOGOPENGLERROR();
    glDeleteProgram(glProgram); LOGOPENGLERROR();

    glProgram = 0;
    glShaderV = 0;
    glShaderF = 0;
}

GLint ShaderProgram::attrib(const GLchar* name) {
    return glGetAttribLocation(glProgram, name); LOGOPENGLERROR();
}

GLint ShaderProgram::uniform(const GLchar* name) {
    return glGetUniformLocation(glProgram, name); LOGOPENGLERROR(); 
}
