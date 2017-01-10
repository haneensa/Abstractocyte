#version 330 core

in vec4 posAttr;
out vec4 posAttrV;

layout (std140) uniform Matrices
{
    mat4 view;
    mat4 projection;
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

    mat4 pvmMatrix = pMatrix * view * mMatrix;
//    mat4 pvmMatrix = projection * view * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);
}
