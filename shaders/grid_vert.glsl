#version 430

// in: per vertex data
layout (location = 0) in vec2 coord2D;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

void main(void)
{
    mat4 pvmMatrix = pMatrix /* vMatrix  * mMatrix*/;
    gl_Position =  pvmMatrix * vec4(coord2D, 2.5,  1.0);
}
