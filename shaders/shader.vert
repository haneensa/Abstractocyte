#version 150

// in: per vertex data
in vec4 posAttr;
uniform mat4 mvpMatrix;

void main(void)
{
    gl_Position =  mvpMatrix * vec4(posAttr.x, posAttr.y, 0.0, 1.0);
}
