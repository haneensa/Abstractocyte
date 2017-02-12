#version 430

in vec4         Vskeleton_vx[];
in  int         V_ID[];
in  vec4        V_center[];
in  int         V_bleeding[];

out float       color_intp;
out vec4        color_val;
out vec3        normal_out;
out float       alpha;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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
  vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  normal_out = normalize(cross(A,B));

 // int is_bleeding;
  //if ( V_bleeding[0] == 1 || V_bleeding[1] == 1 || V_bleeding[2] == 1)
  //   is_bleeding = 1;
 // else
  //   is_bleeding = 0;

  for(int i = 0; i < 3; i++) {
    int ID = V_ID[i];
    int type = int(SSBO_data[ID].center.w);
    if (V_bleeding[i] == 1)
        color_val = vec4(1.0, 0.0, 0.0, 1.0);
    else
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

    if (alpha6.x == 0) {
        return;
    }

    alpha =  translate(slider, alpha2.x, alpha2.y, alpha2.z, alpha2.w);
    if (alpha < alpha5.x){
        return;
    }

    float position_intp = translate(slider, alpha1.x, alpha1.y, alpha1.z, alpha1.w);
    color_intp = translate(slider, alpha3.x, alpha3.y, alpha3.z, alpha3.w);

    switch(int(alpha5.z))
    {
    case 1: pos1 = gl_in[i].gl_Position; break;
    case 2: pos1 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos1 = vec4(V_center[i].xyz, 1.0); break;
    }

    switch(int(alpha5.w))
    {
    case 1: pos2 = gl_in[i].gl_Position; break;
    case 2: pos2 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos2 = vec4(V_center[i].xyz, 1.0); break;
    }

    vec4 new_position = mix(pos1 , pos2, position_intp);
    gl_Position = new_position;
    EmitVertex();
  }

  EndPrimitive();
}


