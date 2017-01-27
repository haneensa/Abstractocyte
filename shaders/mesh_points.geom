#version 430

in vec4         Vskeleton_vx[];

in int          V_ID[];
in vec4         V_center[];

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
    vec4 space2d[2][5];
};

void main() {
    int i = 0;
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    vec4 pos1, pos2;
    int slider = (type == 0) ? y_axis : x_axis;
    vec4 alpha1 = space2d[type][0];
    vec4 alpha2 = space2d[type][1];
    vec4 alpha3 = space2d[type][2];
    vec4 alpha4 = space2d[type][3];
    vec4 alpha5 = space2d[type][4];

    float val = translate(slider, alpha1.x, alpha1.y, alpha1.z, alpha1.w);
    alpha =  translate(slider, alpha2.x, alpha2.y, alpha2.w, alpha2.z);
    color_intp = translate(slider, alpha3.x, alpha3.y, alpha3.z, alpha3.w);
    gl_PointSize =  translate(slider, alpha4.x, alpha4.y, alpha4.z, alpha4.w);

    int pos1_flag = int(alpha5.z);
    int pos2_flag = int(alpha5.w);

    // to switch from skeleton to node
    if (type == 1) {
        if (slider > alpha5.y) {
            pos1_flag = 2;
            pos2_flag = 3;
            gl_PointSize =  translate(slider, alpha5.y, 100, 7, 30);
            val = translate(slider, alpha5.y, 100, 0.0, 1.0);
            alpha = 1.0;
        }

   }

    switch(pos1_flag)
    {
    case 1: pos1 = gl_in[i].gl_Position; break;
    case 2: pos1 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos1 = vec4(V_center[i].xyz, 1.0); break;
    }

    switch(pos2_flag)
    {
    case 1: pos2 = gl_in[i].gl_Position; break;
    case 2: pos2 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos2 = vec4(V_center[i].xyz, 1.0); break;
    }

   vec4 new_position = mix(pos1 , pos2, val);
   gl_Position = new_position;
   EmitVertex();
   EndPrimitive();

}


