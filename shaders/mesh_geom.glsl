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
    vec2 interval;
    vec2 positions;
    vec4 render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
    vec4 extra_info;  // x: axis type, y, z, w: empty slots
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

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 color_alpha = space_properties.color_alpha; // color_intp
    vec2 interval = space_properties.interval; // additional info
    vec2 positions = space_properties.positions; // additional info
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type

    if (render_type.x == 0) {
        return;
    }

    float leftMin = interval.x;
    float leftMax = interval.y;

    alpha =  translate(slider, leftMin, leftMax,  trans_alpha.x, trans_alpha.y);

    float position_intp = translate(slider, leftMin, leftMax, pos_alpha.x, pos_alpha.y);
    color_intp = translate(slider, leftMin, leftMax, color_alpha.x, color_alpha.y);

    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);

    int pos1_flag = int(positions.x);
    int pos2_flag = int(positions.y);

    vec4 pos1, pos2;

    switch(pos1_flag)
    {
    case 1: pos1 = gl_in[i].gl_Position; break;
    case 2: pos1 = vec4(Vskeleton_vx[i].xyz, 1.0); break;
    case 3: pos1 = center4d; break;
    }

    switch(pos2_flag)
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


