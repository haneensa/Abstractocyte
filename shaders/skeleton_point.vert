#version 430

layout(location = 0) in vec4 posAttr;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

out int         V_ID;


void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);
    int ID = int(posAttr.w);
    V_ID = ID;
}
