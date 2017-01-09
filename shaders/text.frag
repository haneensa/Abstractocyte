#version 330 core

uniform sampler2D tex;
in vec2 vertUVg;
out vec4 fragColor;

void main(void)
{
    fragColor = texture(tex, vertUVg.st);
}
