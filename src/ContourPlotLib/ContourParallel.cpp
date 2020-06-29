#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallel.h"

ContourParallel::ContourParallel()
    : ContourPlot() {
}

bool ContourParallel::update(matrix_t* points, area_t a, double t) {
    threshold = t;
    
    area = a;
    
    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;

    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    int max_idx = xdiv * ydiv;

    lines_t lines;
    lines.reserve(max_idx * 2);

#pragma omp parallel
    {
        lines_t iter_lines;

#pragma omp for nowait
        for (int idx = 0; idx < max_idx; idx++) {

            int i = idx % xdiv;
            int j = idx / xdiv;

            double y = a.ymin + j * dY;
            double x = a.xmin + i * dX;

            vals_t vals;
            vals.v[0] = points->data[(j)*(xdiv + 1) + (i)] - threshold;
            vals.v[1] = points->data[(j)*(xdiv + 1) + (i + 1)] - threshold;
            vals.v[2] = points->data[(j + 1)*(xdiv + 1) + (i + 1)] - threshold;
            vals.v[3] = points->data[(j + 1)*(xdiv + 1) + (i)] - threshold;

            SquareFlags flags = CellType(vals);

            if (flags == SquareFlags::SouthWest || flags == SquareFlags::NorthWest ||
                flags == SquareFlags::NorthEast || flags == SquareFlags::SouthEast
                || flags == (SquareFlags::All ^ SquareFlags::SouthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthEast)
                || flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                // One corner
                double x1, y1;
                double x2, y2;
                double sx = dX, sy = dY;

                if (flags == SquareFlags::SouthWest ||
                    flags == (SquareFlags::All ^ SquareFlags::SouthWest)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                }
                else if (flags == SquareFlags::NorthWest ||
                    flags == (SquareFlags::All ^ SquareFlags::NorthWest)) {
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if (flags == SquareFlags::NorthEast ||
                    flags == (SquareFlags::All ^ SquareFlags::NorthEast)) {
                    x1 = x + sx * ValuesRatio(vals, 3, 2);
                    y1 = y + sy;
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if (flags == SquareFlags::SouthEast ||
                    flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;
                }

                iter_lines.emplace_back(x1, y1, x2, y2);
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest)
                || flags == (SquareFlags::NorthWest | SquareFlags::NorthEast)
                || flags == (SquareFlags::NorthEast | SquareFlags::SouthEast)
                || flags == (SquareFlags::SouthEast | SquareFlags::SouthWest)) {
                // Half
                double x1, y1;
                double x2, y2;
                double sx = dX, sy = dY;

                if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest) ||
                    flags == (SquareFlags::SouthEast | SquareFlags::NorthEast)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if (flags == (SquareFlags::NorthWest | SquareFlags::NorthEast) ||
                    flags == (SquareFlags::SouthWest | SquareFlags::SouthEast)) {
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;
                }

                iter_lines.emplace_back(x1, y1, x2, y2);
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) ||
                flags == (SquareFlags::NorthWest | SquareFlags::SouthEast)) {
                // Ambiguity
                double v;
                bool u;
                v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                u = v > 0.0;

                double x1, y1;
                double x2, y2;
                double x3, y3;
                double x4, y4;
                double sx = dX, sy = dY;

                if ((flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) && u) ||
                    (flags == (SquareFlags::NorthWest | SquareFlags::SouthEast) && !u)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;

                    x3 = x + sx * ValuesRatio(vals, 0, 1);
                    y3 = y;
                    x4 = x + sx;
                    y4 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if ((flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) && !u) ||
                    (flags == (SquareFlags::NorthWest | SquareFlags::SouthEast) && u)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 0, 1);
                    y2 = y;

                    x3 = x + sx * ValuesRatio(vals, 3, 2);
                    y3 = y + sy;
                    x4 = x + sx;
                    y4 = y + sy * ValuesRatio(vals, 1, 2);
                }

                iter_lines.emplace_back(x1, y1, x2, y2);
                iter_lines.emplace_back(x3, y3, x4, y4);
            }
        }

#pragma omp critical
        {
            if (!iter_lines.empty())
            {
                lines.insert(lines.end(),
                    iter_lines.begin(),
                    iter_lines.end());
            }
        }
    }

    vbo_count = lines.size() * 2;

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(Math::vec4f) * lines.size(), lines.data(),
                 GL_DYNAMIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel line contour with " << lines.size() << " lines";

    return true;
}

void ContourParallel::render(const glm::mat4& mvp,
                             double zoom,
                             const glm::vec2& offset,
                             const std::array<GLfloat, 4>& c) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, (GLfloat)zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform2f(u_res, static_cast<GLfloat>(w), static_cast<GLfloat>(h)); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c.data()); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}
