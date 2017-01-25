#version 430

out vec4        outcol;
in vec4         posAttrG;

uniform int     y_axis;
uniform int     x_axis;
in vec4        color_val;

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
    outcol = vec4(color_val.rgb, 1.0);
    float val = translate(y_axis, 50, 100, 0.0, 1.0);

    if (outcol.a < 0.5)
        discard;
}
