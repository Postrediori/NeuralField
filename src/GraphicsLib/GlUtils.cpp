#include "stdafx.h"
#include "GlUtils.h"

void LogOpenGLError(const char* file, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE << " OpenGL Error in file " << file << " line " << line /*<< " : " << gluErrorString(err)*/;
    }
}

FPSCounter::FPSCounter()
    : frame(0)
    , timePrev(0.f)
    , fps(0.f) {
}

void FPSCounter::update(float t) {
    frame++;

    int dt = (int)(t - timePrev);
    if (dt < 1) {
        return;
    }

    fps = frame / (float)dt;
    timePrev = t;
    frame = 0;
}
