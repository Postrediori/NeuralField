#pragma once

#ifdef NDEBUG
# define LOGOPENGLERROR()
#else
# define LOGOPENGLERROR() LogOpenGLError(__FILE__,__LINE__)
#endif

void LogOpenGLError(const char *file, int line);

class FPSCounter {
public:
    FPSCounter();

    void update(float t);

public:
    int frame;
    float timePrev;
    int fps;
};
