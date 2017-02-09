#version 430

layout (points) in;
layout (points, max_vertices = 1) out;



struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};


layout (std430, binding=3) buffer space2d_data
{
    vec4 space2d[2][6];
};


void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 20;
    EmitVertex();
    EndPrimitive();
}
