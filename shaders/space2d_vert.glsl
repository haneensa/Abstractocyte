#version 430

// in: per vertex data
layout(location = 0) in vec4 posAttr;
layout(location = 1) in int ID;

out int V_ID;
uniform mat4 pMatrix;

void main(void)
{
    gl_Position =  pMatrix * vec4(posAttr.x, posAttr.y, 0.0, 1.0);
    V_ID = ID;
}
