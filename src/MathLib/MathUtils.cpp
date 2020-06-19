#include "stdafx.h"
#include "MathUtils.h"

namespace Math
{
    mat4f GetDiagonalMatrix(float s) {
        mat4f m;
        m.v[0].s[0] = s;
        m.v[1].s[1] = s;
        m.v[2].s[2] = s;
        m.v[3].s[3] = s;
        return m;
    }

    mat4f GetOrthoProjection(float left, float right, float bottom, float top) {
        mat4f m = GetDiagonalMatrix(1.0f);

        m.v[0].s[0] = 2.0f / (right - left);
        m.v[1].s[1] = 2.0f / (top - bottom);
        m.v[2].s[2] = -1.0f;
        m.v[3].s[0] = -(right + left) / (right - left);
        m.v[3].s[1] = -(top + bottom) / (top - bottom);

        return m;
    }
}
