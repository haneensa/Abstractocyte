#version 430

#define astrocyte 6
#define spine       5
#define bouton      3
#define mito      1

layout(location = 0) in vec4    vertex;
layout(location = 1) in vec4    knot1;
layout(location = 2) in vec4    knot2;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

uniform int     y_axis;
uniform int     x_axis;

out int         V_ID;
out float       V_alpha;
out float       V_color_intp;

// make common header and add all these shared data together!
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

vec4 project_point_to_lint(vec4 A, vec4 B, vec4 p);

void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 skeleton_vertex =  pvmMatrix * vec4(vertex.xyz, 1.0);

    int ID = int(vertex.w);

    int type = int(SSBO_data[ID].center.w);     // 0: astrocyte, 1: neurite

    properties space_properties = (type == astrocyte) ? space2d.ast : space2d.neu;

    int isFiltered = int(SSBO_data[ID].info.w);
    if ( type == spine || type == bouton || type == mito) {
        // if this is a child and parent is not filtered
        int ParentID = int(SSBO_data[ID].info.z);
        int parentType = int(SSBO_data[ParentID].center.w);
        int isParentFiltered = int(SSBO_data[ParentID].info.w);

        // test this
        if (parentType == spine || parentType == bouton) {
            if (isParentFiltered == 1) // if they are filtered but their parent is not, make this as part of the parent
                ParentID =  int(SSBO_data[ParentID].info.z);
        }

        isParentFiltered = int(SSBO_data[ParentID].info.w);
        // end test

        if (isParentFiltered == 0 && isFiltered == 1)  // parent not filtered but child is
            ID = ParentID;
        if (type == mito && parentType == astrocyte)
            space_properties = space2d.ast;


    }

    V_ID = ID;


    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 color_alpha = space_properties.color_alpha; // color_intp
    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // additional info
    vec2 positions = space_properties.positions; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type

    if (type == astrocyte)
        V_alpha =  translate(slider, interval.x, interval.y, trans_alpha.x, trans_alpha.y);
    else
        V_alpha = 1;

    float position_intp = translate(slider,interval.x, interval.y,  pos_alpha.x, pos_alpha.y);
    V_color_intp = translate(slider, interval.x, interval.y, color_alpha.y, color_alpha.x);


    // project the points onto line here
    vec4 A = pvmMatrix * knot1;
    vec4 B = pvmMatrix * knot2;
    vec4 p = skeleton_vertex;
    vec4 projected_point =  project_point_to_lint( A,  B,  p);

    int pos1_flag = int(positions.x);
    int pos2_flag = int(positions.y);

    float max_point_size = point_size.y;
    if (pos2_flag == 6)
      max_point_size = point_size.y + SSBO_data[ID].info.x;
    gl_PointSize =  translate(slider, interval.x, interval.y, point_size.x, max_point_size);

    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);
    vec4 pos1, pos2;

    switch(pos1_flag)
    {
    case 4: pos1 = skeleton_vertex; break; // duplicate!!
    case 5: pos1 = projected_point; break;
    case 6: pos1 = center4d; break;
    default:
        pos1 = skeleton_vertex;
    }

    switch(pos2_flag)
    {
    case 4: pos2 = skeleton_vertex; break;
    case 5: pos2 = projected_point; break;
    case 6: pos2 = center4d; break;
    default:
        pos2 = skeleton_vertex;
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

vec4 project_point_to_lint(vec4 A, vec4 B, vec4 p)
{
    vec4 q; // closest point to p on the line segment from A to B

    vec4 AB = (B-A); // vector from A to B
    float AB_squared = dot(AB, AB); // squared distance from A to B
    if (AB_squared == 0.0) {
        // A and B are the same point
        q = A;
    } else {
        vec4 Ap = (p-A); // vector from A to p
        float t = dot(Ap, AB) / AB_squared; // A + t (B - A)
        if ( t < 0.0) // before A on the line, return A
            q = A;
        else if (t > 1.0) // after B on the line, return B
            q = B;
        else // projection lines inbetween A and B on the line
            q = A + t * AB;
    }

    return q;
}

