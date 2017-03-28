#version 430

#define astrocyte   6
#define spine       5
#define bouton      3
#define mito      1


layout (points) in;
layout (points, max_vertices = 1) out;

out float       alpha;
out vec4        color_val;
out float       G_ID;
out float       node2D_alpha; /*1 -> 3D, 0 -> 2D*/
flat out int    hasMito;
out vec3        G_fragTexCoord;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];
in float        V_node2D_alpha[];
in vec3         V_fragTexCoord[];

uniform int     hoveredID;

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


void main() {
    G_fragTexCoord = V_fragTexCoord[0];

    node2D_alpha = V_node2D_alpha[0];

    int ID = V_ID[0];
    G_ID = float(ID);

    alpha = V_alpha[0];
    if (alpha < 0.05){
        return;
    }

    if ( V_render[0] == 0  ) {
        return;
    }

    int filter_value = int(SSBO_data[ID].info.w);
    int visibility = (filter_value >> 0) & 1;
    if (visibility == 1)
        return;

    gl_PointSize = gl_in[0].gl_PointSize;

    color_val = SSBO_data[ID].color;

    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    if (SSBO_data[ID].color.w == 1.0)
        hasMito = 1;
    else
        hasMito = 0;

    if (hoveredID == ID) {
        color_val += vec4(0.2, 0.2, 0.2, 0);
    }

    gl_Position = gl_in[0].gl_Position;


    EmitVertex();
    EndPrimitive();
}
