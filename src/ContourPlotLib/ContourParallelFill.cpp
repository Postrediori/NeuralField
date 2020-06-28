#include "stdafx.h"
#include "Matrix.h"
#include "MathUtils.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallelFill.h"

ContourParallelFill::ContourParallelFill()
    : ContourPlot() {
}

bool ContourParallelFill::update(matrix_t* points, area_t a, double t) {
    threshold = t;

    area = a;

    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    int max_idx = xdiv * ydiv;

    triangles_t all_triangles;
    all_triangles.reserve(max_idx * 3 * 4);

#pragma omp parallel
    {
        triangles_t triangles;

#pragma omp for nowait
        for (int idx=0; idx<max_idx; idx++) {
            int i = idx % xdiv;
            int j = idx / xdiv;

            double y = a.ymin + j * dY;
            double x = a.xmin + i * dX;
        
            vals_t vals;
            vals.v[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
            vals.v[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals.v[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals.v[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;

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
                    x1 = x+sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x;
                    y2 = y+sy * ValuesRatio(vals, 0, 3);
                    break;

                case FLAG_NW:
                case (FLAG_ALL ^ FLAG_NW):
                    x1 = x+sx;
                    y1 = y+sy * ValuesRatio(vals, 1, 2);
                    x2 = x+sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                    break;

                case FLAG_NE:
                case (FLAG_ALL ^ FLAG_NE):
                    x1 = x+sx * ValuesRatio(vals, 3, 2);
                    y1 = y+sy;
                    x2 = x+sx;
                    y2 = y+sy * ValuesRatio(vals, 1, 2);
                    break;

                case FLAG_SE:
                case (FLAG_ALL ^ FLAG_SE):
                    x1 = x;
                    y1 = y+sy * ValuesRatio(vals, 0, 3);
                    x2 = x+sx * ValuesRatio(vals, 3, 2);
                    y2 = y+sy;
                    break;
                }

                if (flags == FLAG_SW) {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);

                } else if (flags == (FLAG_ALL ^ FLAG_SW)) {
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x+sx, y+sy);

                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x2, y2);

                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x, y+sy);

                } else if (flags == FLAG_NW) {
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);

                } else if (flags == (FLAG_ALL ^ FLAG_NW)) {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x, y+sy);

                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);

                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y+sy);

                } else if (flags == FLAG_NE) {
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);

                } else if (flags == (FLAG_ALL ^ FLAG_NE)) {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x2, y2);

                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);

                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x, y+sy);

                } else if (flags == FLAG_SE) {
                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);

                } else if (flags == (FLAG_ALL ^ FLAG_SE)) {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x1, y1);

                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x2, y2);

                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags == (FLAG_SW | FLAG_NW)
                || flags == (FLAG_NW | FLAG_NE)
                || flags == (FLAG_NE | FLAG_SE)
                || flags == (FLAG_SE | FLAG_SW)) {
                // Half
                double x1, y1;
                double x2, y2;
                double x3, y3;
                double x4, y4;
                double sx = dX, sy = dY;

                switch (flags) {
                case (FLAG_SW | FLAG_NW):
                    x1 = x;
                    y1 = y;
                    x2 = x+sx;
                    y2 = y;
                    x3 = x+sx;
                    y3 = y+sy * ValuesRatio(vals, 1, 2);
                    x4 = x;
                    y4 = y+sy * ValuesRatio(vals, 0, 3);
                    break;

                case (FLAG_SE | FLAG_NE):
                    x1 = x;
                    y1 = y+sy * ValuesRatio(vals, 0, 3);
                    x2 = x+sx;
                    y2 = y+sy * ValuesRatio(vals, 1, 2);
                    x3 = x+sx;
                    y3 = y+sy;
                    x4 = x;
                    y4 = y+sy;
                    break;

                case (FLAG_NW | FLAG_NE):
                    x1 = x+sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x+sx;
                    y2 = y;
                    x3 = x+sx;
                    y3 = y+sy;
                    x4 = x+sx * ValuesRatio(vals, 3, 2);
                    y4 = y+sy;
                    break;

                case (FLAG_SW | FLAG_SE):
                    x1 = x;
                    y1 = y;
                    x2 = x+sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                    x3 = x+sx * ValuesRatio(vals, 3, 2);
                    y3 = y+sy;
                    x4 = x;
                    y4 = y+sy;
                    break;
                }

                triangles.emplace_back(x1, y1);
                triangles.emplace_back(x3, y3);
                triangles.emplace_back(x4, y4);

                triangles.emplace_back(x1, y1);
                triangles.emplace_back(x2, y2);
                triangles.emplace_back(x3, y3);

            } else if (flags == (FLAG_SW | FLAG_NE) || flags == (FLAG_NW | FLAG_SE)) {
                // Ambiguity
                double v;
                bool u;
                v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                u = v > 0.0;

                float x1, y1;
                float x2, y2;
                float x3, y3;
                float x4, y4;
                float sx = dX, sy = dY;

                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
                x3 = x+sx * ValuesRatio(vals, 3, 2);
                y3 = y+sy;
                x4 = x;
                y4 = y+sy * ValuesRatio(vals, 0, 3);
            
                if (u) {
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x3, y3);

                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x3, y3);
                    triangles.emplace_back(x4, y4);
                }
            
                if (flags == (FLAG_SW | FLAG_NE)) {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x4, y4);

                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x3, y3);
                    triangles.emplace_back(x2, y2);

                } else if (flags == (FLAG_NW | FLAG_SE)) {
                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x4, y4);
                    triangles.emplace_back(x3, y3);

                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);
                }

            } else if (flags == FLAG_ALL) {
                // Full fill
                double sx = dX, sy = dY;
            
                triangles.emplace_back(x, y);
                triangles.emplace_back(x+sx, y+sy);
                triangles.emplace_back(x, y+sy);

                triangles.emplace_back(x, y);
                triangles.emplace_back(x+sx, y);
                triangles.emplace_back(x+sx, y+sy);
            }
        }

#pragma omp critical
        {
            all_triangles.insert(
                all_triangles.end(),
                triangles.begin(),
                triangles.end());
        }
    }

    vbo_count = all_triangles.size();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(Math::vec2f) * all_triangles.size(), all_triangles.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel filled contour with " << all_triangles.size() / 3 << " triangles";

    return true;
}

void ContourParallelFill::render(const glm::mat4& mvp,
                                 double zoom,
                                 const glm::vec2& offset,
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
