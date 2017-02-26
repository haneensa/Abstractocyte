// todo: project this point onto the segment of which this is part of
#version 430

in int          V_ID[];

in vec4        V_color_val[];
in float       V_alpha[];
in float       V_color_intp[];

out vec4        color_val;
out float       color_intp;
out float       alpha;


layout(points) in;
layout(points, max_vertices = 1) out;

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
    int ID = V_ID[0]; // ID here is the index of this object in ssbo array
    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    color_val = V_color_val[0];

    // use type to chose which properties this vertex needs
    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

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
    gl_PointSize =  gl_in[0].gl_PointSize;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    EndPrimitive();
}


