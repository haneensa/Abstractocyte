#version 430

#define astrocyte   6
#define spine       5
#define bouton      3
#define mito      1

// in: per vertex data
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec2 layout1;
layout (location = 2) in vec2 layout2;
layout (location = 3) in vec2 layout3;

out int     V_ID;
out float   V_alpha;
out int     V_render;
out float   V_node2D_alpha;
out int     V_connectivity;
out vec3    V_fragTexCoord;

// World transformation
uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;
uniform mat4 vMatrix;
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
    vec4 render_type; // mesh triangles, points skeleton, graph (points, edges), 2D
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

// here I should lock the rotation matrix
void main(void)
{
    V_connectivity = 0;
    // if has parent and parent not filtered, then use it
    // else use child if not filtered as well
    int ID = int(vertex.w);
    V_fragTexCoord = vertex.xyz;
    V_ID = ID;
    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * m_noRartionMatrix;

    vec4 v_vertex =  mvpMatrix * vec4(vertex.xyz, 1); // original position
    vec4 v_layout1 =  m_noRotvpMatrix * vec4(layout1, 0, 1); // original position
    vec4 v_layout2 =  m_noRotvpMatrix * vec4(layout2, 0, 1); // original position
    vec4 v_layout3 =  m_noRotvpMatrix * vec4(layout3, 0, 1); // original position

    vec4 node_center = mvpMatrix * vec4(SSBO_data[ID].center.xyz, 1);
    vec4 node_layout2 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout2, 0, 1);


    properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // interval this state is between
    vec2 positions = space_properties.positions; // which positions to interpolate between them
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    if (render_type.z == 1)
        V_render = 1;
    else
        V_render = 0;

    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type

    if ( type == spine || type == bouton) {
        // if this is a child and parent is not filtered
        int ParentID = int(SSBO_data[ID].info.z);

        int parent_filter_value = int(SSBO_data[ParentID].info.w);
        int parentVisibility = (parent_filter_value >> 0) & 1;
        if (parentVisibility == 0 &&  (int(positions.y) != 6)) // color this special color that would show this is a mix of parent and child
            V_render = 0;
    }

    V_node2D_alpha = translate(slider, interval.x, interval.y, 1, 0);

    switch (type) {
    case mito:
        V_alpha =  V_node2D_alpha;
        break;
    default:
        V_alpha =  1.0;
    }

    if (V_alpha < 0.4)
        V_render = 0;


    float max_point_size = point_size.y;
    float min_point_size = point_size.x;
    if ( type != astrocyte && (int(positions.y) == 6 || int(positions.y) == 5) )
      max_point_size = point_size.y + SSBO_data[ID].info.x;

    if ( int(point_size.x) == int(point_size.y) )
        min_point_size = max_point_size;

    gl_PointSize =  translate(slider, interval.x, interval.y, min_point_size, max_point_size);

    vec4 pos1, pos2;

    float position_intp = translate(slider, interval.x, interval.y,  pos_alpha.x, pos_alpha.y);
    switch( int(positions.x) )
    {
    case 1: pos1 = v_vertex; break;
    case 2: pos1 = v_layout1; break;
    case 3: pos1 = v_layout2; break;
    case 4: pos1 = v_layout3; break;
    case 5: pos1 = node_center; break;
    case 6: pos1 = node_layout2; break;
    }

    switch( int(positions.y) )
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
