#version 430

/* todo: draw thick lines with 3D effects
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

layout (lines) in;
layout(points, max_vertices = 100) out;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];
in vec4         v_viewport[];
out vec4        color_val;
out float       alpha;
out float         G_ID;

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
    int ID = V_ID[0] ;
    G_ID = float(ID);

    if ( int(SSBO_data[V_ID[0]].info.w) == 1 || int(SSBO_data[V_ID[1]].info.w) == 1 )
        return;

    alpha = V_alpha[0];
    if (alpha < 0.01){
        return;
    }

    if ( V_render[0] == 0  ) {
        return;
    }

    gl_PointSize =  gl_in[0].gl_PointSize;

    color_val = SSBO_data[ID].color;

    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    for (int i = 0; i < 100; i++ ) {
         float u = float(i) / float(100);
         float x = (end.x - start.x) * u + start.x;
         float y = (start.y - end.y) / (start.x - end.x) * (x - start.x) + start.y;
         gl_Position = vec4(x, y, 0, 1.0);
         EmitVertex();
         EndPrimitive();

    }
}
