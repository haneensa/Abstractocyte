#version 430
in vec3         normal_out;

in float        alpha;
in float        color_intp;
in vec4         color_val;
out vec4        outcol;

uniform int     y_axis;
uniform int     x_axis;

//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
uniform vec3 diffuseLightDirection;

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
    vec4 color = color_val;
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
    outcol = toon_color * color_intp +   (1.0 - color_intp) * phong_color;
    outcol.a = alpha;

}
