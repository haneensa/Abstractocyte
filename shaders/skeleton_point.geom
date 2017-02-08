#version 430

in int          V_ID[];

out vec4        color_val;
out float       alpha;

layout (points) in;
layout (points, max_vertices = 1) out;

uniform int     y_axis;
uniform int     x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

layout (std430, binding=3) buffer space2d_data
{
    vec4 space2d[2][6];
};

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

    int ID = V_ID[0];

    int type = int(SSBO_data[ID].center.w);

    color_val = SSBO_data[ID].color;

    // astrocyte or neurites?
    int slider = (type == 0) ? y_axis : x_axis;

    vec4 alpha1 = space2d[type][0]; // position interpolation (pos1, pos2)
    vec4 alpha2 = space2d[type][1]; // alpha
    vec4 alpha3 = space2d[type][2]; // color_intp
    vec4 alpha4 = space2d[type][3]; // point_size
    vec4 alpha5 = space2d[type][4]; // additional info
    vec4 alpha6 = space2d[type][5]; // additional info

    // use the space2D values to get: value of interpolation between pos1 and pos2, alpha, color_interpolation, point size
    alpha =  translate(slider, alpha2.x, alpha2.y, alpha2.z, alpha2.w);

    if (alpha < 0.09){
        return;
    }

    if (alpha6.z == 0) {
        return;
    }

    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 6;

    EmitVertex();
    EndPrimitive();
}



