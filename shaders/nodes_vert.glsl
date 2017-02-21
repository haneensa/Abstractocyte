#version 430

// in: per vertex data
layout (location = 0) in vec3 coord3D;
layout (location = 1) in vec2 coord2D;
layout (location = 2) in int ID; // todo: pack this with coord3D

/*
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec2 layout1;
layout (location = 2) in vec2 layout2;
layout (location = 3) in vec2 layout3;
*/

out int                      V_ID;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

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
    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;
    vec4 Coord3D =  mvpMatrix * vec4(coord3D, 1.0); // original position
    vec4 Coord2D = mvpMatrix * vec4(coord2D, 0.0,  1.0); // layouted node
    gl_Position =  Coord3D;
    V_ID = ID;
}
