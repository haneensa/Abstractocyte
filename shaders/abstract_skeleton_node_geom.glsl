#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

out float       alpha;
out vec4        color_val;

in int          V_ID[];
in float        V_alpha[];


struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

void main() {
    int ID = V_ID[0];
    color_val = SSBO_data[ID].color;

    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    alpha = V_alpha[0];
    EmitVertex();
    EndPrimitive();
}
