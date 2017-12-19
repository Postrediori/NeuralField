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

    void update(int t);
    
public:
    int frame;
    int timePrev;
    float fps;
};
