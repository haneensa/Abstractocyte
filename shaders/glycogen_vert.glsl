#version 430

// in: per vertex data
layout (location = 0) in int ID;
layout (location = 1) in vec4 center;

uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;

// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;

    gl_Position =  mvpMatrix * vec4(center.xyz, 1.0);
}
