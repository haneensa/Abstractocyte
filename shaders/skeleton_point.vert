#version 330 core

in vec4 posAttr;
out vec4 posAttrV;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    posAttrV = posAttr;
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);
}
