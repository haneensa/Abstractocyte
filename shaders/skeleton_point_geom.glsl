#version 430

#define astrocyte 6

in int          V_ID[];

in float        V_alpha[];
in float        V_color_intp[];

out float       color_intp;
out vec4        color_val;
out float       alpha;
out float       G_ID;


uniform int     y_axis;
uniform int     x_axis;


layout (points) in;
layout (points, max_vertices = 1) out;

// make common header and add all these shared data together!
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
    int ID = V_ID[0];
    G_ID = float(ID);
    int isFiltered = int(SSBO_data[ID].info.w);
    if (isFiltered == 1)
        return;

    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    gl_PointSize =  gl_in[0].gl_PointSize;

    properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

    vec4 render_type = space_properties.render_type; // additional info
    if (render_type.y == 0) {
        return;
    }

    // use the space2D values to get: value of interpolation between pos1 and pos2, alpha, color_interpolation, point size
    alpha =  V_alpha[0];
    if (alpha < 0.01){
        return;
    }

    color_intp = V_color_intp[0];
    gl_Position = gl_in[0].gl_Position;

    EmitVertex();
}



