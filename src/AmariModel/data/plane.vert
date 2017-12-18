#version 130

in vec2 coord;

uniform mat4 mvp;
uniform float zoom;
uniform vec2 ofs;
uniform vec2 res;

void main() {
    vec2 p = (coord + ofs) * zoom;
    if(res.x>res.y){
        p.x *= res.y / res.x;
    }else{
        p.y *= res.x / res.y;
    }
    gl_Position = mvp * vec4(p, 0., 1.);
}
