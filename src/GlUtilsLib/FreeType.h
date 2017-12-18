#pragma once

#include <map>
#include "GlUtils.h"
#include "Shader.h"

typedef int FontSize_t;

/*****************************************************************************
 * FontAtlas
 ****************************************************************************/

static const size_t g_characterCount = 128;
static const int g_maxWidth = 1024;

struct CharInfo_t {
    float ax, ay; // advance x and y
    float bw, bh; // bitmap width and height
    float bl, bt; // bitmap left and top
    float tx, ty; // x and y offsets in texture coords
};

class FontAtlas {
public:
    FontAtlas(FT_Face face, FontSize_t height);
    ~FontAtlas();
    
public:
    GLuint tex;
    int w, h;
    CharInfo_t characters[g_characterCount];
};

/*****************************************************************************
 * FontRenderer
 ****************************************************************************/
struct FontArea_t {
    float x, y;
    float sx, sy;
};

typedef std::unique_ptr<FontAtlas> FontAtlasGuard_t;
typedef std::map<FontSize_t, FontAtlasGuard_t> Fonts_t;

class FontRenderer {
public:
    FontRenderer();
    
    bool init();
    bool init(const char* vertex_shader, const char* fragment_shader);
    bool load(const char* filename);
    void createAtlas(int height);

    void release();

    void renderStart();
    void renderEnd();
    void renderColor(const GLfloat* c);
    void renderText(FontSize_t size, FontArea_t area, const std::string& str);

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
    
    Fonts_t fonts;
};
