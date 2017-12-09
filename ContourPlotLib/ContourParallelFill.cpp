#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallelFill.h"

ContourParallelFill::ContourParallelFill(GLuint p)
    : ContourPlot(p) {
}

bool ContourParallelFill::init(const float* const points,
                              int xdiv, int ydiv,
                              float xmn, float xmx, float ymn, float ymx,
                              float t) {
    threshold = t;
    vbo_count = 0;
    vbo = 0;

    xmin = xmn;
    xmax = xmx;
    ymin = ymn;
    ymax = ymx;

    float dX = (xmax - xmin) / (float)xdiv;
    float dY = (ymax - ymin) / (float)ydiv;

    //unsigned char flags, u;
    //float x, y;
    //float vals[4];
    //float v;

    int c = 0;
    int max_idx = xdiv * ydiv;

    std::vector<glm::vec2> triangles;
    triangles.resize(max_idx * 3 * 4);

#ifdef _MSC_VER
#pragma omp parallel for shared(points, triangles)
#else
#pragma omp parallel for
#endif
    for (int idx=0; idx<max_idx; idx++) {
        int i = idx % xdiv;
        int j = idx / xdiv;

        float y = ymin + j * dY;
        float x = xmin + i * dX;
        
        float vals[4];
        vals[0] = points[(j  )*(xdiv+1)+(i  )] - threshold;
        vals[1] = points[(j  )*(xdiv+1)+(i+1)] - threshold;
        vals[2] = points[(j+1)*(xdiv+1)+(i+1)] - threshold;
        vals[3] = points[(j+1)*(xdiv+1)+(i  )] - threshold;

        //unsigned char flags;
        //flags = CellType(vals);
        unsigned char flags = FLAG_NO;
        if (vals[0]>0.f) flags |= FLAG_SW;
        if (vals[1]>0.f) flags |= FLAG_NW;
        if (vals[2]>0.f) flags |= FLAG_NE;
        if (vals[3]>0.f) flags |= FLAG_SE;

        if (flags==1 || flags==2 || flags==4 || flags==7
            || flags==8 || flags==11 || flags==13 || flags==14) {
            // One corner
            float x1, y1;
            float x2, y2;
            float sx = dX, sy = dY;

            switch (flags) {
            case 1:
            case 14:
                x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y1 = y;
                x2 = x;
                y2 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                break;

            case 2:
            case 13:
                x1 = x+sx;
                y1 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
                x2 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y2 = y;
                break;

            case 4:
            case 11:
                x1 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y1 = y+sy;
                x2 = x+sx;
                y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
                break;

            case 7:
            case 8:
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y2 = y+sy;
                break;
            }

            if (flags==1) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x2, y2);
                }

            } else if (flags==14) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x+sx, y+sy);

                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x+sx, y+sy);
                    triangles[c++] = glm::vec2(x2, y2);

                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x+sx, y+sy);
                    triangles[c++] = glm::vec2(x, y+sy);
                }

            } else if (flags==2) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x2, y2);
                }

            } else if (flags==13) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x, y+sy);

                    triangles[c++] = glm::vec2(x, y+sy);
                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x1, y1);

                    triangles[c++] = glm::vec2(x, y+sy);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x+sx, y+sy);
                }

            } else if (flags==4) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x+sx, y+sy);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x2, y2);
                }

            } else if (flags==11) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x2, y2);

                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x1, y1);

                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x, y+sy);
                }

            } else if (flags==8) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y+sy);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x2, y2);
                }

            } else if (flags==7) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x1, y1);

                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x2, y2);

                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x+sx, y+sy);
                    triangles[c++] = glm::vec2(x2, y2);
                }
            }

        } else if (flags==3 || flags==6 || flags==9 || flags==12) {
            // Half
            float x1, y1;
            float x2, y2;
            float x3, y3;
            float x4, y4;
            float sx = dX, sy = dY;

            switch (flags) {
            case 3:
                x1 = x;
                y1 = y;
                x2 = x+sx;
                y2 = y;
                x3 = x+sx;
                y3 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
                x4 = x;
                y4 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                break;

            case 12:
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx;
                y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
                x3 = x+sx;
                y3 = y+sy;
                x4 = x;
                y4 = y+sy;
                break;

            case 6:
                x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y1 = y;
                x2 = x+sx;
                y2 = y;
                x3 = x+sx;
                y3 = y+sy;
                x4 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y4 = y+sy;
                break;

            case 9:
                x1 = x;
                y1 = y;
                x2 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y2 = y;
                x3 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y3 = y+sy;
                x4 = x;
                y4 = y+sy;
                break;
            }

#pragma omp critical
            {
                triangles[c++] = glm::vec2(x1, y1);
                triangles[c++] = glm::vec2(x3, y3);
                triangles[c++] = glm::vec2(x4, y4);

                triangles[c++] = glm::vec2(x1, y1);
                triangles[c++] = glm::vec2(x2, y2);
                triangles[c++] = glm::vec2(x3, y3);
            }

        } else if (flags==5 || flags==10) {
            // Ambiguity
            float v;
            bool u;
            v = (points[(j  )*(xdiv+1)+(i  )]+points[(j  )*(xdiv+1)+(i+1)]+
                 points[(j+1)*(xdiv+1)+(i+1)]+points[(j+1)*(xdiv+1)+(i  )])/4.f;
            v -= threshold;
            u = v>0.f;

            float x1, y1;
            float x2, y2;
            float x3, y3;
            float x4, y4;
            float sx = dX, sy = dY;

            x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
            y1 = y;
            x2 = x+sx;
            y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
            x3 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
            y3 = y+sy;
            x4 = x;
            y4 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
            
            if (u) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x3, y3);

                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x3, y3);
                    triangles[c++] = glm::vec2(x4, y4);
                }
            }
            
            if (flags==5) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y);
                    triangles[c++] = glm::vec2(x1, y1);
                    triangles[c++] = glm::vec2(x4, y4);

                    triangles[c++] = glm::vec2(x+sx, y+sy);
                    triangles[c++] = glm::vec2(x3, y3);
                    triangles[c++] = glm::vec2(x2, y2);
                }

            } else if (flags==10) {
#pragma omp critical
                {
                    triangles[c++] = glm::vec2(x, y+sy);
                    triangles[c++] = glm::vec2(x4, y4);
                    triangles[c++] = glm::vec2(x3, y3);

                    triangles[c++] = glm::vec2(x+sx, y);
                    triangles[c++] = glm::vec2(x2, y2);
                    triangles[c++] = glm::vec2(x1, y1);
                }
            }

        } else if (flags==15) {
            // Full fill
            float sx = dX, sy = dY;
            
#pragma omp critical
            {
                triangles[c++] = glm::vec2(x,    y);
                triangles[c++] = glm::vec2(x+sx, y+sy);
                triangles[c++] = glm::vec2(x,    y+sy);

                triangles[c++] = glm::vec2(x,    y);
                triangles[c++] = glm::vec2(x+sx, y);
                triangles[c++] = glm::vec2(x+sx, y+sy);
            }
        }
    }

    vbo_count = c;

    GLuint genbuf[1];
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for parallel contour fill";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, vbo_count * sizeof(glm::vec2), triangles.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel filled contour with " << c / 3 << " triangles" << std::endl;

    return true;
}

void ContourParallelFill::render(const glm::mat4& mvp,
                                 float zoom,
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
