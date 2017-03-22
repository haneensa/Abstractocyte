#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

uniform int subpath;

void main() {

    if (subpath == 1)
        gl_PointSize = 4;
    else
        gl_PointSize = 2;
    for (int i = 0; i < 1; ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}
