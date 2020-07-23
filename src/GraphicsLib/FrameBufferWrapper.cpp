#include "stdafx.h"
#include "GlUtils.h"
#include "FrameBufferWrapper.h"

FrameBufferWrapper::~FrameBufferWrapper() {
    Release();
}

bool FrameBufferWrapper::Init() {
    glGenFramebuffers(1, &frameBuffer); LOGOPENGLERROR();
    if (!frameBuffer) {
        LOGE << "Failed to create framebuffer";
        return false;
    }

    return true;
}

void FrameBufferWrapper::Release() {
    glDeleteFramebuffers(1, &frameBuffer); LOGOPENGLERROR();
}

void FrameBufferWrapper::SetTexColorBuffer(GLuint tex) {
    texColorBuffer = tex;

    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0); LOGOPENGLERROR();
    Unbind();
}
