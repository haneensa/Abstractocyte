#version 150

in vec4 posAttr;
out vec4 posAttrV;
uniform mat4 mvpMatrix;

void main(void)
{
    posAttrV = posAttr;
    gl_Position =  mvpMatrix * vec4(posAttr.x, posAttr.y, posAttr.z, 1.0);
}
