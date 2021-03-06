#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourLine - level lines
 ****************************************************************************/
class ContourLine : public ContourPlot {
public:
    ContourLine();

    bool update(matrix_t* points, area_t a, double t);
    void render(const glm::mat4& mvp,
                double zoom,
                const glm::vec2& offset,
                const std::array<GLfloat, 4>& c);
};
