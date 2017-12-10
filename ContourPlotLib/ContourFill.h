#pragma once

#include "ContourPlot.h"

/*****************************************************************************
 * ContourFill
 ****************************************************************************/
class ContourFill : public ContourPlot {
public:
    ContourFill(GLuint p);

    bool update(matrix_t* points, area_t a, double t);
    void render(const glm::mat4& mvp,
                double zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
