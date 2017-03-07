#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

out float        alpha;
out vec4         color_val;

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 10;
    alpha = 0.7;
    color_val = vec4(1, 0, 0, 1);
    EmitVertex();
    EndPrimitive();
}
