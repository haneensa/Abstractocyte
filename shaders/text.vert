#version 330 core

// in: per vertex data
in vec2 vertIn;

// interpolated values from the vertex shaders
in vec2 uv_coords;
out vec2 vertUV;
//uniform vec3 textColor;
uniform float scaleX;
uniform float scaleY;
uniform float xpos;
uniform float ypos;
uniform mat4 pMatrix;

void main(void)
{
    vertUV = uv_coords;
    gl_Position = pMatrix * vec4( (xpos + vertIn.x) * scaleX,   (ypos +  vertIn.y) *scaleY,  0.0, 1.0);
}
