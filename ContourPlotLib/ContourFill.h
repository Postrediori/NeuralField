#pragma once
//#include "ContourPlot.h"

class ContourPlot;

/*****************************************************************************
 * ContourFill
 ****************************************************************************/
class ContourFill : public ContourPlot {
public:
    ContourFill(GLuint p);

    bool init(const float* const points,
              int xdiv, int ydiv,
              float xmn, float xmx, float ymn, float ymx,
              float t);
    void render(const glm::mat4& mvp, float zoom, const glm::vec2& offset,
                const GLfloat c[]);
};
