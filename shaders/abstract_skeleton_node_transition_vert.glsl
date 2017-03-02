#version 430

// in: per vertex data
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec2 layout1;
layout (location = 2) in vec2 layout2;
layout (location = 3) in vec2 layout3;

out int V_ID;
out float V_alpha;
out int     V_render;

// World transformation
uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;

// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

uniform int  y_axis;
uniform int  x_axis;
uniform int is2D;

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

// here I should lock the rotation matrix
void main(void)
{
    int ID = int(vertex.w);
    V_ID = ID;
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * m_noRartionMatrix;

    vec4 v_vertex =  mvpMatrix * vec4(vertex.xyz, 1); // original position
    vec4 v_layout1 =  m_noRotvpMatrix * vec4(layout1, 0, 1); // original position
    vec4 v_layout2 =  m_noRotvpMatrix * vec4(layout2, 0, 1); // original position
    vec4 v_layout3 =  m_noRotvpMatrix * vec4(layout3, 0, 1); // original position

    vec4 node_center = mvpMatrix * vec4(SSBO_data[ID].center.xyz, 1);
    vec4 node_layout2 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout2, 0, 1);

    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 color_alpha = space_properties.color_alpha; // color_intp
    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // interval this state is between
    vec2 positions = space_properties.positions; // which positions to interpolate between them
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    if (extra_info.y == 0 && render_type.w == 1)
        V_render = 1;
    else
        V_render = 0;


    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type

    float leftMin = interval.x;
    float leftMax = interval.y;

    gl_PointSize =  translate(slider, leftMin, leftMax, point_size.x, point_size.y);


    V_alpha =  translate(y_axis, leftMin, leftMax, trans_alpha.x, trans_alpha.y);
    int pos1_flag = int(positions.x);
    int pos2_flag = int(positions.y);

    vec4 pos1, pos2;


    float position_intp = translate(slider, leftMin, leftMax,  pos_alpha.x, pos_alpha.y);
    switch(pos1_flag)
    {
    case 1: pos1 = v_vertex; break;
    case 2: pos1 = v_layout1; break;
    case 3: pos1 = v_layout2; break;
    case 4: pos1 = v_layout3; break;
    case 5: pos1 = node_center; break;
    case 6: pos1 = node_layout2; break;
    }

    switch(pos2_flag)
    {
    case 1: pos2 = v_vertex; break;
    case 2: pos2 = v_layout1; break;
    case 3: pos2 = v_layout2; break;
    case 4: pos2 = v_layout3; break;
    case 5: pos2 = node_center; break;
    case 6: pos2 = node_layout2; break;
    }

    gl_Position = mix(pos1,  pos2 ,  position_intp)  ;
}
