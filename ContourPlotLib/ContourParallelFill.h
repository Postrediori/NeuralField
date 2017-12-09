#pragma once

//#include "ContourPlot.h"
class ContourPlot;

/*****************************************************************************
 * ContourParallelFill - level filles plot that uses OpenMP
 ****************************************************************************/
class ContourParallelFill : public ContourPlot {
public:
    ContourParallelFill(GLuint p);
    
    bool init(const float* const points,
              int xdiv, int ydiv,
              float xmn, float xmx, float ymn, float ymx,
              float t);
    void render(const glm::mat4& mvp,
                float zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
