#include "stdafx.h"
#include "GlUtils.h"

void LogOpenGLError(const char * file, int line) {
    GLenum err = glGetError();

    if (err!=GL_NO_ERROR)
        std::cerr << " OpenGL Error in file " << file << " line " << line << " : " << gluErrorString(err) << std::endl;
}

FPSCounter::FPSCounter()
    : frame(0)
    , timePrev(0)
    , fps(0.f) {
}

void FPSCounter::update(int t) {
    frame++;

    int dt = t - timePrev;
    if (dt<1000) return;

    fps = frame * 1000.f / (float)dt;
    timePrev = t;
    frame = 0;
}
