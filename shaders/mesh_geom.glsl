#version 430

in vec4         Vskeleton_vx[];
in  int         V_ID[];
in  int         V_bleeding[];

out float       color_intp;
out vec4        color_val;
out vec3        normal_out;
out float       alpha;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform int     y_axis;
uniform int     x_axis;
// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;


struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;
    vec2 layout2;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

struct properties {
    vec2 pos_alpha;
    vec2 trans_alpha;
    vec2 color_alpha;
    vec2 point_size;
    vec4 extra_info;
    vec4 render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
};

struct ast_neu_properties {
    properties ast;
    properties neu;
};


layout (std430, binding=3) buffer space2d_data
{
    ast_neu_properties space2d;
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

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 alpha1 = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 alpha2 = space_properties.trans_alpha; // alpha
    vec2 alpha3 = space_properties.color_alpha; // color_intp
    vec2 alpha4 = space_properties.point_size; // point_size
    vec4 alpha5 = space_properties.extra_info; // additional info
    vec4 alpha6 = space_properties.render_type; // additional info

    if (alpha6.x == 0) {
        return;
    }

    float leftMin = alpha5.x;
    float leftMax = alpha5.y;

    alpha =  translate(slider, leftMin, leftMax,  alpha2.x, alpha2.y);


    float position_intp = translate(slider, leftMin, leftMax, alpha1.x, alpha1.y);
    color_intp = translate(slider, leftMin, leftMax, alpha3.x, alpha3.y);

    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);


    switch(int(alpha5.z))
    {
    case 1: pos1 = gl_in[i].gl_Position; break;
    case 2: pos1 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos1 = center4d; break;
    }

    switch(int(alpha5.w))
    {
    case 1: pos2 = gl_in[i].gl_Position; break;
    case 2: pos2 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos2 = center4d; break;
    }

    vec4 new_position = mix(pos1 , pos2, position_intp);
    gl_Position = new_position;
    EmitVertex();
  }

  EndPrimitive();
}


