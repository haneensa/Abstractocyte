#version 150

// in: per vertex data
in vec4 posAttr;

in vec4 colAttr;
out vec4 colAttrv;

void main(void)
{
    gl_Position =  vec4(posAttr.x, posAttr.y, 0.0, 1.0);
    gl_PointSize = 100;
    colAttrv = colAttr;
}
