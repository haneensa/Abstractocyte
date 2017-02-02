#version 430

in vec4         posAttrV[];
out vec4        posAttrG;
out vec4        color_val;
in int          V_ID[];
out float       G_ID;

layout (points) in;
layout (points, max_vertices = 1) out;


struct SSBO_datum {
    vec4 color;
    vec4 center;
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

    // astrocyte or neurites?
    int slider = (type == 0) ? y_axis : x_axis;

    vec4 alpha6 = space2d[type][5]; // additional info
    if (alpha6.z == 0) {
        return;
    }

    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 6;
    G_ID = float(V_ID[0]);
    EmitVertex();
    EndPrimitive();
}



