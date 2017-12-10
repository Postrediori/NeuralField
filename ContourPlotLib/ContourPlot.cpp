#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"

flags_t CellType(double vals[]) {
    flags_t flags = FLAG_NO;
    if (vals[0] > 0.0) {
        flags |= FLAG_SW;
    }
    if (vals[1] > 0.0) {
        flags |= FLAG_NW;
    }
    if (vals[2] > 0.0) {
        flags |= FLAG_NE;
    }
    if (vals[3] > 0.0) {
        flags |= FLAG_SE;
    }
    return flags;
}

ContourPlot::ContourPlot(GLuint p)
    : w(0)
    , h(0)
    , vbo_count(0)
    , vbo(0)
    , threshold(0.0)
    , program(p) {
    a_coord = glGetAttribLocation(program, "coord"); LOGOPENGLERROR();
    u_mvp = glGetUniformLocation(program, "mvp"); LOGOPENGLERROR();
    u_zoom = glGetUniformLocation(program, "zoom"); LOGOPENGLERROR();
    u_ofs = glGetUniformLocation(program, "ofs"); LOGOPENGLERROR();
    u_res = glGetUniformLocation(program, "res"); LOGOPENGLERROR();
    u_color = glGetUniformLocation(program, "color"); LOGOPENGLERROR();
}

ContourPlot::~ContourPlot() {
    this->release();
}

bool ContourPlot::init(matrix_t* points, area_t a) { return init(points, a, 0.0); }

bool ContourPlot::init(matrix_t* points, area_t a, double t) { return true; }

void ContourPlot::render(const glm::mat4& mvp, double zoom,
                         const glm::vec2& offset, const GLfloat c[]) { }

void ContourPlot::release() {
    if (vbo) {
        glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
        vbo = 0;
    }
}

void ContourPlot::resize(int width, int height) {
    w = width;
    h = height;
}
