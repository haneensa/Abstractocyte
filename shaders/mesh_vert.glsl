#version 430

#define astrocyte 6

// todo later: find a way to include header
#extension GL_ARB_shading_language_include : require

layout(location = 0) in vec4    mesh_vx;
layout(location = 1) in vec4    skeleton_vx;

out int         V_ID;
out vec4        V_color_val;
out float       V_alpha;
out float       V_color_intp;
out int         V_render;

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

float translate(float value, float leftMin, float leftMax, float rightMin, float rightMax);

void main(void)
{ 

    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 mesh_vertex =  pvmMatrix * vec4(mesh_vx.xyz , 1.0);
    vec4 Vskeleton_vx = pvmMatrix * vec4(skeleton_vx.xyz, 1.0);
    int ID = int(mesh_vx.w);

    V_ID = ID;

    int type = int(SSBO_data[ID].center.w);
    V_color_val = SSBO_data[ID].color;


    // if skeleton_vx.w is below a threshold then color it
    if (skeleton_vx.w <= 0.0001 && type !=  astrocyte )
        V_color_val.rgb = vec3(1.0, 0.0, 0.0);



    properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 color_alpha = space_properties.color_alpha; // color_intp
    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // interval this state is between
    vec2 positions = space_properties.positions; // which positions to interpolate between them
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type
    float leftMin = interval.x;
    float leftMax = interval.y;

    // use the space2D values to get: value of interpolation between pos1 and pos2, alpha, color_interpolation, point size
    V_alpha =  translate(slider, leftMin, leftMax, trans_alpha.x, trans_alpha.y);
    float position_intp = translate(slider,leftMin, leftMax,  pos_alpha.x, pos_alpha.y);
    V_color_intp = translate(slider, leftMin, leftMax, color_alpha.y, color_alpha.x);

    gl_PointSize =  translate(slider, leftMin, leftMax, point_size.x, point_size.y);

    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);

    int pos1_flag = int(positions.x);
    int pos2_flag = int(positions.y);

    vec4 pos1, pos2;


    switch(pos1_flag)
    {
    case 1: pos1 = mesh_vertex; break;
    case 2: pos1 = vec4(Vskeleton_vx.xyz, 1.0); break;
    case 3: pos1 = center4d; break;
    default:
        pos1 = mesh_vertex;
    }

    switch(pos2_flag)
    {
    case 1: pos2 = mesh_vertex; break;
    case 2: pos2 = vec4(Vskeleton_vx.xyz, 1.0); break;
    case 3: pos2 = center4d; break;
    default:
        pos2 = mesh_vertex;
    }

   vec4 new_position = mix(pos1 , pos2, position_intp);

   gl_Position = new_position;
}

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
