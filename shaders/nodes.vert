#version 430

// in: per vertex data
layout (location = 0) in vec3 coord3D;
layout (location = 1) in vec2 coord2D;
layout (location = 2) in int ID;

out int                      V_ID;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(coord3D, 1.0);
    V_ID = ID;
}
