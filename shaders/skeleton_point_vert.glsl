#version 430

layout(location = 0) in vec4    posAttr;
layout(location = 1) in vec4    knot1;
layout(location = 2) in vec4    knot2;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

out int         V_ID;

out vec4        V_knot1;
out vec4        V_knot2;


void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);

    int ID = int(posAttr.w);
    V_ID = ID;

    V_knot1 = pvmMatrix * knot1;
    V_knot2 = pvmMatrix * knot2;
}
