#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"

SquareFlags operator|(SquareFlags lhs, SquareFlags rhs) {
    return static_cast<SquareFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

SquareFlags& operator|=(SquareFlags& lhs, SquareFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
}

SquareFlags operator^(SquareFlags lhs, SquareFlags rhs) {
    return static_cast<SquareFlags>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

SquareFlags& operator^=(SquareFlags& lhs, SquareFlags rhs) {
    lhs = lhs ^ rhs;
    return lhs;
}

SquareFlags CellType(vals_t vals) {
    return ((vals.v[0] > 0.0) ? SquareFlags::SouthWest : SquareFlags::None) |
        ((vals.v[1] > 0.0) ? SquareFlags::NorthWest : SquareFlags::None) |
        ((vals.v[2] > 0.0) ? SquareFlags::NorthEast : SquareFlags::None) |
        ((vals.v[3] > 0.0) ? SquareFlags::SouthEast : SquareFlags::None);
}

double ValuesRatio(vals_t vals, size_t i1, size_t i2) {
    assert(i1 >= 0 && i1 <= 3 && i2 >= 0 && i2 <= 3);
    return fabs(vals.v[i1] / (vals.v[i1] - vals.v[i2]));
}

ContourPlot::ContourPlot() {
}

ContourPlot::~ContourPlot() {
    this->release();
}

bool ContourPlot::init(GLuint p) {
    program = p;

    glGenVertexArrays(1, &vao); LOGOPENGLERROR();
    if (!vao) {
        LOGE << "Failed to create vertex array object";
        return false;
    }
    glBindVertexArray(vao); LOGOPENGLERROR();

    glGenBuffers(1, &vbo); LOGOPENGLERROR();
    if (!vbo) {
        LOGE << "Unable to initialize VBO for parallel contour plot";
        return false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();

    GLint a_coord = glGetAttribLocation(program, "coord"); LOGOPENGLERROR();

    glEnableVertexAttribArray(a_coord); LOGOPENGLERROR();
    glVertexAttribPointer(a_coord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glBindVertexArray(0); LOGOPENGLERROR();

    u_mvp = glGetUniformLocation(program, "mvp"); LOGOPENGLERROR();
    u_zoom = glGetUniformLocation(program, "zoom"); LOGOPENGLERROR();
    u_ofs = glGetUniformLocation(program, "ofs"); LOGOPENGLERROR();
    u_res = glGetUniformLocation(program, "res"); LOGOPENGLERROR();
    u_color = glGetUniformLocation(program, "color"); LOGOPENGLERROR();
    if (a_coord == -1 || u_mvp == -1 || u_zoom == -1 ||
        u_ofs == -1 || u_res == -1 || u_color == -1) {
        LOGE << "Shader program for contour plot doesn't contain all required uniform variables";
        return false;
    }

    return true;
}

bool ContourPlot::update(matrix_t* /*points*/, area_t /*a*/, double /*t*/) { return true; }

void ContourPlot::render(const glm::mat4& /*mvp*/, double /*zoom*/,
                         const glm::vec2& /*offset*/, const std::array<GLfloat, 4>& /*c*/) { }

void ContourPlot::release() {
    if (vbo) {
        glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
        vbo = 0;
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao); LOGOPENGLERROR();
        vao = 0;
    }
}

void ContourPlot::resize(int width, int height) {
    w = width;
    h = height;
}
