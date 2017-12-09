#pragma once

// #include "ContourPlot.h"
class ContourPlot;

/*****************************************************************************
 * ContourLine - level lines
 ****************************************************************************/
class ContourLine : public ContourPlot {
public:
    ContourLine(GLuint p);

    bool init(const float* const points,
              int xdiv, int ydiv,
              float xmn, float xmx, float ymn, float ymx,
              float t);
    void render(const glm::mat4& mvp, float zoom, const glm::vec2& offset,
                const GLfloat c[]);
};
