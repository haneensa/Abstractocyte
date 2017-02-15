#version 430

// todo: draw thick lines with 3D effects

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

in int          V_ID[];
out vec4        color_val;

uniform int     y_axis;
uniform int     x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
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
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 alpha1 = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 alpha2 = space_properties.trans_alpha; // alpha
    vec2 alpha3 = space_properties.color_alpha; // color_intp
    vec2 alpha4 = space_properties.point_size; // point_size
    vec4 alpha5 = space_properties.extra_info; // additional info
    vec4 alpha6 = space_properties.render_type; // additional info

    float leftMin = alpha5.x;
    float leftMax = alpha5.y;

  //  if ( alpha6.w == 0 || (y_axis < 98  || x_axis < 98) ) {
  //      return;
   // }

    //  Ideally I would interpolate betweeen the edges transparency to show it smothly?

    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_PointSize = gl_in[1].gl_PointSize;
    EmitVertex();
    EndPrimitive();

/*

1) get 4 vertices
    1.a) start of previous segment
    1.b) end of previous segment
    1.c) end of current segment
    1.d) end of next segment

2) naive culling
3) get direction of each 3 segments
4) get the normal of each of the 3 segments
5) miter lines
6) length of the miter by projecting it onto normal then inverse it
7) prevent long miters at sharp corners
8) close the gap
9) generate triangle strip


*/



}
