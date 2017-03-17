#version 430

#define astrocyte   6

layout (location = 0) in int ID;


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

struct properties {
    vec2 pos_alpha;
    vec2 trans_alpha;
    vec2 color_alpha;
    vec2 point_size;
    vec2 interval;
    vec2 positions;
    vec4 render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
    vec4 extra_info;  // x: axis type, y, z, w: empty slots
};

struct ast_neu_properties {
    properties ast;
    properties neu;
};

layout (std430, binding=3) buffer space2d_data
{
    ast_neu_properties space2d;
};

// World transformation
uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;
uniform int  y_axis;
uniform int  x_axis;


out float   V_alpha;
out int     V_ID;
out int     V_render;


void main(void)
{
    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite
    vec2 point_size = space2d.neu.point_size; // point_size
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * m_noRartionMatrix;
    vec4 node_pos = m_noRotvpMatrix * vec4(SSBO_data[ID].layout1.xy, 0, 1);
    V_ID = ID;
    gl_Position = node_pos;
    gl_PointSize = 15 + SSBO_data[ID].info.x;
}
