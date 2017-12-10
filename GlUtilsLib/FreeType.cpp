#include "stdafx.h"
#include "GlUtils.h"
#include "Shader.h"
#include "FreeType.h"

struct Coord2d_t {
    GLfloat x, y;
    GLfloat s, t;
};

static const size_t g_firstCharacter = 32;

static const char vertex_src_1_30[] =
    "#version 130\n"
    "in vec4 coord;"
    "out vec2 tex_coord;"
    "void main(){"
    "    gl_Position=vec4(coord.xy,0.,1.);"
    "    tex_coord=coord.zw;"
    "}";

static const char fragment_src_1_30[] =
    "#version 130\n"
    "in vec2 tex_coord;"
    "out vec4 frag_color;"
    "uniform vec4 color;"
    "uniform sampler2D tex;"
    "void main(){"
    "    float a=texture(tex,tex_coord).r;"
    "    frag_color=vec4(1.,1.,1.,a)*color;"
    "}";

static const char vertex_src_1_10[] =
    "#version 110\n"
    "attribute vec4 coord;"
    "varying vec2 tex_coord;"
    "void main(){"
    "    gl_Position=vec4(coord.xy,0.,1.);"
    "    tex_coord=coord.zw;"
    "}";

static const char fragment_src_1_10[] =
    "#version 110\n"
    "varying vec2 tex_coord;"
    "uniform vec4 color;"
    "uniform sampler2D tex;"
    "void main(){"
    "    float a=texture2D(tex,tex_coord).r;"
    "    gl_FragColor=vec4(1.,1.,1.,a)*color;"
    "}";

static int max(int a, int b) {
    return (a>b) ? a : b;
}

/*****************************************************************************
 * FontAtlas
 ****************************************************************************/
 
FontAtlas::FontAtlas(FT_Face face, FontSize_t height) {
    LOGI << "Started font loading";
	
    FT_Set_Pixel_Sizes(face, 0, height);
    FT_GlyphSlot g = face->glyph;

    memset(characters, 0, sizeof(characters));

    w = 0;
    h = 0;
    int roww = 0, rowh = 0;

    // Find minimum size for a texture holding all visible ASCII characters
    for (int i = g_firstCharacter; i < g_characterCount; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            LOGE << "FreeType Error: Loading character " << i << " failed!";
            continue;
        }

        if (roww+g->bitmap.width + 1 >= g_maxWidth) {
            w = max(w, roww);
            h += rowh;
            roww = 0;
            rowh = 0;
        }

        roww += g->bitmap.width + 1;
        rowh = max(rowh, g->bitmap.rows);
    }

    w = max(w, roww);
    h += rowh;

    // Create texture for all ASCII glyphs
    glGenTextures(1, &tex); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, tex); LOGOPENGLERROR();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0); LOGOPENGLERROR();

    // 1-byte alignment required when uploading texture data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); LOGOPENGLERROR();

    // Clamping edges is important to prevent artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); LOGOPENGLERROR();

    // Linear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); LOGOPENGLERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); LOGOPENGLERROR();

    // Paste all glyph bitmaps into the texture, remembering offset
    int ox = 0, oy = 0;

    rowh = 0;

    for (int i = g_firstCharacter; i < g_characterCount; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            LOGE << "FreeType Error: Loading character " << i << " failed!";
            continue;
        }

        if ((ox + g->bitmap.width + 1) >= g_maxWidth) {
            oy += rowh;
            rowh = 0;
            ox = 0;
        }

        glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy,
                        g->bitmap.width, g->bitmap.rows,
                        GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
        LOGOPENGLERROR();

        CharInfo_t inf;
        inf.ax = g->advance.x >> 6;
        inf.ay = g->advance.y >> 6;

        inf.bw = g->bitmap.width;
        inf.bh = g->bitmap.rows;

        inf.bl = g->bitmap_left;
        inf.bt = g->bitmap_top;

        inf.tx = ox / (float)w;
        inf.ty = oy / (float)h;

        characters[i] = inf;

        rowh = max(rowh, g->bitmap.rows);
        ox += g->bitmap.width + 1;
    }

    LOGI << "FreeType: Generated a "
        << w << "x" << h
        << " ("<< (w*h/1024) << " kb) texture atlas";
}

FontAtlas::~FontAtlas() {
    glDeleteTextures(1, &tex); LOGOPENGLERROR();
}

/*****************************************************************************
 * FontRenderer
 ****************************************************************************/
FontRenderer::FontRenderer()
    : vbo(0)
    , aCoord(0)
    , uTex(0)
    , uColor(0) {
}

