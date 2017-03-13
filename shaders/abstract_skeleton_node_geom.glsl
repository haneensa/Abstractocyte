#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

out float       alpha;
out vec4        color_val;
out float         G_ID;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];

layout(location = 9) uniform int   max_astro_coverage;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
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

    alpha = V_alpha[0];
    if (alpha < 0.05){
        return;
    }

    if ( V_render[0] == 0  ) {
        return;
    }

    int isFiltered = int(SSBO_data[ID].info.w);
    if (isFiltered == 1)
        return;

    gl_PointSize = gl_in[0].gl_PointSize;

    color_val = SSBO_data[ID].color;
    if (color_val.w <= 0.0001) {
            float coverage = SSBO_data[ID].info.y / (float(max_astro_coverage));
            vec4 add_color = (color_val - vec4(1, 1, 1, 1)) * coverage;
            color_val.rgb = color_val.rgb - add_color.rgb;
    }

    gl_Position = gl_in[0].gl_Position;


    EmitVertex();
    EndPrimitive();
}
