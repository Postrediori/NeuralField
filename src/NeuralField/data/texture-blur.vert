#version 330 core

in vec2 coord;
in vec2 tex_coord;

out vec2 xy_coord;

void main(void) {
    gl_Position=vec4(coord,0.,1.);
    xy_coord=tex_coord;
}
