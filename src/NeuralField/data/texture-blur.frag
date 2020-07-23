#version 330 core

in vec2 xy_coord;

out vec4 frag_color;

uniform sampler2D tex;

uniform int blur_dir;
uniform sampler1D blur_kernel_tex;

const int VerticalBlur=1;
const int HorizontalBlur=2;

vec2 blur_duv(int dir, int idx, vec2 dxy) {
    if (dir==VerticalBlur) {
        return vec2(dxy.x*float(idx),0.);
    }
    else if (dir==HorizontalBlur) {
        return vec2(0.,dxy.y*float(idx));
    }
    return vec2(0.); // for safety
}

void main(void){
    int blur_size=textureSize(blur_kernel_tex,0);
    float dt=1./float(blur_size);

    ivec2 tex_size=textureSize(tex,0);
    vec2 dxy=vec2(1.)/float(tex_size);

    float c=0.;
    for (int i=0,k=-blur_size/2; i<blur_size; i++,k++) {
        vec2 uv=xy_coord+blur_duv(blur_dir,k,dxy);
        c+=texture(tex,uv).r * texture(blur_kernel_tex,float(i)*dt).r;
    }

    frag_color=vec4(c);
}
