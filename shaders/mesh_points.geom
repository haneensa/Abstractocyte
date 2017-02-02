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
    vec4 space2d[2][6];
};

void main() {
    int i = 0;
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    color_val = SSBO_data[ID].color;
    vec4 pos1, pos2;

    // astrocyte or neurites?
    int slider = (type == 0) ? y_axis : x_axis;

    vec4 alpha1 = space2d[type][0]; // position interpolation (pos1, pos2)
    vec4 alpha2 = space2d[type][1]; // alpha
    vec4 alpha3 = space2d[type][2]; // color_intp
    vec4 alpha4 = space2d[type][3]; // point_size
    vec4 alpha5 = space2d[type][4]; // additional info
    vec4 alpha6 = space2d[type][5]; // additional info

    if (alpha6.y == 0) {
        return;
    }

    // use the space2D values to get: value of interpolation between pos1 and pos2, alpha, color_interpolation, point size
    alpha =  translate(slider, alpha2.x, alpha2.y, alpha2.w, alpha2.z);
    if (alpha < alpha5.y){
        return;
    }

    float position_intp = translate(slider, alpha1.x, alpha1.y, alpha1.z, alpha1.w);
    color_intp = translate(slider, alpha3.x, alpha3.y, alpha3.z, alpha3.w);
    gl_PointSize =  translate(slider, alpha4.x, alpha4.y, alpha4.z, alpha4.w);

    int pos1_flag = int(alpha5.z);
    int pos2_flag = int(alpha5.w);

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

   vec4 new_position = mix(pos1 , pos2, position_intp);
   gl_Position = new_position;
   EmitVertex();
   EndPrimitive();

}


