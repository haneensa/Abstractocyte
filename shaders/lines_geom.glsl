#version 430

// todo: draw thick lines with 3D effects

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

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
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 interval = space_properties.interval; // additional info
    vec2 positions = space_properties.positions; // additional info
    vec4 render_type = space_properties.render_type; // additional info

    float leftMin = interval.x;
    float leftMax = interval.y;

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
