#pragma once

class ContourPlot;

/*****************************************************************************
 * ContourParallel - level lines plot that uses OpenMP
 ****************************************************************************/
class ContourLine : public ContourPlot {
public:
    ContourLine() = default;
    
    bool Update(matrix_t* points, const HMM_Vec4& area, double t);
    void Render(const HMM_Mat4& mvp,
                double zoom,
                const HMM_Vec2& offset,
                const FloatColor& c);
};
