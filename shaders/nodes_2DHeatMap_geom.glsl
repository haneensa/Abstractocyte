#version 430

layout (points) in;
layout (points, max_vertices = 1) out;


in float   V_alpha[];
in int     V_ID[];
in int     V_render[];

out vec4   color_val;


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

    gl_Position = gl_in[0].gl_Position;
    int ID =  int(V_ID[0]);
    color_val = vec4(SSBO_data[ID].color.rgb, 1);
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();
    EndPrimitive();
}
