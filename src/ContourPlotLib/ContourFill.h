#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourParallelFill - level filles plot that uses OpenMP
 ****************************************************************************/
class ContourFill : public ContourPlot {
public:
    ContourFill() = default;
    
    bool Update(matrix_t* points, const hmm_vec4& area, double t);
    void Render(const hmm_mat4& mvp,
                double zoom,
                const hmm_vec2& offset,
                const FloatColor& c);
};
