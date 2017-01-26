#version 430

in vec4         Vskeleton_vx[];

in int          V_ID[];
in vec4         V_center[];
vec4            pos2;

out vec4        color_val;
out float       color_intp;
out float       alpha;

layout(points) in;
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
    vec4 space2d[2][4];
};

void main() {
    int i = 0;
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    vec4 pos1 = gl_in[i].gl_Position;
    pos2 = vec4(Vskeleton_vx[i].xyz, 1.0);
    int slider = (type == 0) ? y_axis : x_axis;
    vec4 alpha1 = space2d[type][0];
    vec4 alpha2 = space2d[type][1];
    vec4 alpha3 = space2d[type][2];
    vec4 alpha4 = space2d[type][3];

    float val = translate(slider, alpha1.x, alpha1.y, alpha1.z, alpha1.w);
    alpha =  translate(slider, alpha2.x, alpha2.y, alpha2.w, alpha2.z);
    color_intp = translate(slider, alpha3.x, alpha3.y, alpha3.z, alpha3.w);
    gl_PointSize =  translate(slider, alpha4.x, alpha4.y, alpha4.z, alpha4.w);

    // to switch from skeleton to node
    if (type == 1) {
        int div = 50;
        if (x_axis > div) {
            pos1 = vec4(Vskeleton_vx[i].xyz, 1.0);
            pos2 = vec4(V_center[i].xyz, 1.0);
            gl_PointSize =  translate(slider, div, 100, 7, 30);
            val = translate(slider, div, 100, 0.0, 1.0);
            alpha = 1.0;
        }

   }

   vec4 new_position = pos2 * val + (1.0 - val) * pos1;
   gl_Position = new_position;
   EmitVertex();
   EndPrimitive();

}


