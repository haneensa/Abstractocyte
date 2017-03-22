#version 430

layout(location = 0) in vec2 vertex;

uniform mat4 pMatrix;

void main(void)
{
    gl_Position =  pMatrix * vec4(vertex.xy, 0.0, 1.0);
}
