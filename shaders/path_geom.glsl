#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

void main() {

    gl_PointSize = 2;
    for (int i = 0; i < 1; ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}
