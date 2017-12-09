#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourFill.h"

void MakeFill(std::vector<glm::vec2>& triangles, unsigned char flags,
              float x, float y, float sx, float sy, float vals[]);

void MakeFillCorner(std::vector<glm::vec2>& triangles, unsigned char flags,
                    float x, float y, float sx, float sy, float vals[]);

void MakeFillHalf(std::vector<glm::vec2>& triangles, unsigned char flags,
                  float x, float y, float sx, float sy, float vals[]);

void MakeFillAmbiguity(std::vector<glm::vec2>& triangles, unsigned char flags, unsigned char u,
                       float x, float y, float sx, float sy, float vals[], float v);

ContourFill::ContourFill(GLuint p)
    : ContourPlot(p) {
}

bool ContourFill::init(const float* const points,
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

    unsigned char flags, u;
    float x, y;
    float vals[4];
    float v;

    std::vector<glm::vec2> triangles;

    for (int j=0; j<ydiv; j++) {
        y = ymin + j * dY;
        for (int i=0; i<xdiv; i++) {
            x = xmin + i * dX;
            vals[0] = points[(j  )*(xdiv+1)+(i  )] - threshold;
            vals[1] = points[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals[2] = points[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals[3] = points[(j+1)*(xdiv+1)+(i  )] - threshold;
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
                MakeFillCorner(triangles, flags, x, y, dX, dY, vals);
                break;

            case 3:
            case 6:
            case 9:
            case 12:
                // Half
                MakeFillHalf(triangles, flags, x, y, dX, dY, vals);
                break;

            case 5:
            case 10:
                // Ambiguity
                v = (points[(j  )*(xdiv+1)+(i  )]+points[(j  )*(xdiv+1)+(i+1)]+
                     points[(j+1)*(xdiv+1)+(i+1)]+points[(j+1)*(xdiv+1)+(i  )])/4.f;
                v -= threshold;
                u = v>0.f;
                MakeFillAmbiguity(triangles, flags, u, x, y, dX, dY, vals, v);
                break;

            case 15:
                // Full fill
                MakeFill(triangles, flags, x, y, dX, dY, vals);
                break;

            case 0:
            default:
                // No lines
                break;
            }
        }
    }

    vbo_count = triangles.size();

    GLuint genbuf[1];
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for contour fill";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vbo_count, triangles.data(), GL_STATIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created outline contour with " << triangles.size() / 3 << " triangles";
    
    return true;
}

void ContourFill::render(const glm::mat4& mvp, float zoom, const glm::vec2& offset,
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

void MakeFill(std::vector<glm::vec2>& triangles, unsigned char flags,
              float x, float y, float sx, float sy, float vals[]) {
    triangles.push_back(glm::vec2(x,    y));
    triangles.push_back(glm::vec2(x+sx, y+sy));
    triangles.push_back(glm::vec2(x,    y+sy));

    triangles.push_back(glm::vec2(x,    y));
    triangles.push_back(glm::vec2(x+sx, y));
    triangles.push_back(glm::vec2(x+sx, y+sy));
}

void MakeFillCorner(std::vector<glm::vec2>& triangles, unsigned char flags,
                    float x, float y, float sx, float sy, float vals[]) {
    float x1, y1;
    float x2, y2;

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
        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==14) {
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x+sx, y+sy));

        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x+sx, y+sy));
        triangles.push_back(glm::vec2(x2, y2));

        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x+sx, y+sy));
        triangles.push_back(glm::vec2(x, y+sy));

    } else if (flags==2) {
        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==13) {
        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x, y+sy));

        triangles.push_back(glm::vec2(x, y+sy));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x1, y1));

        triangles.push_back(glm::vec2(x, y+sy));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x+sx, y+sy));

    } else if (flags==4) {
        triangles.push_back(glm::vec2(x+sx, y+sy));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==11) {
        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x2, y2));

        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x1, y1));

        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x, y+sy));

    } else if (flags==8) {
        triangles.push_back(glm::vec2(x, y+sy));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==7) {
        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x1, y1));

        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x2, y2));

        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x+sx, y+sy));
        triangles.push_back(glm::vec2(x2, y2));
    }
}

void MakeFillHalf(std::vector<glm::vec2>& triangles, unsigned char flags,
                  float x, float y, float sx, float sy, float vals[]) {
    float x1, y1;
    float x2, y2;
    float x3, y3;
    float x4, y4;

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

    triangles.push_back(glm::vec2(x1, y1));
    triangles.push_back(glm::vec2(x3, y3));
    triangles.push_back(glm::vec2(x4, y4));

    triangles.push_back(glm::vec2(x1, y1));
    triangles.push_back(glm::vec2(x2, y2));
    triangles.push_back(glm::vec2(x3, y3));
}

void MakeFillAmbiguity(std::vector<glm::vec2>& triangles, unsigned char flags, unsigned char u,
                       float x, float y, float sx, float sy, float vals[], float v) {
    float x1, y1;
    float x2, y2;
    float x3, y3;
    float x4, y4;

    x1 = x+sx*fabs(vals[0]/(vals[0]-vals[1]));
    y1 = y;
    x2 = x+sx;
    y2 = y+sy*fabs(vals[1]/(vals[1]-vals[2]));
    x3 = x+sx*fabs(vals[3]/(vals[3]-vals[2]));
    y3 = y+sy;
    x4 = x;
    y4 = y+sy*fabs(vals[0]/(vals[0]-vals[3]));

    if (u) {
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x3, y3));

        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x3, y3));
        triangles.push_back(glm::vec2(x4, y4));
    }

    if (flags==5) {
        triangles.push_back(glm::vec2(x, y));
        triangles.push_back(glm::vec2(x1, y1));
        triangles.push_back(glm::vec2(x4, y4));

        triangles.push_back(glm::vec2(x+sx, y+sy));
        triangles.push_back(glm::vec2(x3, y3));
        triangles.push_back(glm::vec2(x2, y2));

    } else if (flags==10) {
        triangles.push_back(glm::vec2(x, y+sy));
        triangles.push_back(glm::vec2(x4, y4));
        triangles.push_back(glm::vec2(x3, y3));

        triangles.push_back(glm::vec2(x+sx, y));
        triangles.push_back(glm::vec2(x2, y2));
        triangles.push_back(glm::vec2(x1, y1));
    }
}
