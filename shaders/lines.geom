#version 430

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

in int          V_ID[];

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
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    vec4 alpha6 = space2d[type][5]; // additional info
    if (alpha6.w == 0) {
        return;
    }

    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_PointSize = gl_in[1].gl_PointSize;
    EmitVertex();
    EndPrimitive();
}
