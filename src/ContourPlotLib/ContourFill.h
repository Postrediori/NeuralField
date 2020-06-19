#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourFill
 ****************************************************************************/
class ContourFill : public ContourPlot {
public:
    ContourFill(GLuint p);

    bool update(matrix_t* points, area_t a, double t);
    void render(const Math::mat4f& mvp,
                double zoom,
                const Math::vec2f& offset,
                const GLfloat c[]);
};
