#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

in int          V_ID[];
out vec4        color_val;


in vec4 v_vertex[];
in vec4 v_layout1[];
in vec4 v_layout2[];
in vec4 v_layout3[];

uniform int     y_axis;
uniform int     x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;
    vec2 layout2;
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
    gl_Position = gl_in[0].gl_Position;
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec4 render_type = space_properties.render_type; // additional info

    if (render_type.w == 0) {
        return;
    }

    color_val = SSBO_data[ID].color;
    gl_PointSize = 2;

    EmitVertex();
    EndPrimitive();
}
