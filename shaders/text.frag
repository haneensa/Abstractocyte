#version 430

uniform sampler2D tex;
in vec2 vertUVg;
out vec4 fragColor;

void main(void)
{
    fragColor = texture(tex, vertUVg.st);
}
