#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourParallelFill - level filles plot that uses OpenMP
 ****************************************************************************/
class ContourParallelFill : public ContourPlot {
public:
    ContourParallelFill(GLuint p);
    
    bool update(matrix_t* points, area_t a, double t);
    void render(const Math::mat4f& mvp,
                double zoom,
                const Math::vec2f& offset,
                const GLfloat c[]);
};
