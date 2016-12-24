#version 150

layout (points) in;
layout (points, max_vertices = 1) out;

in vec4 colAttrv[];
out vec4 colAttrg;

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 50;
    colAttrg = colAttrv[0];
    EmitVertex();
    EndPrimitive();
}
