#version 330 core

in vec2 coord;

uniform mat4 mvp;
uniform float zoom;
uniform vec2 ofs;
uniform vec2 res;

vec2 adjust_proportions(vec2 u, vec2 res) {
    vec2 v = u;
    if(res.x>res.y){
        v.x *= res.y / res.x;
    }
    else{
        v.y *= res.x / res.y;
    }
    return v;
}

void main() {
    vec2 p = (coord + ofs) * zoom;
    p = adjust_proportions(p, res);
    gl_Position = mvp * vec4(p, 0., 1.);
}
