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

layout (std430, binding=3) buffer space2d_data
{
    vec4 space2d[2][6];
};

void main() {
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    vec4 alpha6 = space2d[type][5]; // additional info

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
