#include "stdafx.h"
#include "Matrix.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallelFill.h"

ContourParallelFill::ContourParallelFill(GLuint p)
    : ContourPlot(p) {
}

bool ContourParallelFill::update(matrix_t* points, area_t a, double t) {
    threshold = t;

    area = a;

    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    int max_idx = xdiv * ydiv;

    triangles_t triangles;
    triangles.reserve(max_idx * 3 * 4);

#ifdef _MSC_VER
#pragma omp parallel for shared(points, triangles)
#else
#pragma omp parallel for
#endif
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

        //unsigned char flags;
        //flags = CellType(vals);
        flags_t flags = FLAG_NO;
        if (vals.v[0]>0.0) flags |= FLAG_SW;
        if (vals.v[1]>0.0) flags |= FLAG_NW;
        if (vals.v[2]>0.0) flags |= FLAG_NE;
        if (vals.v[3]>0.0) flags |= FLAG_SE;

        if (flags==1 || flags==2 || flags==4 || flags==7
            || flags==8 || flags==11 || flags==13 || flags==14) {
            // One corner
            double x1, y1;
            double x2, y2;
            double sx = dX, sy = dY;

            switch (flags) {
            case 1:
            case 14:
                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x;
                y2 = y+sy * ValuesRatio(vals, 0, 3);
                break;

            case 2:
            case 13:
                x1 = x+sx;
                y1 = y+sy * ValuesRatio(vals, 1, 2);
                x2 = x+sx * ValuesRatio(vals, 0, 1);
                y2 = y;
                break;

            case 4:
            case 11:
                x1 = x+sx * ValuesRatio(vals, 3, 2);
                y1 = y+sy;
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
                break;

            case 7:
            case 8:
                x1 = x;
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx * ValuesRatio(vals, 3, 2);
                y2 = y+sy;
                break;
            }

            if (flags==1) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags==14) {
#pragma omp critical
                {
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x+sx, y+sy);

                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x2, y2);

                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x, y+sy);
                }

            } else if (flags==2) {
#pragma omp critical
                {
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags==13) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x, y+sy);

                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);

                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x+sx, y+sy);
                }

            } else if (flags==4) {
#pragma omp critical
                {
                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags==11) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x2, y2);

                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);

                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x, y+sy);
                }

            } else if (flags==8) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags==7) {
#pragma omp critical
                {
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
            }

        } else if (flags==3 || flags==6 || flags==9 || flags==12) {
            // Half
            double x1, y1;
            double x2, y2;
            double x3, y3;
            double x4, y4;
            double sx = dX, sy = dY;

            switch (flags) {
            case 3:
                x1 = x;
                y1 = y;
                x2 = x+sx;
                y2 = y;
                x3 = x+sx;
                y3 = y+sy * ValuesRatio(vals, 1, 2);
                x4 = x;
                y4 = y+sy * ValuesRatio(vals, 0, 3);
                break;

            case 12:
                x1 = x;
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
                x3 = x+sx;
                y3 = y+sy;
                x4 = x;
                y4 = y+sy;
                break;

            case 6:
                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x+sx;
                y2 = y;
                x3 = x+sx;
                y3 = y+sy;
                x4 = x+sx * ValuesRatio(vals, 3, 2);
                y4 = y+sy;
                break;

            case 9:
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

#pragma omp critical
            {
                triangles.emplace_back(x1, y1);
                triangles.emplace_back(x3, y3);
                triangles.emplace_back(x4, y4);

                triangles.emplace_back(x1, y1);
                triangles.emplace_back(x2, y2);
                triangles.emplace_back(x3, y3);
            }

        } else if (flags==5 || flags==10) {
            // Ambiguity
            double v;
            bool u;
            v = (points->data[(j  )*(xdiv+1)+(i  )]+points->data[(j  )*(xdiv+1)+(i+1)]+
                 points->data[(j+1)*(xdiv+1)+(i+1)]+points->data[(j+1)*(xdiv+1)+(i  )])/4.f;
            v -= threshold;
            u = v>0.f;

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
#pragma omp critical
                {
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x3, y3);

                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x3, y3);
                    triangles.emplace_back(x4, y4);
                }
            }
            
            if (flags==5) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y);
                    triangles.emplace_back(x1, y1);
                    triangles.emplace_back(x4, y4);

                    triangles.emplace_back(x+sx, y+sy);
                    triangles.emplace_back(x3, y3);
                    triangles.emplace_back(x2, y2);
                }

            } else if (flags==10) {
#pragma omp critical
                {
                    triangles.emplace_back(x, y+sy);
                    triangles.emplace_back(x4, y4);
                    triangles.emplace_back(x3, y3);

                    triangles.emplace_back(x+sx, y);
                    triangles.emplace_back(x2, y2);
                    triangles.emplace_back(x1, y1);
                }
            }

        } else if (flags==15) {
            // Full fill
            double sx = dX, sy = dY;
            
#pragma omp critical
            {
                triangles.emplace_back(x,    y);
                triangles.emplace_back(x+sx, y+sy);
                triangles.emplace_back(x,    y+sy);

                triangles.emplace_back(x,    y);
                triangles.emplace_back(x+sx, y);
                triangles.emplace_back(x+sx, y+sy);
            }
        }
    }

    vbo_count = triangles.size();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * triangles.size(), triangles.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel filled contour with " << triangles.size() / 3 << " triangles" << std::endl;

    return true;
}

void ContourParallelFill::render(const glm::mat4& mvp,
                                 double zoom,
                                 const glm::vec2& offset,
                                 const GLfloat c[]) {
    glUseProgram(program); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform2f(u_res, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c); LOGOPENGLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glEnableVertexAttribArray(a_coord); LOGOPENGLERROR();
    glVertexAttribPointer(a_coord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLES, 0, vbo_count); LOGOPENGLERROR();
}
