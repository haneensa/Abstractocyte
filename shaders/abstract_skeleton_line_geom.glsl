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
//layout(triangle_strip, max_vertices = 6) out;
layout(line_strip, max_vertices = 2) out;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];
in vec4         v_viewport[];
out vec4        color_val;
out float       alpha;

uniform vec4 viewport;

out vec2 v_start;
out vec2 v_line;
out float v_l2;

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
    for(int i = 0; i < 2; i++) {
        int ID = V_ID[i];

        int isFiltered = int(SSBO_data[ID].info.w);
        if (isFiltered == 1)
            return;

        color_val = SSBO_data[ID].color;


        alpha = V_alpha[i];
        if (alpha < 0.01){
            return;
        }

        int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

        properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

        vec4 render_type = space_properties.render_type; // additional info

        if ( V_render[i] == 0  ) {
            return;
        }

        gl_PointSize =  gl_in[i].gl_PointSize;
    }

    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    float t0 = start.z + start.w;
    float t1 = end.z + end.w;
    if(t0 < 0.0)
    {
        if(t1 < 0.0)
        {
            return;
        }
       start = mix(start, end, (0 - t0) / (t1 - t0));
    }
    if(t1 < 0.0)
    {
       end = mix(start, end, (0 - t0) / (t1 - t0));
    }


    // get viewport
    vec2 vpSize = v_viewport[0].zw/*viewport.yz*/;

    // Compute line axis and side vector in screen space
    vec2 startInNDC = start.wy / start.w;
    vec2 endInNDC = end.xy / end.w ;
    vec2 lineInNDC = endInNDC - startInNDC;
    vec2 startInScreen = (0.5 * startInNDC + vec2(0.5)) * vpSize  + v_viewport[0].xy;
    vec2 endInScreen = (0.5 * endInNDC + vec2(0.5)) * vpSize + v_viewport[0].xy;
    vec2 lineInScreen = lineInNDC * vpSize; // ndc to screen (direction vector)
    vec2 axisInScreen = normalize(lineInScreen);
    vec2 sideInScreen = vec2(-axisInScreen.y, axisInScreen.x);
    vec2 axisInNDC = axisInScreen / vpSize;
    vec2 sideInNDC = sideInScreen / vpSize;
    vec4 axis = vec4(axisInNDC, 0.0, 0.0) * 9 /*line width*/;
    vec4 side = vec4(sideInNDC, 0.0, 0.0) * 9 /*line width*/;

    vec4 a = (start + (side - axis) * start.w);
    vec4 b = (end + (side + axis)*  end.w);
    vec4 c = (end - (side - axis)*  end.w);
    vec4 d = (start - (side + axis)* start.w);


    v_start = startInScreen;
    v_line = endInScreen - startInScreen;
    v_l2 = dot(v_line, v_line);



//    gl_Position = a; EmitVertex();
//    gl_Position = d; EmitVertex();
//    gl_Position = b;  EmitVertex();
//    gl_Position = c; EmitVertex();
    gl_Position = start; EmitVertex();
    gl_Position = end; EmitVertex();
    EndPrimitive();
}

//  a - - - - - - - - - - - - - - - - b
//  |      |                   |      |
//  |      |                   |      |
//  |      |                   |      |
//  | - - -start - - - - - - end- - - |
//  |      |                   |      |
//  |      |                   |      |
//  |      |                   |      |
//  d - - - - - - - - - - - - - - - - c
