#version 330 core

in vec4 posAttr;
out vec4 posAttrV;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

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
//    mat4 pvmMatrix = projection * view * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);
}
