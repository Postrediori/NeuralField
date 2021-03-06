#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourLine.h"

void MakeCorner(lines_t& lines, SquareFlags flags, discrete_t d, vals_t vals);
void MakeHalf(lines_t& lines, SquareFlags flags, discrete_t d, vals_t vals);
void MakeAmbiguity(lines_t& lines, SquareFlags flags, bool u, discrete_t d, vals_t vals, double v);

ContourLine::ContourLine()
    : ContourPlot() {
}

bool ContourLine::update(matrix_t* points, area_t a, double t) {
    threshold = t;
    
    area = a;

    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    SquareFlags flags;
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

            if (flags == SquareFlags::SouthWest || flags == SquareFlags::NorthWest ||
                flags == SquareFlags::NorthEast || flags == SquareFlags::SouthEast
                || flags == (SquareFlags::All ^ SquareFlags::SouthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthEast)
                || flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                // One corner
                MakeCorner(lines, flags, d, vals);
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest)
                || flags == (SquareFlags::NorthWest | SquareFlags::NorthEast)
                || flags == (SquareFlags::NorthEast | SquareFlags::SouthEast)
                || flags == (SquareFlags::SouthEast | SquareFlags::SouthWest)) {
                // Half
                MakeHalf(lines, flags, d, vals);
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) ||
                flags == (SquareFlags::NorthWest | SquareFlags::SouthEast)) {
                // Ambiguity
                v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                u = v > 0.0;
                MakeAmbiguity(lines, flags, u, d, vals, v);
            }
            else if (flags == SquareFlags::All || flags == SquareFlags::None) {
                // No lines
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

void ContourLine::render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
                         const std::array<GLfloat, 4>& c) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform2f(u_res, static_cast<GLfloat>(w), static_cast<GLfloat>(h)); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c.data()); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}

void MakeCorner(lines_t& lines, SquareFlags flags,
                discrete_t d, vals_t vals) {
    if (flags == SquareFlags::SouthWest ||
        flags == (SquareFlags::All ^ SquareFlags::SouthWest)) {
        Math::vec4f line(
            d.x,
            d.y + d.sy * ValuesRatio(vals, 0, 3),
            d.x + d.sx * ValuesRatio(vals, 0, 1),
            d.y);
        lines.push_back(line);
    }
    else if (flags == SquareFlags::NorthWest ||
        flags == (SquareFlags::All ^ SquareFlags::NorthWest)) {
        Math::vec4f line(
            d.x + d.sx * ValuesRatio(vals, 0, 1),
            d.y,
            d.x + d.sx,
            d.y + d.sy * ValuesRatio(vals, 1, 2));
        lines.push_back(line);
    }
    else if (flags == SquareFlags::NorthEast ||
        flags == (SquareFlags::All ^ SquareFlags::NorthEast)) {
        Math::vec4f line(
            d.x + d.sx * ValuesRatio(vals, 3, 2),
            d.y + d.sy,
            d.x + d.sx,
            d.y + d.sy * ValuesRatio(vals, 1, 2));
        lines.push_back(line);
    }
    else if (flags == SquareFlags::SouthEast || 
        flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
        Math::vec4f line(
            d.x,
            d.y+d.sy * ValuesRatio(vals, 0, 3),
            d.x+d.sx * ValuesRatio(vals, 3, 2),
            d.y+d.sy);
        lines.push_back(line);
    }
}

void MakeHalf(lines_t& lines, SquareFlags flags,
              discrete_t d, vals_t vals) {
    if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest) ||
        flags == (SquareFlags::NorthEast | SquareFlags::SouthEast)) {
        Math::vec4f line(
            d.x,
            d.y + d.sy * ValuesRatio(vals, 0, 3),
            d.x + d.sx,
            d.y + d.sy * ValuesRatio(vals, 1, 2));
        lines.push_back(line);
    }
    else if (flags == (SquareFlags::NorthWest | SquareFlags::NorthEast) ||
        flags == (SquareFlags::SouthEast | SquareFlags::SouthWest)) {
        Math::vec4f line(
            d.x+d.sx * ValuesRatio(vals, 0, 1),
            d.y,
            d.x+d.sx * ValuesRatio(vals, 3, 2),
            d.y+d.sy);
        lines.push_back(line);
    }
}

void MakeAmbiguity(lines_t& lines, SquareFlags flags, bool u,
                   discrete_t d, vals_t vals, double /*v*/) {
    if ((flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) && u) ||
        (flags == (SquareFlags::NorthWest | SquareFlags::SouthEast) && !u)) {
        Math::vec4f line1(
            d.x,
            d.y+d.sy * ValuesRatio(vals, 0, 3),
            d.x+d.sx * ValuesRatio(vals, 3, 2),
            d.y+d.sy);

        Math::vec4f line2(
            d.x+d.sx * ValuesRatio(vals, 0, 1),
            d.y,
            d.x+d.sx,
            d.y+d.sy * ValuesRatio(vals, 1, 2));

        lines.push_back(line1);
        lines.push_back(line2);
    }
    else if ((flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) && !u) ||
        (flags == (SquareFlags::NorthWest | SquareFlags::SouthEast) && u)) {
        Math::vec4f line1(
            d.x,
            d.y+d.sy * ValuesRatio(vals, 0, 3),
            d.x+d.sx * ValuesRatio(vals, 0, 1),
            d.y);

        Math::vec4f line2(
            d.x+d.sx * ValuesRatio(vals, 3, 2),
            d.y+d.sy,
            d.x+d.sx,
            d.y+d.sy * ValuesRatio(vals, 1, 2));

        lines.push_back(line1);
        lines.push_back(line2);
    }
}
