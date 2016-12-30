#version 150

out vec4  outcol;
in vec4 posAttrG;

in vec3 normal_out;

// Cosine of the angle between the normal and the light direction,
// clamped above 0
//  - light is at the vertical of the triangle -> 1
//  - light is perpendicular to the triangle -> 0
//  - light is behind the triangle -> 0

vec3 light = vec3(0.1, 0.8, -0.1);

float cosTheta = clamp( dot( normal_out,light ), 0,1 );


void main() {
    outcol = vec4(posAttrG.r, posAttrG.g, posAttrG.b, 1.0) * cosTheta;
}
