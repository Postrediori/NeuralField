#version 330 core

in vec2 coord;
in vec2 tex_coord;

out vec2 xy_coord;

uniform vec2 iRes;
uniform mat4 mvp;

vec2 adjust_proportions(vec2 v, vec2 res) {
    vec2 xy=v;
    if (res.x>res.y) {
        xy.x*=res.y/res.x;
    } else {
        xy.y*=res.x/res.y;
    }
    return xy;
}

void main(void) {
    xy_coord=adjust_proportions(coord.xy,iRes);
    gl_Position=mvp*vec4(xy_coord,0.,1.);
    xy_coord=tex_coord.xy;
}
