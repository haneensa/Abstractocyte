#version 330 core

in vec4         posAttrG;
in vec3         normal_out;
out vec4        outcol;

uniform int     y_axis;
uniform int     x_axis;
uniform int     state;

// Cosine of the angle between the normal and the light direction,
// clamped above 0
//  - light is at the vertical of the triangle -> 1
//  - light is perpendicular to the triangle -> 0
//  - light is behind the triangle -> 0

//-------------------- AMBIENT LIGHT PROPERTIES --------------------
vec4 ambient = vec4(0.10, 0.10, 0.10, 1.0); // sets lighting level, same across many vertices

//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
vec3 diffuseLightDirection = vec3(-1.5f, 1.0f, -0.4f);
vec4 diffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
float diffuseIntersity = 1.0;

// ------------------- TOON SHADER PROPERTIES ----------------------
// vec3 lineColor = vec4(0.0, 0.0,  0.0, 1.0); -> color to draw the lines in  (black)
// float lineThickness = 0.03

float cosTheta = clamp( dot( normal_out, diffuseLightDirection ), 0, 1 );
float intensity = dot(diffuseLightDirection, normal_out);

float translate(float value, float leftMin, float leftMax, float rightMin, float rightMax)
{
    // if value < leftMin -> value = leftMin
    value = max(value, leftMin);
    // if value > leftMax -> value = leftMax
    value = min(value, leftMax);
    // Figure out how 'wide' each range is
    float leftSpan = leftMax - leftMin;
    float rightSpan = rightMax - rightMin;

    // Convert the left range into a 0-1 range (float)
    float valueScaled = float(value - leftMin) / float(leftSpan);

    // Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan);
}

void main() {
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * diffuseColor * diffuseIntersity + ambient;
    vec4 toon_color = vec4(color.r, color.g, color.b, 1.0);
    vec4 phong_color = vec4(color.r, color.g, color.b, 1.0) * cosTheta;

    if (intensity > 0.95)
        toon_color = vec4(1.0, 1.0, 1.0, 1.0) * toon_color;
    else if (intensity > 0.5)
        toon_color = vec4(0.7, 0.7, 0.7, 1.0) * toon_color;
    else if (intensity > 0.05)
        toon_color = vec4(0.35, 0.35, 0.35, 1.0) * toon_color;
    else
        toon_color = vec4(0.1, 0.1, 0.1, 1.0) * toon_color;

    // interpolate between two colors
    // todo: based on the mesh type (astro, neurite)
    // y_axis astrocyte, else if neurite use (x_axis)
    float val = translate(y_axis, 0, 20, 0.0, 1.0);
    outcol = toon_color * val +   (1.0 - val) * phong_color;
    val = translate(y_axis, 20, 100, 0.0, 1.0);
    outcol.a = 1.0 - val;
}
