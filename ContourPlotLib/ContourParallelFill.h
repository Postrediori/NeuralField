#pragma once

#include "ContourPlot.h"

/*****************************************************************************
 * ContourParallelFill - level filles plot that uses OpenMP
 ****************************************************************************/
class ContourParallelFill : public ContourPlot {
public:
    ContourParallelFill(GLuint p);
    
    bool update(matrix_t* points, area_t a, double t);
    void render(const glm::mat4& mvp,
                double zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
