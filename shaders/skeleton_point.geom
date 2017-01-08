#version 150
in vec4 posAttrV[];
out vec4 posAttrG;

layout (points) in;
layout (points, max_vertices = 1) out;


void main() {
gl_Position = gl_in[0].gl_Position;
gl_PointSize = 2;
EmitVertex();
EndPrimitive();
}



