#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourLine - level lines
 ****************************************************************************/
class ContourLine : public ContourPlot {
public:
    ContourLine(GLuint p);

    bool update(matrix_t* points, area_t a, double t);
    void render(const Math::mat4f& mvp,
                double zoom,
                const Math::vec2f& offset,
                const GLfloat c[]);
};
