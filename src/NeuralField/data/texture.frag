#version 330 core

in vec2 xy_coord;

out vec4 frag_color;

uniform sampler2D tex;

const vec4 col0=vec4(.5,.5,1.,1.);

void main(void){
    float c=texture(tex,xy_coord).r;
    frag_color=mix(col0,vec4(0.15),1.-c);
    frag_color.a=1.;
}
