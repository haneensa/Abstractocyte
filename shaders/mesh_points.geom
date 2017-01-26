#version 430

in vec4         Vskeleton_vx[];

in int          V_ID[];
in vec4         V_center[];
vec4            pos2;

out float       G_ID;

out vec4        color_val;
out float       color_intp;
out float       alpha;

layout(triangles) in;
layout(points, max_vertices = 1) out;

uniform int     y_axis;
uniform int     x_axis;

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

struct SSBO_datum {
    vec4 color;
    vec4 center;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

layout (std430, binding=3) buffer space2d_data
{
    int space2d[2][100];
};

void main() {
    int i = 0;
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    float val;
    vec4 pos1 = gl_in[i].gl_Position;
    pos2 = vec4(Vskeleton_vx[i].xyz, 1.0);

    if (type == 0) {
        if (y_axis < 99) {
            val = translate(y_axis, 20, 100, 0.0, 1.0);
            alpha = translate(y_axis, 20, 30, 0.0, 1.0);
            color_intp = translate(y_axis, 0, 20, 0.0, 1.0);
            vec4 new_position = pos2 * val + (1.0 - val) * pos1;
            gl_Position = new_position;
            gl_PointSize = 7;
            EmitVertex();
            EndPrimitive();
        }
   } else {
        int div = 50;
        if (x_axis < div) {
            gl_PointSize =  7;
            val = translate(x_axis, 20, div, 0.0, 1.0);
            alpha = translate(x_axis, 20, div, 0.0, 1.0);
        } else {
            pos1 = vec4(Vskeleton_vx[i].xyz, 1.0);
            pos2 = vec4(V_center[i].xyz, 1.0);
            gl_PointSize =  translate(x_axis, div, 100, 7, 30);
            val = translate(x_axis, div, 100, 0.0, 1.0);
            alpha = 1.0;
        }

        color_intp = translate(x_axis, 0, 20, 0.0, 1.0);
        vec4 new_position = pos2 * val + (1.0 - val) * pos1;
        gl_Position = new_position;
        EmitVertex();
        EndPrimitive();
   }
}


