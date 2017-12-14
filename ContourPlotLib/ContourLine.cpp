#include "stdafx.h"
#include "GlUtils.h"
#include "ContourPlot.h"
#include "ContourLine.h"

void MakeCorner(lines_t& lines, flags_t flags, discrete_t d, double vals[]);
void MakeHalf(lines_t& lines, flags_t flags, discrete_t d, double vals[]);
void MakeAmbiguity(lines_t& lines, flags_t flags, bool u, discrete_t d, double vals[], double v);

ContourLine::ContourLine(GLuint p)
    : ContourPlot(p) {
}

bool ContourLine::update(matrix_t* points, area_t a, double t) {
    threshold = t;
    
    area = a;

    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (a.xmax - a.xmin) / (double)xdiv;
    double dY = (a.ymax - a.ymin) / (double)ydiv;

    flags_t flags;
    bool u;
    double vals[4];
    double v;
    discrete_t d;

    lines_t lines;

    for (int j=0; j<ydiv; j++) {
        double y = a.ymin + j * dY;
        for (int i=0; i<xdiv; i++) {
            double x = a.xmin + i * dX;
            d = {x, y, dX, dY};
            vals[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
            vals[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;
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
                MakeCorner(lines, flags, d, vals);
                break;

            case 3:
            case 6:
            case 9:
            case 12:
                // Half
                MakeHalf(lines, flags, d, vals);
                break;

            case 5:
            case 10:
                // Ambiguity
                v = (points->data[(j  )*(xdiv+1)+(i  )]+points->data[(j  )*(xdiv+1)+(i+1)]+
                     points->data[(j+1)*(xdiv+1)+(i+1)]+points->data[(j+1)*(xdiv+1)+(i  )])/4.f;
                v -= threshold;
                u = v>0.f;
                MakeAmbiguity(lines, flags, u, d, vals, v);
                break;

            case 0:
            case 15:
            default:
                // No lines
                break;
            }
        }
    }

    vbo_count = lines.size() * 2;

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * lines.size(),
        lines.data(), GL_DYNAMIC_DRAW); LOGOPENGLERROR();
    
    LOGD << "Created filled contour with " << lines.size() / 2 << " lines";
    
    return true;
}

void ContourLine::render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
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

void MakeCorner(lines_t& lines, flags_t flags,
                discrete_t d, double vals[]) {
    double x1, y1;
    double x2, y2;

    switch (flags) {
    case 1:
    case 14:
        x1 = d.x;
        y1 = d.y+d.sy*fabs(vals[0]/(vals[0]-vals[3]));
        x2 = d.x+d.sx*fabs(vals[0]/(vals[0]-vals[1]));
        y2 = d.y;
        break;
    case 2:
    case 13:
        x1 = d.x+d.sx*fabs(vals[0]/(vals[0]-vals[1]));
        y1 = d.y;
        x2 = d.x+d.sx;
        y2 = d.y+d.sy*fabs(vals[1]/(vals[1]-vals[2]));
        break;
    case 4:
    case 11:
        x1 = d.x+d.sx*fabs(vals[3]/(vals[3]-vals[2]));
        y1 = d.y+d.sy;
        x2 = d.x+d.sx;
        y2 = d.y+d.sy*fabs(vals[1]/(vals[1]-vals[2]));
        break;
    case 7:
    case 8:
        x1 = d.x;
        y1 = d.y+d.sy*fabs(vals[0]/(vals[0]-vals[3]));
        x2 = d.x+d.sx*fabs(vals[3]/(vals[3]-vals[2]));
        y2 = d.y+d.sy;
        break;
    }

    lines.emplace_back(x1, y1, x2, y2);
}

void MakeHalf(lines_t& lines, flags_t flags,
              discrete_t d, double vals[]) {
    double x1, y1;
    double x2, y2;

    switch (flags) {
    case 3:
    case 12:
        x1 = d.x;
        y1 = d.y+d.sy*fabs(vals[0]/(vals[0]-vals[3]));
        x2 = d.x+d.sx;
        y2 = d.y+d.sy*fabs(vals[1]/(vals[1]-vals[2]));
        break;
        
    case 6:
    case 9:
        x1 = d.x+d.sx*fabs(vals[0]/(vals[0]-vals[1]));
        y1 = d.y;
        x2 = d.x+d.sx*fabs(vals[3]/(vals[3]-vals[2]));
        y2 = d.y+d.sy;
        break;
    }

    lines.emplace_back(x1, y1, x2, y2);
}

void MakeAmbiguity(lines_t& lines, flags_t flags, bool u,
                   discrete_t d, double vals[], double v) {
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double x4, y4;

    if ((flags==5 && u) || (flags==10 && !u)) {
        x1 = d.x;
        y1 = d.y+d.sy*fabs(vals[0]/(vals[0]-vals[3]));
        x2 = d.x+d.sx*fabs(vals[3]/(vals[3]-vals[2]));
        y2 = d.y+d.sy;

        x3 = d.x+d.sx*fabs(vals[0]/(vals[0]-vals[1]));
        y3 = d.y;
        x4 = d.x+d.sx;
        y4 = d.y+d.sy*fabs(vals[1]/(vals[1]-vals[2]));
    }

    if ((flags==5 && !u) || (flags==10 && u)) {
        x1 = d.x;
        y1 = d.y+d.sy*fabs(vals[0]/(vals[0]-vals[3]));
        x2 = d.x+d.sx*fabs(vals[0]/(vals[0]-vals[1]));
        y2 = d.y;

        x3 = d.x+d.sx*fabs(vals[3]/(vals[3]-vals[2]));
        y3 = d.y+d.sy;
        x4 = d.x+d.sx;
        y4 = d.y+d.sy*fabs(vals[1]/(vals[1]-vals[2]));
    }

    lines.emplace_back(x1, y1, x2, y2);
    lines.emplace_back(x3, y3, x4, y4);
}
