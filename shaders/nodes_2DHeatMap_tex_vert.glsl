#version 430

layout (location = 0) in vec4 vertex;

out vec2 V_fragTexCoord;

void main(void)
{
    V_fragTexCoord = vec2(vertex.z, vertex.w);
    gl_Position = vec4(vertex.xy, 0.5, 1);
}
