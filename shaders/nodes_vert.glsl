#version 430

// this is for neurites nodes at the most abstract point.
// so only when the curser is at (100, 100) the node of this will show uo
// do I need this? only for the edges

// in: per vertex data
layout (location = 0) in int ID;

out float   V_alpha;
out int     V_ID;
out int      V_render;

// World transformation
uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;
uniform int  y_axis;
uniform int  x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
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

void main(void)
{

    V_ID = ID;
    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * m_noRartionMatrix;

    // todo: interpolate between different layouts based on state
    vec4 node_layout1 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout1, 0, 1);
    vec4 node_layout2 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout2, 0, 1);

    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite
    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 interval = space_properties.interval; // interval this state is between
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)
    if (extra_info.y == 1 && render_type.w == 1)
        V_render = 1;
    else
        V_render = 0;


    float alphax =  translate(x_axis, interval.x, interval.y,  0, 1);
    float alphay =  translate(y_axis, extra_info.z, extra_info.w,  0, 1);
    V_alpha = min(alphax, alphay);

    gl_PointSize = 1;

    float position_intp_y = translate(y_axis, extra_info.z, extra_info.w, 0, 1);
   // vec4 skeleton_pos = mix(v_layout1 , v_layout3, position_intp_y);
    vec4 node_pos  = mix(node_layout1, node_layout2 ,  position_intp_y);

    // if type == neurite -> mix (skeleton, node, x )
   // float position_intp_x = translate(x_axis, interval.x, interval.y, 0, 1);
    // gl_Position = node_pos * position_intp_x + ( 1.0 - position_intp_x) * skeleton_pos;
    //todo: mix the point on the skeleton with the two new layouted positions for smooth interpolation
    // along the x axis
    gl_Position = node_pos;

}
