#pragma once

#define FLAG_NO 0x00
#define FLAG_SW  0x01
#define FLAG_NW  0x02
#define FLAG_NE  0x04
#define FLAG_SE  0x08

unsigned char CellType(float vals[]);

/*****************************************************************************
 * ContourPlot
 ****************************************************************************/
class ContourPlot {
public:
    ContourPlot(GLuint p);
    virtual ~ContourPlot();

    virtual bool init(const float* const points,
                      int xdiv, int ydiv,
                      float xmn, float xmx, float ymn, float ymx,
                      float t);
    virtual void render(const glm::mat4& mvp, float zoom,
                        const glm::vec2& offset, const GLfloat c[]);

    void release();
    void resize(int width, int height);

protected:
    int w, h;
    float xmin, xmax, ymin, ymax;

    float threshold;

    int vbo_count;
    GLuint vbo;
    GLuint program;
    GLint a_coord, u_mvp, u_zoom, u_ofs, u_res, u_color;
};
