#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourLine - level lines
 ****************************************************************************/
class ContourLine : public ContourPlot {
public:
    ContourLine(GLuint p);

    bool update(matrix_t* points, area_t a, double t);
    void render(const glm::mat4& mvp,
                double zoom,
                const glm::vec2& offset,
                const GLfloat c[]);
};
