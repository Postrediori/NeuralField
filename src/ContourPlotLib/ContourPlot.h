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

/*
 * Helper structure that describes an area of a marching square
 */
struct area_t {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

typedef std::vector<Math::vec2f> triangles_t;
typedef std::vector<Math::vec4f> lines_t;

/*****************************************************************************
 * Contour Plot base class
 ****************************************************************************/
class ContourPlot {
public:
    ContourPlot();
    virtual ~ContourPlot();

    bool init(GLuint p);
    virtual bool update(matrix_t* points, area_t a, double t);
    virtual void render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
                        const std::array<GLfloat, 4>& c);

    void release();
    void resize(int width, int height);

protected:
    int w = 0, h = 0;
    area_t area;

    double threshold = 0.0;

    int vbo_count = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint program = 0;
    GLint u_mvp = -1, u_zoom = -1, u_ofs = -1, u_res = -1, u_color = -1;
};
