#pragma once

/*
 * Flags for corners of a marching square
 */
enum class SquareFlags : int {
    None = 0,

    SouthWest = (1 << 1),
    NorthWest = (1 << 2),
    NorthEast = (1 << 3),
    SouthEast = (1 << 4),

    All = (SouthEast | NorthEast | NorthWest | SouthWest)
};

SquareFlags operator|(SquareFlags lhs, SquareFlags rhs);
SquareFlags& operator|=(SquareFlags& lhs, SquareFlags rhs);
SquareFlags operator^(SquareFlags lhs, SquareFlags rhs);
SquareFlags& operator^=(SquareFlags& lhs, SquareFlags rhs);

/*
 * Structure with values of a marching cube.
 * Numeration is clock-wise from the bottom left corner of a square:
 * (1)---(2)
 *  |     |
 *  |     |
 * (0)---(3)
 */
struct vals_t {
    double v[4];
};

/*
 * Structure with coordinate and size of a marching square
 */
struct discrete_t {
    double x;
    double y;
    double sx;
    double sy;
};

/*
 * Calculate marching square flags from its values
 */
SquareFlags CellType(vals_t vals);

/*
 * Linear interpolation between values of nodes i1 and i2 of a marching square
 */
double ValuesRatio(vals_t vals, size_t i1, size_t i2);

using triangles_t = std::vector<hmm_vec2>;
using lines_t = std::vector<hmm_vec4>;

/*****************************************************************************
 * Contour Plot base class
 ****************************************************************************/
class ContourPlot {
public:
    ContourPlot() = default;
    virtual ~ContourPlot();

    bool Init(GLuint p);
    virtual bool Update(matrix_t* /*points*/, const hmm_vec4& /*area*/, double /*t*/) { return false; }
    virtual void Render(const hmm_mat4& /*mvp*/, double /*zoom*/, const hmm_vec2& /*offset*/,
    	const FloatColor& /*c*/) { }

    void Release();
    void Resize(int width, int height);

protected:
    int w = 0, h = 0;
    hmm_vec4 area = { 0.0, 0.0, 0.0, 0.0 };

    double threshold = 0.0;

    int vbo_count = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLint u_mvp = -1, u_zoom = -1, u_ofs = -1, u_res = -1, u_color = -1;
};
