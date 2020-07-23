#pragma once

class FrameBufferWrapper {
public:
    ~FrameBufferWrapper();

    bool Init();
    void Release();

    void SetTexColorBuffer(GLuint tex);

    void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); LOGOPENGLERROR(); }
    static void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); LOGOPENGLERROR(); }

private:
    GLuint frameBuffer = 0;
    GLuint texColorBuffer = 0;
};
