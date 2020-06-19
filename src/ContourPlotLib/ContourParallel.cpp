#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallel.h"

ContourParallel::ContourParallel(GLuint p)
    : ContourPlot(p) {
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

            flags_t flags = CellType(vals);

            if (flags == FLAG_SW || flags == FLAG_NW || flags == FLAG_NE || flags == FLAG_SE
                || flags == (FLAG_ALL ^ FLAG_SW)
                || flags == (FLAG_ALL ^ FLAG_NW)
                || flags == (FLAG_ALL ^ FLAG_NE)
                || flags == (FLAG_ALL ^ FLAG_SE)) {
                // One corner
                double x1, y1;
                double x2, y2;
                double sx = dX, sy = dY;

                switch (flags) {
                case FLAG_SW:
                case (FLAG_ALL ^ FLAG_SW):
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                    break;

                case FLAG_NW:
                case (FLAG_ALL ^ FLAG_NW):
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                    break;

                case FLAG_NE:
                case (FLAG_ALL ^ FLAG_NE):
                    x1 = x + sx * ValuesRatio(vals, 3, 2);
                    y1 = y + sy;
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                    break;

                case FLAG_SE:
                case (FLAG_ALL ^ FLAG_SE):
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;
                    break;
                }

                iter_lines.emplace_back(x1, y1, x2, y2);

            }
            else if (flags == (FLAG_SW | FLAG_NW)
                || flags == (FLAG_NW | FLAG_NE)
                || flags == (FLAG_NE | FLAG_SE)
                || flags == (FLAG_SE | FLAG_SW)) {
                // Half
                double x1, y1;
                double x2, y2;
                double sx = dX, sy = dY;

                if (flags == (FLAG_SW | FLAG_NW) || flags == (FLAG_SE | FLAG_NE)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if (flags == (FLAG_NW | FLAG_NE) || flags == (FLAG_SW | FLAG_SE)) {
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;
                }

                iter_lines.emplace_back(x1, y1, x2, y2);

            }
            else if (flags == (FLAG_SW | FLAG_NE) || flags == (FLAG_NW | FLAG_SE)) {
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

                if ((flags == (FLAG_SW | FLAG_NE) && u) || (flags == (FLAG_NW | FLAG_SE) && !u)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx * ValuesRatio(vals, 3, 2);
                    y2 = y + sy;

                    x3 = x + sx * ValuesRatio(vals, 0, 1);
                    y3 = y;
                    x4 = x + sx;
                    y4 = y + sy * ValuesRatio(vals, 1, 2);
                }
                if ((flags == (FLAG_SW | FLAG_NE) && !u) || (flags == (FLAG_NW | FLAG_SE) && u)) {
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

void ContourParallel::render(const Math::mat4f& mvp,
                             double zoom,
                             const Math::vec2f& offset,
                             const GLfloat c[]) {
    glUseProgram(program); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, (const GLfloat *)(&mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, (GLfloat)zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, (const GLfloat *)(&offset)); LOGOPENGLERROR();
    glUniform2f(u_res, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c); LOGOPENGLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glEnableVertexAttribArray(a_coord); LOGOPENGLERROR();
    glVertexAttribPointer(a_coord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
}
