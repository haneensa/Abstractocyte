#version 430

in vec4 posAttrV[];
out vec4 posAttrG;
in  vec4        V_color[];
out vec4        color_val;

layout (points) in;
layout (points, max_vertices = 1) out;


void main() {
color_val = V_color[0];
gl_Position = gl_in[0].gl_Position;
gl_PointSize = 2;
EmitVertex();
EndPrimitive();
}



