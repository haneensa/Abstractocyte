#version 430

layout (points) in;
layout (points, max_vertices = 1) out;

in float   V_alpha[];
in int     V_ID[];
in int     V_render[];

out vec4   color_val;


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

void main()
{
    gl_Position = gl_in[0].gl_Position;
    int ID =  int(V_ID[0]);
    float intensity = SSBO_data[ID].info.y;

    // if filtered then return
    int filter_value = int(SSBO_data[ID].info.w);
    int visibility = (filter_value >> 0) & 1;
    if (visibility == 1)
        return;

    // check: intensity shold be normalied
    color_val = vec4(intensity, 0, 0, intensity); // get the maximum value for make this normalized

    gl_PointSize = gl_in[0].gl_PointSize + 100;

    EmitVertex();
    EndPrimitive();
}
