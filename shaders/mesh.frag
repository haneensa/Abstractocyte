#version 150

out vec4  outcol;
in vec4 posAttrG;

in vec3 normal_out;

// Cosine of the angle between the normal and the light direction,
// clamped above 0
//  - light is at the vertical of the triangle -> 1
//  - light is perpendicular to the triangle -> 0
//  - light is behind the triangle -> 0

vec3 light = vec3(1.0, 1.0, 0.0);
float cosTheta = dot( normal_out, light );

void main() {
    outcol = vec4(posAttrG.r, 0.5, posAttrG.b, 1.0) * cosTheta;
}
