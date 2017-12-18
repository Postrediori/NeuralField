#pragma once

#include "ContourPlot.h"

/*****************************************************************************
 * ContourParallel - level lines plot that uses OpenMP
 ****************************************************************************/
class ContourParallel : public ContourPlot {
public:
    ContourParallel(GLuint p);
    
    bool update(matrix_t* points, area_t a, double t);
    void render(const glm::mat4& mvp,
                double zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
