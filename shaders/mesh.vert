#version 150

in vec4 posAttr;
out vec4 posAttrV;

// todo: separate the transformation
// World transformation
// View Transformation
// Projection transformation
uniform mat4 mvpMatrix;

void main(void)
{
    posAttrV = posAttr;
    gl_Position =  mvpMatrix * vec4(posAttr.xyz, 1.0);
}
