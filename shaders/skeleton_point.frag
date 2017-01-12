#version 330 core
out vec4        outcol;
in vec4         posAttrG;
uniform vec3    color;

uniform int     y_axis;
uniform int     x_axis;
uniform int     state;

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
    outcol = vec4(color.rgb, 1.0);
    float val = translate(y_axis, 20, 100, 0.0, 1.0);
    outcol.a = val;

    if (outcol.a < 0.5)
        discard;
}
