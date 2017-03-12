#version 430

in float        G_ID;
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
    float blue = int(G_ID)%256;
    float grean = int(G_ID / 256) % 256;
    int red = int(G_ID / 65536) % 256;

    // deal with parent and child overlapping IDs
    outcol =   vec4(blue/255.0, grean/255.0, 0, 1.0);
}
