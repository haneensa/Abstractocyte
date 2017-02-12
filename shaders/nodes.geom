#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

in int          V_ID[];
out vec4        color_val;


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
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    vec4 alpha6 = space2d[type][5]; // additional info
    if (alpha6.w == 0) {
        return;
    }
    color_val = SSBO_data[ID].color;
    gl_PointSize = 20;
    EmitVertex();
    EndPrimitive();
}