bool FontRenderer::init(const char* vertex_shader, const char* fragment_shader) {
    if (!init_renderer()) {
        return false;
    }

    // Init shader
    if(program.load(vertex_shader, fragment_shader) != 0) {
        LOGE << "Unable to load shader program from files";
        return false;
    }
    
    if (!init_shader()) {
        return false;
    }

    return true;
}

bool FontRenderer::init() {
    if (!init_renderer()) {
        return false;
    }

    // Init shader
    if (!program.load(vertex_src_1_30, fragment_src_1_30)) {
        LOGE << "Unable to load shader program from embedded source code";
        return false;
    }
    
    if (!init_shader()) {
        return false;
    }

    return true;
}

bool FontRenderer::init_renderer() {
    // Init FreeType
    if (FT_Init_FreeType(&ft) != 0) {
        LOGE << "Unable to initialize FreeType library";
        return false;
    }

    // Init VBO
    glGenBuffers(1, &vbo); LOGOPENGLERROR();
    if (!vbo) {
        LOGE << "Unable to allocate VBO";
        return false;
    }
    
    return true;
}

bool FontRenderer::init_shader() {
    aCoord = program.attrib("coord");
    uTex = program.uniform("tex");
    uColor = program.uniform("color");

    if (aCoord==-1 || uTex==-1 || uColor==-1) {
        LOGE << "Invalid shader program";
        return false;
    }
    
    return true;
}

bool FontRenderer::load(const char* filename) {
    if (FT_New_Face(ft, filename, 0, &face)) {
        LOGE << "Unable to load font from file " << filename;
        return false;
    }
    return true;
}

void FontRenderer::createAtlas(const int height) {
    fonts.emplace(std::make_pair(height, FontAtlasGuard_t(new FontAtlas(face, height))));
}

void FontRenderer::release() {
    // program.release();
    glDeleteBuffers(1, &vbo); LOGOPENGLERROR();
}

void FontRenderer::renderStart() {
    // Push GL_BLEND and Blending function
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND); LOGOPENGLERROR();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); LOGOPENGLERROR();
    glUseProgram(program.program()); LOGOPENGLERROR();
}

void FontRenderer::renderEnd() {
    glUseProgram(0); LOGOPENGLERROR();
    glPopAttrib(); LOGOPENGLERROR();
}

void FontRenderer::renderColor(const GLfloat c[]) {
    glUniform4fv(uColor, 1, c); LOGOPENGLERROR();
}

void FontRenderer::renderText(FontSize_t size, FontArea_t area, const std::string& str) {
    if (str.empty()) {
        return;
    }
    
    const auto& a = fonts[size];

    glActiveTexture(GL_TEXTURE0); LOGOPENGLERROR();
    glBindTexture(GL_TEXTURE_2D, a->tex); LOGOPENGLERROR();
    glUniform1i(uTex, 0); LOGOPENGLERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glEnableVertexAttribArray(aCoord); LOGOPENGLERROR();
    glVertexAttribPointer(aCoord, 4, GL_FLOAT, GL_FALSE, 0, 0); LOGOPENGLERROR();

    size_t len = str.length();
    std::vector<Coord2d_t> coords;
    coords.reserve(6 * len);
    int c = 0;

    // Loop through all characters
    GLfloat tdx, tdy;
    float w, h;
    float x2, y2;
    float x = area.x, y = area.y;
    for (const auto& c : str) {
        CharInfo_t inf = a->characters[c];

        // Calculate vertex and texture coordinates
        x2 = x + inf.bl * area.sx;
        y2 = -y - inf.bt * area.sy;
        w = inf.bw * area.sx;
        h = inf.bh * area.sy;

        // Advance the cursor to the start of the next character
        x += inf.ax * area.sx;
        y += inf.ay * area.sy;

        // Skip glyphs that have no pixels
        if (!w || !h) {
            continue;
        }

        tdx = inf.bw/(float)a->w;
        tdy = inf.bh/(float)a->h;

        coords.push_back({x2 + w, -y2, inf.tx + tdx, inf.ty});
        coords.push_back({x2, -y2 - h, inf.tx, inf.ty + tdy});
        coords.push_back({x2 + w, -y2 - h, inf.tx + tdx, inf.ty + tdy});

        coords.push_back({x2, -y2, inf.tx, inf.ty});
        coords.push_back({x2, -y2 - h, inf.tx, inf.ty + tdy});
        coords.push_back({x2 + w, -y2, inf.tx + tdx, inf.ty});
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Coord2d_t) * coords.size(),
        coords.data(), GL_DYNAMIC_DRAW);
    LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLES, 0, coords.size()); LOGOPENGLERROR();
}
