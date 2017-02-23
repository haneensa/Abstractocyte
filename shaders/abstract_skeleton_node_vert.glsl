#version 430

// in: per vertex data
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec2 layout1;
layout (location = 2) in vec2 layout2;
layout (location = 3) in vec2 layout3;

out int V_ID;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

uniform int is2D;

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

void main(void)
{
    int ID = int(vertex.w);
    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;
    vec3 position;

    // todo: interpolate between different layouts based on state
    if (is2D == 1) {
        position = vec3(layout2, 0.0);
    } else {
        position =  vertex.xyz ;
    }

    gl_Position =  mvpMatrix * vec4(position, 1.0); // original position
    V_ID = ID;
}
