#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

out float        alpha;
out vec4         color_val;

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


void main() {
    vec4 render_type = space2d.ast.render_type; // additional info


    if ( !(render_type.x == 1 || render_type.y == 1  || render_type.z == 1) ) {
        return;
    }


    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 10;
    alpha = 0.7;
    color_val = vec4(1, 0, 0.5, 1);
    EmitVertex();
    EndPrimitive();
}
