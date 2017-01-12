#version 330 core

in vec3 posAttr1;
in vec3 posAttr2;
// I need posAttrSkeleton -> and then interpolate between posAttr and posAttrSkeleton
out vec4 posAttrV;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    posAttrV = pvmMatrix * vec4(posAttr2.xyz, 1.0);
    gl_Position =  pvmMatrix * vec4(posAttr1.xyz , 1.0);
}
