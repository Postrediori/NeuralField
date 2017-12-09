#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourLine.h"

void MakeCorner(std::vector<glm::vec2>& lines, unsigned char flags,
                float x, float y, float sx, float sy, float vals[]);

void MakeHalf(std::vector<glm::vec2>& lines, unsigned char flags,
              float x, float y, float sx, float sy, float vals[]);

void MakeAmbiguity(std::vector<glm::vec2>& lines, unsigned char flags, unsigned char u,
                   float x, float y, float sx, float sy, float vals[], float v);

ContourLine::ContourLine(GLuint p)
    : ContourPlot(p) {
}

bool ContourLine::init(const float* const points,
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

    std::vector<glm::vec2> lines;

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
                MakeCorner(lines, flags, x, y, dX, dY, vals);
                break;

            case 3:
            case 6:
            case 9:
            case 12:
                // Half
                MakeHalf(lines, flags, x, y, dX, dY, vals);
                break;

            case 5:
            case 10:
                // Ambiguity
                v = (points[(j  )*(xdiv+1)+(i  )]+points[(j  )*(xdiv+1)+(i+1)]+
                     points[(j+1)*(xdiv+1)+(i+1)]+points[(j+1)*(xdiv+1)+(i  )])/4.f;
                v -= threshold;
                u = v>0.f;
                MakeAmbiguity(lines, flags, u, x, y, dX, dY, vals, v);
                break;

            case 0:
            case 15:
            default:
                // No lines
                break;
            }
        }
    }

    vbo_count = lines.size();

    GLuint genbuf[1];
    glGenBuffers(1, genbuf); LOGOPENGLERROR();
    vbo = genbuf[0];
    if (!vbo) {
        LOGE << "Unable to initialize VBO for contour line";
        return false;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vbo_count, lines.data(), GL_STATIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created filled contour with " << lines.size() / 2 << " lines";
    
    return true;
}

void ContourLine::render(const glm::mat4& mvp, float zoom, const glm::vec2& offset,
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

void MakeCorner(std::vector<glm::vec2>& lines, unsigned char flags,
                float x, float y, float sx, float sy, float vals[]) {
    float x1, y1;
    float x2, y2;

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

    lines.push_back(glm::vec2(x1, y1));
    lines.push_back(glm::vec2(x2, y2));
}

void MakeHalf(std::vector<glm::vec2>& lines, unsigned char flags,
              float x, float y, float sx, float sy, float vals[]) {
    float x1, y1;
    float x2, y2;

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

    lines.push_back(glm::vec2(x1, y1));
    lines.push_back(glm::vec2(x2, y2));
}

void MakeAmbiguity(std::vector<glm::vec2>& lines, unsigned char flags, unsigned char u,
                   float x, float y, float sx, float sy, float vals[], float v) {
    float x1, y1;
    float x2, y2;
    float x3, y3;
    float x4, y4;

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

    lines.push_back(glm::vec2(x1, y1));
    lines.push_back(glm::vec2(x2, y2));
    lines.push_back(glm::vec2(x3, y3));
    lines.push_back(glm::vec2(x4, y4));
}
