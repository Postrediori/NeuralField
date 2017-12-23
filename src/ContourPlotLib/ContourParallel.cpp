#include "stdafx.h"
#include "Matrix.h"
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

#pragma omp parallel for shared(lines)
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
                x1 = x;
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx * ValuesRatio(vals, 0, 1);
                y2 = y;
                break;
            case 2:
            case 13:
                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
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
    
#pragma omp critical
            {
                lines.emplace_back(x1, y1, x2, y2);
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
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
                break;
            case 6:
            case 9:
                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x+sx * ValuesRatio(vals, 3, 2);
                y2 = y+sy;
                break;
            }

#pragma omp critical
            {
                lines.emplace_back(x1, y1, x2, y2);
            }

        } else if (flags==5 || flags==10) {
            // Ambiguity
            double v;
            bool u;
            v = (points->data[(j  )*(xdiv+1)+(i  )]+points->data[(j  )*(xdiv+1)+(i+1)]+
                 points->data[(j+1)*(xdiv+1)+(i+1)]+points->data[(j+1)*(xdiv+1)+(i  )])/4.0;
            v -= threshold;
            u = v > 0.0;

            double x1, y1;
            double x2, y2;
            double x3, y3;
            double x4, y4;
            double sx = dX, sy = dY;

            if ((flags==5 && u) || (flags==10 && !u)) {
                x1 = x;
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx * ValuesRatio(vals, 3, 2);
                y2 = y+sy;

                x3 = x+sx * ValuesRatio(vals, 0, 1);
                y3 = y;
                x4 = x+sx;
                y4 = y+sy * ValuesRatio(vals, 1, 2);
            }

            if ((flags==5 && !u) || (flags==10 && u)) {
                x1 = x;
                y1 = y+sy * ValuesRatio(vals, 0, 3);
                x2 = x+sx * ValuesRatio(vals, 0, 1);
                y2 = y;

                x3 = x+sx * ValuesRatio(vals, 3, 2);
                y3 = y+sy;
                x4 = x+sx;
                y4 = y+sy * ValuesRatio(vals, 1, 2);
            }

#pragma omp critical
            {
                lines.emplace_back(x1, y1, x2, y2);
                lines.emplace_back(x3, y3, x4, y4);
            }
        }
    }

    vbo_count = lines.size() * 2;

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * lines.size(), lines.data(),
                 GL_DYNAMIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel line contour with " << lines.size() << " lines";

    return true;
}

void ContourParallel::render(const glm::mat4& mvp,
                             double zoom,
                             const glm::vec2& offset,
                             const GLfloat c[]) {
    glUseProgram(program); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, (GLfloat)zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, glm::value_ptr(offset)); LOGOPENGLERROR();
    glUniform2f(u_res, (GLfloat)w, (GLfloat)h); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c); LOGOPENGLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glEnableVertexAttribArray(a_coord); LOGOPENGLERROR();
    glVertexAttribPointer(a_coord, 2, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    glDrawArrays(GL_LINES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
}