#version 430

in float          G_ID;
out vec4        outcol;


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
    outcol =   vec4(G_ID/255.0, 1, 0, 1.0);
}
