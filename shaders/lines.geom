#version 430

layout (lines) in;
layout (line_strip, max_vertices = 2) out;


void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_PointSize = gl_in[1].gl_PointSize;
    EmitVertex();
    EndPrimitive();

}
