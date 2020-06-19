#pragma once

namespace Math
{
    struct vec2f
    {
        float x, y;

        vec2f() : x(0.0f), y(0.0f) { }
        vec2f(float x, float y) : x(x), y(y) { }

        vec2f& operator+(const vec2f& a)
        {
            vec2f n(x + a.x, y + a.y);
            return n;
        }
    };

    struct vec4f
    {
        float s[4];

        vec4f() { s[0] = 0.0f; s[1] = 0.0f; s[2] = 0.0f; s[3] = 0.0f; }
        vec4f(float x, float y, float z, float w) { s[0] = x; s[1] = y; s[2] = z; s[3] = w; }
    };

    struct mat4f
    {
        vec4f v[4];

        mat4f() { v[0] = vec4f(); v[1] = vec4f(); v[2] = vec4f(); v[3] = vec4f(); }
        mat4f(vec4f v0, vec4f v1, vec4f v2, vec4f v3) { v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3; }
    };

    mat4f GetDiagonalMatrix(float s);

    mat4f GetOrthoProjection(float left, float right, float bottom, float top);
}
