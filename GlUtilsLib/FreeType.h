#pragma once

#include "GlUtils.h"
#include "Shader.h"

/*****************************************************************************
 * FontAtlas
 ****************************************************************************/

//const int CharacterCount = 128;
//const int MaxWidth = 1024;
#define CharacterCount 128
#define MaxWidth 1024

struct CHARINFO {
    float ax, ay; // advance x and y
    float bw, bh; // bitmap width and height
    float bl, bt; // bitmap left and top
    float tx, ty; // x and y offsets in texture coords
};

class FontAtlas {
public:
    FontAtlas(FT_Face face, const int height);
    ~FontAtlas();
    
public:
    GLuint tex;
    int w, h;
    CHARINFO characters[CharacterCount];
};

/*****************************************************************************
 * FontRenderer
 ****************************************************************************/
struct COORD2D {
    GLfloat x, y;
    GLfloat s, t;
};

class FontRenderer {
public:
    FontRenderer();
    
    bool init();
    bool init(const char* vertex_shader, const char* fragment_shader);
    bool load(const char* filename);
    FontAtlas* createAtlas(const int height);

    void release();

    void renderStart();
    void renderEnd();
    void renderColor(const GLfloat* c);
    void renderText(const FontAtlas* a,
                    const float textx, const float texty,
                    const float sx, const float sy,
                    const char* fmt, ...);

private:
    bool init_renderer();
    bool init_shader();

public:
    GLuint vbo;

    GLint aCoord;
    GLint uTex, uColor;

    ShaderProgram program;
    FT_Library ft;
    FT_Face face;
};
