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

out int         V_ID;
out float       V_alpha;
out int         V_render;
out float       V_node2D_alpha;
out int         V_connectivity;
out vec3        V_fragTexCoord;

uniform mat4    m_noRartionMatrix;
uniform mat4    vMatrix;
uniform mat4    pMatrix;
uniform int     y_axis;
uniform int     x_axis;

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

float translate(float value, float leftMin, float leftMax, float rightMin, float rightMax);

// here I should lock the rotation matrix
void main(void)
{
    V_connectivity = 0;
    V_node2D_alpha = 0;
    int ID = int(vertex.w);
    V_fragTexCoord = vertex.xyz;
    V_ID = ID;
    mat4 m_noRotvpMatrix = pMatrix * vMatrix * m_noRartionMatrix;

    // if we switch to 2D I can store the vertex multiplied by only rotation matrix here
    // then use the ssbo to transfer the data there
    // then I can have that information available all th time

    vec4 v_layout1 =  m_noRotvpMatrix * vec4(layout1, 0, 1); // original position
    vec4 v_layout2 =  m_noRotvpMatrix * vec4(layout2, 0, 1); // original position
    vec4 v_layout3 =  m_noRotvpMatrix * vec4(layout3, 0, 1); // original position

    vec4 node_layout1 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout1, 0, 1);
    vec4 node_layout2 = m_noRotvpMatrix * vec4(SSBO_data[ID].layout2, 0, 1);

    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite

    properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // interval this state is between
    vec2 positions = space_properties.positions; // which positions to interpolate between them
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)
    if (render_type.w == 1)
        V_render = 1;
    else
        V_render = 0;

   if (type == mito)
       V_render = 0;


   // if we are in the 2D space, then we have two interpolation for the neurites
   // for the nodes and skeletons along the y axis (node layout 1 and node layout 2)
   // for the neurites skeleton as well along the y axis (layout 1 and layout 3)
   // then we interpolate along the x axis using these two values

   // for skeleton get the value between v_layout3 and v_layout1 using mix with y_axis
   // then get another position for node in layout 1 and layout 2
   // then use these two new values to get the final one along the x axis

    gl_PointSize =   point_size.x;
   if (type != astrocyte){ // enruties
        if (x_axis >= interval.y - 5) {
            float max_point_size = point_size.y + SSBO_data[ID].info.x /* ? */;
            float min_point_size = point_size.x;
            if ( int(point_size.x) == int(point_size.y) )
                min_point_size = max_point_size;

            gl_PointSize =  translate(x_axis, interval.x, interval.y, min_point_size, max_point_size);

        } else if (x_axis < interval.y - 10) {
            // more skeleton
             if ( type == spine || type == bouton || type == mito ) {
                // if this is a child and parent is not filtered
                int ParentID = int(SSBO_data[ID].info.z);

                int parent_filter_value = int(SSBO_data[ParentID].info.w);
                int parentVisibility = (parent_filter_value >> 0) & 1;
                if (parentVisibility == 0) // color this special color that would show this is a mix of parent and child
                    V_render = 0;
            }
        }

        V_alpha = 1;
        float position_intp_y = translate(y_axis, interval.x, interval.y - 5, 0, 1);
        vec4 skeleton_pos = mix(v_layout1 , v_layout3, position_intp_y);
        vec4 node_pos  = mix(node_layout2, node_layout1 ,  position_intp_y);

        // if type == neurite -> mix (skeleton, node, x )
        float position_intp_x = translate(x_axis, interval.x, interval.y - 5, 0, 1);
        gl_Position =  mix(skeleton_pos, node_pos , position_intp_x);
    } else {
        V_alpha =  translate(y_axis, extra_info.z, extra_info.w - 10, 1, 0); // hide astrocyte before end of interval
        // if type == astrocyte -> mix (skeleton layout 1, skeleton layout 2, x )
        float position_intp_x = translate(x_axis, interval.x, interval.y, 0, 1);
        gl_Position = mix(v_layout1,  v_layout2 ,  position_intp_x);

   }

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

