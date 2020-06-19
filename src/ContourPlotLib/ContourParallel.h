#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourParallel - level lines plot that uses OpenMP
 ****************************************************************************/
class ContourParallel : public ContourPlot {
public:
    ContourParallel(GLuint p);
    
    bool update(matrix_t* points, area_t a, double t);
    void render(const Math::mat4f& mvp,
                double zoom,
                const Math::vec2f& offset,
                const GLfloat c[]);
};
