#pragma once

typedef unsigned char flags_t;
#define FLAG_NO (flags_t)(0x00)
#define FLAG_SW (flags_t)(0x01)
#define FLAG_NW (flags_t)(0x02)
#define FLAG_NE (flags_t)(0x04)
#define FLAG_SE (flags_t)(0x08)

/*
 * (1)---(2)
 *  |     |
 *  |     |
 * (0)---(3)
 */

struct vals_t {
    double v[4];
};

flags_t CellType(vals_t vals);
double ValuesRatio(vals_t vals, size_t i1, size_t i2);

struct area_t {
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};


struct discrete_t {
    double x;
    double y;
    double sx;
    double sy;
};


typedef std::vector<glm::vec2> triangles_t;
typedef std::vector<glm::vec4> lines_t;

/*****************************************************************************
 * ContourPlot
 ****************************************************************************/
class ContourPlot {
public:
    ContourPlot(GLuint p);
    virtual ~ContourPlot();

    bool init();
    virtual bool update(matrix_t* points, area_t a, double t);
    virtual void render(const glm::mat4& mvp, double zoom, const glm::vec2& offset,
                        const GLfloat c[]);

    void release();
    void resize(int width, int height);

protected:
    int w, h;
    area_t area;

    double threshold;

    int vbo_count;
    GLuint vbo;
    GLuint program;
    GLint a_coord, u_mvp, u_zoom, u_ofs, u_res, u_color;
};

typedef std::unique_ptr<ContourPlot> ContourPlotGuard_t;
