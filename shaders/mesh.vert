#version 150

in vec4 posAttr;
uniform mat4 pMatrix;

void main(void)
{
    gl_Position =  pMatrix * vec4(posAttr.x, posAttr.y, posAttr.z, 1.0);
}
