#version 430

// in: per vertex data
layout (location = 0) in vec3 coord3D;
layout (location = 1) in vec2 coord2D;
layout (location = 2) in int ID; // todo: pack this with coord3D

out int                      V_ID;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;
    vec4 Coord3D =  mvpMatrix * vec4(coord3D, 1.0); // original position
    vec4 Coord2D = mvpMatrix * vec4(coord2D, coord3D.z,  1.0); // layouted node
    gl_Position =  Coord3D;
    V_ID = ID;
}
