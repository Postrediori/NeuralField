#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourParallel.h"

ContourParallel::ContourParallel(GLuint p)
    : ContourPlot(p) {
}

bool ContourParallel::init(matrix_t* points, area_t a, double t) {
    threshold = t;
    vbo_count = 0;
    vbo = 0;
    
    area = a;
    
    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;

    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    //unsigned char flags, u;
    //float x, y;
    //float vals[4];
    //float v;

    int c = 0;
    int max_idx = xdiv * ydiv;

    lines_t lines;
    lines.resize(max_idx * 2);

#pragma omp parallel for shared(lines)
    for (int idx=0; idx<max_idx; idx++) {
        int i = idx % xdiv;
        int j = idx / xdiv;

        double y = a.ymin + j * dY;
        double x = a.xmin + i * dX;
        
        float vals[4];
        vals[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
        vals[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
        vals[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
        vals[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;

        //unsigned char flags;
        //flags = CellType(vals);
        flags_t flags = FLAG_NO;
        if (vals[0]>0.f) flags |= FLAG_SW;
        if (vals[1]>0.f) flags |= FLAG_NW;
        if (vals[2]>0.f) flags |= FLAG_NE;
        if (vals[3]>0.f) flags |= FLAG_SE;

        if (flags==1 || flags==2 || flags==4 || flags==7
            || flags==8 || flags==11 || flags==13 || flags==14) {
            // One corner
            double x1, y1;
            double x2, y2;
            double sx = dX, sy = dY;

            switch (flags) {
            case 1:
            case 14:
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y2 = y;
                break;
            case 2:
            case 13:
                x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y1 = y;
                x2 = x+sx;
                y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
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
    
#pragma omp critical
            {
                lines[c++] = glm::vec4(x1, y1, x2, y2);
            }

        } else if (flags==3 || flags==6 || flags==9 || flags==12) {
            // Half
            double x1, y1;
            double x2, y2;
            double sx = dX, sy = dY;

            switch (flags) {
            case 3:
            case 12:
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx;
                y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
                break;
            case 6:
            case 9:
                x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y1 = y;
                x2 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y2 = y+sy;
                break;
            }

#pragma omp critical
            {
                lines[c++] = glm::vec4(x1, y1, x2, y2);
            }

        } else if (flags==5 || flags==10) {
            // Ambiguity
            double v;
            bool u;
            v = (points->data[(j  )*(xdiv+1)+(i  )]+points->data[(j  )*(xdiv+1)+(i+1)]+
                 points->data[(j+1)*(xdiv+1)+(i+1)]+points->data[(j+1)*(xdiv+1)+(i  )])/4.f;
            v -= threshold;
            u = v>0.f;

            double x1, y1;
            double x2, y2;
            double x3, y3;
            double x4, y4;
            double sx = dX, sy = dY;

            if ((flags==5 && u) || (flags==10 && !u)) {
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y2 = y+sy;

                x3 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y3 = y;
                x4 = x+sx;
                y4 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
            }

            if ((flags==5 && !u) || (flags==10 && u)) {
                x1 = x;
                y1 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));
                x2 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
                y2 = y;

                x3 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
                y3 = y+sy;
                x4 = x+sx;
                y4 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
            }

#pragma omp critical
            {
                lines[c++] = glm::vec4(x1, y1, x2, y2);
                lines[c++] = glm::vec4(x3, y3, x4, y4);
            }
        }
    }

    vbo_count = c * 2;

    GLuint genbuf[1];
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for parallel contour plot";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, c * sizeof(glm::vec4), lines.data(), GL_DYNAMIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel line contour with " << c << " lines";

    return true;
}

void ContourParallel::render(const glm::mat4& mvp,
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

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();
}
