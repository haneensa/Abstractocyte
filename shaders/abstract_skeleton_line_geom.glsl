#version 430

#define astrocyte   6
#define spine       5
#define bouton      3
#define mito      1

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
layout(points, max_vertices = 93) out;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];
in int          V_connectivity[];
in float        V_node2D_alpha[];

out vec4        color_val;
out float       alpha;
out float       G_ID;
out float       node2D_alpha; /*1 -> 3D, 0 -> 2D*/

uniform int                 hoveredID;

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

void main() {
    int ID = V_ID[0] ;
    G_ID = float(ID);

    for (int i = 0; i < 2; ++i) {
        int filter_value = int(SSBO_data[V_ID[i]].info.w);
        int visibility = (filter_value >> 0) & 1;

        if ( visibility == 1 )
            return;
    }


    alpha = V_alpha[0];
    if (alpha < 0.01){
        return;
    }

    if ( V_render[0] == 0  ) {
        return;
    }

    node2D_alpha = V_node2D_alpha[0];

    gl_PointSize =  gl_in[0].gl_PointSize;

    if (V_connectivity[0] == 1)
        color_val = vec4(0.8, 0.8, 0.8, 1);
    else
        color_val = SSBO_data[ID].color;

    if (hoveredID == ID) {
        color_val += vec4(0.2, 0.2, 0.2, 0);
    }

    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    for (int i = 0; i < 93; i++ ) {
         float u = float(i) / float(93);
         float x = (end.x - start.x) * u + start.x;
         float y = (start.y - end.y) / (start.x - end.x) * (x - start.x) + start.y;
         gl_Position = vec4(x, y, 0, 1.0);
         EmitVertex();
         EndPrimitive();

    }
}
