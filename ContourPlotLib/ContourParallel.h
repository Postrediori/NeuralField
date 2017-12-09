#pragma once
//#include "ContourPlot.h"
class ContourPlot;

/*****************************************************************************
 * ContourParallel - level lines plot that uses OpenMP
 ****************************************************************************/
class ContourParallel : public ContourPlot {
public:
    ContourParallel(GLuint p);
    
    bool init(const float* const points,
              int xdiv, int ydiv,
              float xmn, float xmx, float ymn, float ymx,
              float t);
    void render(const glm::mat4& mvp,
                float zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
