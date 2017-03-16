#version 430


layout(location = 0) out vec4        outcol;

in vec4 color_val;

void main() {
    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;


    outcol = vec4(color_val.rgb, 1);
}
