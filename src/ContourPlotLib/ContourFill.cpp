#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourFill.h"

void MakeFill(triangles_t& triangles, discrete_t d);
void MakeFillCorner(triangles_t& triangles, flags_t flags, discrete_t d, vals_t vals);
void MakeFillHalf(triangles_t& triangles, flags_t flags, discrete_t d, vals_t vals);
void MakeFillAmbiguity(triangles_t& triangles, flags_t flags, bool u, discrete_t d, vals_t vals, double v);

ContourFill::ContourFill()
    : ContourPlot() {
}

bool ContourFill::update(matrix_t* points, area_t a, double t) {
    threshold = t;

    area = a;
    
    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    flags_t flags;
    bool u;
    vals_t vals;
    double v;

    triangles_t triangles;

    for (int j=0; j<ydiv; j++) {
        double y = a.ymin + j * dY;
        for (int i=0; i<xdiv; i++) {
            double x = a.xmin + i * dX;
            discrete_t d = {x, y, dX, dY};
            vals.v[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
            vals.v[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals.v[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals.v[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;
            flags = CellType(vals);

            switch (flags) {
            case FLAG_SW:
            case FLAG_NW:
            case FLAG_NE:
            case FLAG_SE:
            case (FLAG_ALL ^ FLAG_SW):
            case (FLAG_ALL ^ FLAG_NW):
            case (FLAG_ALL ^ FLAG_NE):
            case (FLAG_ALL ^ FLAG_SE):
                // One corner
                MakeFillCorner(triangles, flags, d, vals);
                break;

            case (FLAG_SW | FLAG_NW):
            case (FLAG_NW | FLAG_NE):
            case (FLAG_NE | FLAG_SE):
            case (FLAG_SE | FLAG_SW):
                // Half
                MakeFillHalf(triangles, flags, d, vals);
                break;

            case (FLAG_SW | FLAG_NE):
            case (FLAG_NW | FLAG_SE):
                // Ambiguity
                v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                u = v > 0.0;
                MakeFillAmbiguity(triangles, flags, u, d, vals, v);
                break;

            case FLAG_ALL:
                // Full fill
                MakeFill(triangles, d);
                break;

            case FLAG_NO:
                // No lines
                break;

            default:
                break;
            }
        }
    }

    vbo_count = triangles.size();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
	glBufferData(GL_ARRAY_BUFFER, sizeof(Math::vec2f) * triangles.size(),
        triangles.data(), GL_DYNAMIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created outline contour with " << triangles.size() / 3 << " triangles";
    
    return true;
}

void ContourFill::render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
                         const GLfloat c[]) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform2f(u_res, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c); LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}

void MakeFill(triangles_t& triangles, discrete_t d) {
    triangles.emplace_back(d.x, d.y);
    triangles.emplace_back(d.x + d.sx, d.y + d.sy);
    triangles.emplace_back(d.x, d.y + d.sy);

    triangles.emplace_back(d.x, d.y);
    triangles.emplace_back(d.x + d.sx, d.y);
    triangles.emplace_back(d.x + d.sx, d.y + d.sy);
}

void MakeFillCorner(triangles_t& triangles, flags_t flags,
                    discrete_t d, vals_t vals) {
    double x1(0.0), y1(0.0);
    double x2(0.0), y2(0.0);

    switch (flags) {
    case FLAG_SW:
    case (FLAG_ALL ^ FLAG_SW):
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x;
        y2 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        break;

    case FLAG_NW:
    case (FLAG_ALL ^ FLAG_NW):
        x1 = d.x+d.sx;
        y1 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;
        break;

    case FLAG_NE:
    case (FLAG_ALL ^ FLAG_NE):
        x1 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y1 = d.y+d.sy;
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        break;

    case FLAG_SE:
    case (FLAG_ALL ^ FLAG_SE):
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y2 = d.y+d.sy;
        break;
    }
    
    Math::vec2f v(d.x, d.y), v1(x1, y1), v2(x2, y2);
    Math::vec2f sx(d.sx, 0.0f), sy(0.0f, d.sy), s(d.sx, d.sy);

    if (flags == FLAG_SW) {
        triangles.push_back(v);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags == (FLAG_ALL ^ FLAG_SW)) {
        triangles.push_back(v1);
        triangles.push_back(v + sx);
        triangles.push_back(v + s);

        triangles.push_back(v1);
        triangles.push_back(v + s);
        triangles.push_back(v2);

        triangles.push_back(v2);
        triangles.push_back(v + s);
        triangles.push_back(v + sy);

    } else if (flags == FLAG_NW) {
        triangles.push_back(v + sx);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags == (FLAG_ALL ^ FLAG_NW)) {
        triangles.push_back(v);
        triangles.push_back(v2);
        triangles.push_back(v + sy);

        triangles.push_back(v + sy);
        triangles.push_back(v2);
        triangles.push_back(v1);

        triangles.push_back(v + sy);
        triangles.push_back(v1);
        triangles.push_back(v + s);

    } else if (flags == FLAG_NE) {
        triangles.push_back(v + s);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags == (FLAG_ALL ^ FLAG_NE)) {
        triangles.push_back(v);
        triangles.push_back(v + sx);
        triangles.push_back(v2);

        triangles.push_back(v);
        triangles.push_back(v2);
        triangles.push_back(v1);

        triangles.push_back(v);
        triangles.push_back(v1);
        triangles.push_back(v + sy);

    } else if (flags == FLAG_SE) {
        triangles.push_back(v + sy);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags == (FLAG_ALL ^ FLAG_SE)) {
        triangles.push_back(v);
        triangles.push_back(v + sx);
        triangles.push_back(v1);

        triangles.push_back(v1);
        triangles.push_back(v + sx);
        triangles.push_back(v2);

        triangles.push_back(v + sx);
        triangles.push_back(v + s);
        triangles.push_back(v2);
    }
}

void MakeFillHalf(triangles_t& triangles, flags_t flags,
                  discrete_t d, vals_t vals) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;
    
    switch (flags) {
    case (FLAG_SW | FLAG_NW):
        x1 = d.x;
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y;
        x3 = d.x+d.sx;
        y3 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x4 = d.x;
        y4 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        break;

    case (FLAG_NE | FLAG_SE):
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x3 = d.x+d.sx;
        y3 = d.y+d.sy;
        x4 = d.x;
        y4 = d.y+d.sy;
        break;

    case (FLAG_NW | FLAG_NE):
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y;
        x3 = d.x+d.sx;
        y3 = d.y+d.sy;
        x4 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y4 = d.y+d.sy;
        break;

    case (FLAG_SE | FLAG_SW):
        x1 = d.x;
        y1 = d.y;
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;
        x3 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y3 = d.y+d.sy;
        x4 = d.x;
        y4 = d.y+d.sy;
        break;
    }

    triangles.emplace_back(x1, y1);
    triangles.emplace_back(x3, y3);
    triangles.emplace_back(x4, y4);

    triangles.emplace_back(x1, y1);
    triangles.emplace_back(x2, y2);
    triangles.emplace_back(x3, y3);
}

void MakeFillAmbiguity(triangles_t& triangles, flags_t flags, bool u,
                       discrete_t d, vals_t vals, double /*v*/) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
    y1 = d.y;
    x2 = d.x+d.sx;
    y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
    x3 = d.x+d.sx * ValuesRatio(vals, 3, 2);
    y3 = d.y+d.sy;
    x4 = d.x;
    y4 = d.y+d.sy * ValuesRatio(vals, 0, 3);

    if (u) {
        triangles.emplace_back(x1, y1);
        triangles.emplace_back(x2, y2);
        triangles.emplace_back(x3, y3);

        triangles.emplace_back(x1, y1);
        triangles.emplace_back(x3, y3);
        triangles.emplace_back(x4, y4);
    }

    if (flags == (FLAG_SW | FLAG_NE)) {
        triangles.emplace_back(d.x, d.y);
        triangles.emplace_back(x1, y1);
        triangles.emplace_back(x4, y4);

        triangles.emplace_back(d.x + d.sx, d.y + d.sy);
        triangles.emplace_back(x3, y3);
        triangles.emplace_back(x2, y2);

    } else if (flags == (FLAG_NW | FLAG_SE)) {
        triangles.emplace_back(d.x, d.y + d.sy);
        triangles.emplace_back(x4, y4);
        triangles.emplace_back(x3, y3);

        triangles.emplace_back(d.x + d.sx, d.y);
        triangles.emplace_back(x2, y2);
        triangles.emplace_back(x1, y1);
    }
}
