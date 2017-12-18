#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourFill.h"

void MakeFill(triangles_t& triangles, discrete_t d);
void MakeFillCorner(triangles_t& triangles, flags_t flags, discrete_t d, vals_t vals);
void MakeFillHalf(triangles_t& triangles, flags_t flags, discrete_t d, vals_t vals);
void MakeFillAmbiguity(triangles_t& triangles, flags_t flags, bool u, discrete_t d, vals_t vals, double v);

ContourFill::ContourFill(GLuint p)
    : ContourPlot(p) {
}

bool ContourFill::update(matrix_t* points, area_t a, double t) {
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

    triangles_t triangles;

    for (int j=0; j<ydiv; j++) {
        double y = a.ymin + j * dY;
        for (int i=0; i<xdiv; i++) {
            double x = a.xmin + i * dX;
            discrete_t d = {x, y, dX, dY};
            vals.v[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
            vals.v[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals.v[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals.v[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;
            flags = CellType(vals);

            switch (flags) {
            case 1:
            case 2:
            case 4:
            case 7:
            case 8:
            case 11:
            case 13:
            case 14:
                // One corner
                MakeFillCorner(triangles, flags, d, vals);
                break;

            case 3:
            case 6:
            case 9:
            case 12:
                // Half
                MakeFillHalf(triangles, flags, d, vals);
                break;

            case 5:
            case 10:
                // Ambiguity
                v = (points->data[(j  )*(xdiv+1)+(i  )]+points->data[(j  )*(xdiv+1)+(i+1)]+
                     points->data[(j+1)*(xdiv+1)+(i+1)]+points->data[(j+1)*(xdiv+1)+(i  )]) / 4.0;
                v -= threshold;
                u = v > 0.0;
                MakeFillAmbiguity(triangles, flags, u, d, vals, v);
                break;

            case 15:
                // Full fill
                MakeFill(triangles, d);
                break;

            case 0:
            default:
                // No lines
                break;
            }
        }
    }

    vbo_count = triangles.size();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * triangles.size(),
        triangles.data(), GL_DYNAMIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created outline contour with " << triangles.size() / 3 << " triangles";
    
    return true;
}

void ContourFill::render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
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

    glUseProgram(0); LOGOPENGLERROR();
}

void MakeFill(triangles_t& triangles, discrete_t d) {
    triangles.push_back(glm::vec2(d.x,      d.y));
    triangles.push_back(glm::vec2(d.x+d.sx, d.y+d.sy));
    triangles.push_back(glm::vec2(d.x,      d.y+d.sy));

    triangles.push_back(glm::vec2(d.x,      d.y));
    triangles.push_back(glm::vec2(d.x+d.sx, d.y));
    triangles.push_back(glm::vec2(d.x+d.sx, d.y+d.sy));
}

void MakeFillCorner(triangles_t& triangles, flags_t flags,
                    discrete_t d, vals_t vals) {
    double x1, y1;
    double x2, y2;

    switch (flags) {
    case 1:
    case 14:
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x;
        y2 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        break;

    case 2:
    case 13:
        x1 = d.x+d.sx;
        y1 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;
        break;

    case 4:
    case 11:
        x1 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y1 = d.y+d.sy;
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        break;

    case 7:
    case 8:
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y2 = d.y+d.sy;
        break;
    }
    
    glm::vec2 v(d.x, d.y), v1(x1, y1), v2(x2, y2);
    glm::vec2 sx(d.sx, 0), sy(0, d.sy), s(d.sx, d.sy);

    if (flags==1) {
        triangles.push_back(v);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags==14) {
        triangles.push_back(v1);
        triangles.push_back(v + sx);
        triangles.push_back(v + s);

        triangles.push_back(v1);
        triangles.push_back(v + s);
        triangles.push_back(v2);

        triangles.push_back(v2);
        triangles.push_back(v + s);
        triangles.push_back(v + sy);

    } else if (flags==2) {
        triangles.push_back(v + sx);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags==13) {
        triangles.push_back(v);
        triangles.push_back(v2);
        triangles.push_back(v + sy);

        triangles.push_back(v + sy);
        triangles.push_back(v2);
        triangles.push_back(v1);

        triangles.push_back(v + sy);
        triangles.push_back(v1);
        triangles.push_back(v + s);

    } else if (flags==4) {
        triangles.push_back(v + s);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags==11) {
        triangles.push_back(v);
        triangles.push_back(v + sx);
        triangles.push_back(v2);

        triangles.push_back(v);
        triangles.push_back(v2);
        triangles.push_back(v1);

        triangles.push_back(v);
        triangles.push_back(v1);
        triangles.push_back(v + sy);

    } else if (flags==8) {
        triangles.push_back(v + sy);
        triangles.push_back(v1);
        triangles.push_back(v2);

    } else if (flags==7) {
        triangles.push_back(v);
        triangles.push_back(v + sx);
        triangles.push_back(v1);

        triangles.push_back(v1);
        triangles.push_back(v + sx);
        triangles.push_back(v2);

        triangles.push_back(v + sx);
        triangles.push_back(v + s);
        triangles.push_back(v2);
    }
}

void MakeFillHalf(triangles_t& triangles, flags_t flags,
                  discrete_t d, vals_t vals) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    switch (flags) {
    case 3:
        x1 = d.x;
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y;
        x3 = d.x+d.sx;
        y3 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x4 = d.x;
        y4 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        break;

    case 12:
        x1 = d.x;
        y1 = d.y+d.sy * ValuesRatio(vals, 0, 3);
        x2 = d.x+d.sx;
        y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
        x3 = d.x+d.sx;
        y3 = d.y+d.sy;
        x4 = d.x;
        y4 = d.y+d.sy;
        break;

    case 6:
        x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y;
        x3 = d.x+d.sx;
        y3 = d.y+d.sy;
        x4 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y4 = d.y+d.sy;
        break;

    case 9:
        x1 = d.x;
        y1 = d.y;
        x2 = d.x+d.sx * ValuesRatio(vals, 0, 1);
        y2 = d.y;
        x3 = d.x+d.sx * ValuesRatio(vals, 3, 2);
        y3 = d.y+d.sy;
        x4 = d.x;
        y4 = d.y+d.sy;
        break;
    }

    triangles.push_back(glm::vec2(x1, y1));
    triangles.push_back(glm::vec2(x3, y3));
    triangles.push_back(glm::vec2(x4, y4));

    triangles.push_back(glm::vec2(x1, y1));
    triangles.push_back(glm::vec2(x2, y2));
    triangles.push_back(glm::vec2(x3, y3));
}

void MakeFillAmbiguity(triangles_t& triangles, flags_t flags, bool u,
                       discrete_t d, vals_t vals, double v) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    x1 = d.x+d.sx * ValuesRatio(vals, 0, 1);
    y1 = d.y;
    x2 = d.x+d.sx;
    y2 = d.y+d.sy * ValuesRatio(vals, 1, 2);
    x3 = d.x+d.sx * ValuesRatio(vals, 3, 2);
    y3 = d.y+d.sy;
    x4 = d.x;
    y4 = d.y+d.sy * ValuesRatio(vals, 0, 3);

    if (u) {
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x3, y3));

        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x3, y3));
        triangles.push_back(glm::vec2(x4, y4));
    }

    if (flags==5) {
        triangles.push_back(glm::vec2(d.x, d.y));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x4, y4));

        triangles.push_back(glm::vec2(d.x+d.sx, d.y+d.sy));
        triangles.push_back(glm::vec2(x3, y3));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==10) {
        triangles.push_back(glm::vec2(d.x, d.y+d.sy));
        triangles.push_back(glm::vec2(x4, y4));
        triangles.push_back(glm::vec2(x3, y3));

        triangles.push_back(glm::vec2(d.x+d.sx, d.y));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x1, y1));
    }
}
