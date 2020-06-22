#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourLine.h"

void MakeCorner(lines_t& lines, flags_t flags, discrete_t d, vals_t vals);
void MakeHalf(lines_t& lines, flags_t flags, discrete_t d, vals_t vals);
void MakeAmbiguity(lines_t& lines, flags_t flags, bool u, discrete_t d, vals_t vals, double v);

ContourLine::ContourLine(GLuint p)
    : ContourPlot(p) {
}

bool ContourLine::update(matrix_t* points, area_t a, double t) {
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
    discrete_t d;

    lines_t lines;

    for (int j=0; j<ydiv; j++) {
        double y = a.ymin + j * dY;
        for (int i=0; i<xdiv; i++) {
            double x = a.xmin + i * dX;
            d = {x, y, dX, dY};
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
                MakeCorner(lines, flags, d, vals);
                break;

            case (FLAG_SW | FLAG_NW):
            case (FLAG_NW | FLAG_NE):
            case (FLAG_NE | FLAG_SE):
            case (FLAG_SE | FLAG_SW):
                // Half
                MakeHalf(lines, flags, d, vals);
                break;

            case (FLAG_SW | FLAG_NE):
            case (FLAG_NW | FLAG_SE):
                // Ambiguity
                v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                u = v > 0.0;
                MakeAmbiguity(lines, flags, u, d, vals, v);
                break;

            case FLAG_ALL:
            case FLAG_NO:
                // No lines
                break;

            default:
                break;
            }
        }
    }

    vbo_count = lines.size() * 2;

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(Math::vec4f) * lines.size(),
        lines.data(), GL_DYNAMIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created filled contour with " << lines.size() / 2 << " lines";
    
    return true;
}

void ContourLine::render(const Math::mat4f& mvp, double zoom, const Math::vec2f& offset,
                         const GLfloat c[]) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, (const GLfloat *)(&mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, (const GLfloat *)(&offset)); LOGOPENGLERROR();
    glUniform2f(u_res, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}

void MakeCorner(lines_t& lines, flags_t flags,
                discrete_t d, vals_t vals) {
    double x1, y1;
    double x2, y2;

    switch (flags) {
    case FLAG_SW:
    case (FLAG_ALL ^ FLAG_SW):
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;
        break;

    case FLAG_NW:
    case (FLAG_ALL ^ FLAG_NW):
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
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

    lines.emplace_back(x1, y1, x2, y2);
}

void MakeHalf(lines_t& lines, flags_t flags,
              discrete_t d, vals_t vals) {
    double x1, y1;
    double x2, y2;

    switch (flags) {
    case (FLAG_SW | FLAG_NW):
    case (FLAG_NE | FLAG_SE):
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        break;
        
    case (FLAG_NW | FLAG_NE):
    case (FLAG_SE | FLAG_SW):
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y2 = d.y+d.sy;
        break;
    }

    lines.emplace_back(x1, y1, x2, y2);
}

void MakeAmbiguity(lines_t& lines, flags_t flags, bool u,
                   discrete_t d, vals_t vals, double /*v*/) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    if ((flags == (FLAG_SW | FLAG_NE) && u) || (flags == (FLAG_NW | FLAG_SE) && !u)) {
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y2 = d.y+d.sy;

        x3 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y3 = d.y;
        x4 = d.x+d.sx;
        y4 = d.y+d.sy * ValuesRatio(vals, 1, 2);
    }

    if ((flags == (FLAG_SW | FLAG_NE) && !u) || (flags == (FLAG_NW | FLAG_SE) && u)) {
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;

        x3 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y3 = d.y+d.sy;
        x4 = d.x+d.sx;
        y4 = d.y+d.sy * ValuesRatio(vals, 1, 2);
    }

    lines.emplace_back(x1, y1, x2, y2);
    lines.emplace_back(x3, y3, x4, y4);
}
