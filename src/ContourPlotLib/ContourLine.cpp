#include "stdafx.h"
#include "Matrix.h"
#include "GraphicsUtils.h"
#include "GraphicsLogger.h"
#include "ContourPlot.h"
#include "ContourLine.h"

bool ContourLine::Update(matrix_t* points, const hmm_vec4& area, double t) {
    threshold = t;
    
    this->area = area;
    
    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;

    double dX = (area.Y - area.X) / static_cast<double>(xdiv);
    double dY = (area.W - area.Z) / static_cast<double>(ydiv);

    int max_idx = xdiv * ydiv;

    lines_t lines;
    lines.reserve(max_idx * 2);

#ifdef USE_OPENMP
#pragma omp parallel
#endif
    {
        lines_t iter_lines;

#ifdef USE_OPENMP
#pragma omp for nowait
#endif
        for (int idx = 0; idx < max_idx; idx++) {

            int i = idx % xdiv;
            int j = idx / xdiv;

            float y = area.Z + static_cast<float>(j) * dY;
            float x = area.X + static_cast<float>(i) * dX;

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
                float x1 = 0.f, y1 = 0.f;
                float x2 = 0.f, y2 = 0.f;
                float sx = dX, sy = dY;

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

                iter_lines.push_back({ x1, y1, x2, y2 });
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest)
                || flags == (SquareFlags::NorthWest | SquareFlags::NorthEast)
                || flags == (SquareFlags::NorthEast | SquareFlags::SouthEast)
                || flags == (SquareFlags::SouthEast | SquareFlags::SouthWest)) {
                // Half
                float x1 = 0.f, y1 = 0.f;
                float x2 = 0.f, y2 = 0.f;
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

                iter_lines.push_back({ x1, y1, x2, y2 });
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) ||
                flags == (SquareFlags::NorthWest | SquareFlags::SouthEast)) {
                // Ambiguity
                double v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                bool u = v > 0.0;

                float x1 = 0.f, y1 = 0.f;
                float x2 = 0.f, y2 = 0.f;
                float x3 = 0.f, y3 = 0.f;
                float x4 = 0.f, y4 = 0.f;
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

                iter_lines.push_back({ x1, y1, x2, y2 });
                iter_lines.push_back({ x3, y3, x4, y4 });
            }
        }

#ifdef USE_OPENMP
#pragma omp critical
#endif
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(hmm_vec4) * lines.size(), lines.data(),
                 GL_DYNAMIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel line contour with " << lines.size() << " lines";

    return true;
}

void ContourLine::Render(const hmm_mat4& mvp,
                         double zoom,
                         const hmm_vec2& offset,
                         const FloatColor& c) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, (GLfloat)zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, reinterpret_cast<const GLfloat*>(&offset)); LOGOPENGLERROR();
    glUniform2f(u_res, static_cast<GLfloat>(w), static_cast<GLfloat>(h)); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c.data()); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}
