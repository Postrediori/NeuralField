#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _MSC_VER
# pragma warning(disable: 4786)
#endif

#ifdef NDEBUG
# define LOGOPENGLERROR()
#else
# define LOGOPENGLERROR() LogOpenGLError(__FILE__,__LINE__)
#endif

void LogOpenGLError(const char *file, int line);

class FPSCounter {
public:
    FPSCounter();

    void update(int t);
    
public:
    int frame;
    int timePrev;
    float fps;
};
