#version 430

layout(location = 0) in vec4    posAttr;
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


void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 skeleton_vertex =  pvmMatrix * vec4(posAttr.xyz, 1.0);

    int ID = int(posAttr.w);
    V_ID = ID;

    int type = int(SSBO_data[ID].center.w);     // 0: astrocyte, 1: neurite

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 pos_alpha = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 trans_alpha = space_properties.trans_alpha; // alpha
    vec2 color_alpha = space_properties.color_alpha; // color_intp
    vec2 point_size = space_properties.point_size; // point_size
    vec2 interval = space_properties.interval; // additional info
    vec2 positions = space_properties.positions; // additional info
    vec4 render_type = space_properties.render_type; // additional info
    vec4 extra_info = space_properties.extra_info;   // x: axis type (0: x_axis, 1: y_axis)

    int slider = (extra_info.x == 1) ? y_axis : x_axis;  // need to make this general and not tied to object type

    float leftMin = interval.x;
    float leftMax = interval.y;


    V_alpha =  translate(slider, leftMin, leftMax, trans_alpha.x, trans_alpha.y);


    float position_intp = translate(slider,leftMin, leftMax,  pos_alpha.x, pos_alpha.y);
    V_color_intp = translate(slider, leftMin, leftMax, color_alpha.y, color_alpha.x);

    gl_PointSize =  translate(slider, leftMin, leftMax, point_size.x, point_size.y);

    // project the points onto line here
    vec4 A = pvmMatrix * knot1;;
    vec4 B = pvmMatrix * knot2;;
    vec4 p = skeleton_vertex;
    vec4 projected_point =  project_point_to_lint( A,  B,  p);

    int pos1_flag = int(positions.x);
    int pos2_flag = int(positions.y);

    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);
    vec4 pos1, pos2;

    switch(pos1_flag)
    {
    case 1: pos1 = skeleton_vertex; break; // duplicate!!
    case 2: pos1 = skeleton_vertex; break;
    case 3: pos1 = center4d; break;
    case 4: pos1 = projected_point; break;
    }

    switch(pos2_flag)
    {
    case 1: pos2 = skeleton_vertex; break;
    case 2: pos2 = skeleton_vertex; break;
    case 3: pos2 = center4d; break;
    case 4: pos2 = projected_point; break;
    }

   vec4 new_position = mix(pos1 , pos2, position_intp);
   gl_Position = new_position;

}
