#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

in int          V_ID[];
out vec4        color_val;

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
    vec4 extra_info;
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

    vec2 alpha1 = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 alpha2 = space_properties.trans_alpha; // alpha
    vec2 alpha3 = space_properties.color_alpha; // color_intp
    vec2 alpha4 = space_properties.point_size; // point_size
    vec4 alpha5 = space_properties.extra_info; // additional info
    vec4 alpha6 = space_properties.render_type; // additional info

   // if ( alpha6.w == 0 || (y_axis < 98 || x_axis < 98) ) {
  //      return;
   // }

    color_val = SSBO_data[ID].color;
    gl_PointSize = 20;

    EmitVertex();
    EndPrimitive();
}
