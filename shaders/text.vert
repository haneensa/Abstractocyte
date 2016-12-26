#version 150

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

void main(void)
{
    vertUV = uv_coords;
  //  gl_Position =  vec4( ((xpos + vertIn.x)*scaleX) - 1.0, ((ypos + vertIn.y) * scaleY) + 1.0, 0.0, 1.0);
    gl_Position = vec4((xpos + vertIn.x) *scaleX,   (ypos +  vertIn.y) *scaleY,  0.0, 1.0);
//    gl_PointSize = 200;
}
