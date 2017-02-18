#version 430

in int          V_ID[];
in vec4         V_knot1[];
in vec4         V_knot2[];

out float       color_intp;
out vec4        color_val;
out float       alpha;

layout (points) in;
layout (points, max_vertices = 1) out;

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


void main() {
    int ID = V_ID[0];
    int type = int(SSBO_data[ID].center.w);     // 0: astrocyte, 1: neurite
    int slider = (type == 0) ? y_axis : x_axis; // astrocyte or neurites?

    color_val = SSBO_data[ID].color;

    properties space_properties = (type == 0) ? space2d.ast : space2d.neu;

    vec2 alpha1 = space_properties.pos_alpha; // position interpolation (pos1, pos2)
    vec2 alpha2 = space_properties.trans_alpha; // alpha
    vec2 alpha3 = space_properties.color_alpha; // color_intp
    vec2 alpha4 = space_properties.point_size; // point_size
    vec4 alpha5 = space_properties.extra_info; // additional info
    vec4 alpha6 = space_properties.render_type; // additional info

    if (alpha6.z == 0) {
        return;
    }

    float leftMin = alpha5.x;
    float leftMax = alpha5.y;

    // use the space2D values to get: value of interpolation between pos1 and pos2, alpha, color_interpolation, point size
    alpha =  translate(slider, leftMin, leftMax, alpha2.x, alpha2.y);
    if (alpha < 0.01){
        return;
    }

    float position_intp = translate(slider,leftMin, leftMax,  alpha1.x, alpha1.y);
    color_intp = translate(slider, leftMin, leftMax, alpha3.y, alpha3.x);

    gl_PointSize =  translate(slider, leftMin, leftMax, alpha4.x, alpha4.y);


    // project the points onto line here
    vec4 A = V_knot1[0];
    vec4 B = V_knot2[0];
    vec4 p = gl_in[0].gl_Position;
    vec4 projected_point =  project_point_to_lint( A,  B,  p);

    int pos1_flag = int(alpha5.z);
    int pos2_flag = int(alpha5.w);

    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    vec4 center4d  = pvmMatrix * vec4(SSBO_data[ID].center.xyz, 1.0);
    vec4 pos1, pos2;

    switch(pos1_flag)
    {
    case 1: pos1 = gl_in[0].gl_Position; break;
    case 2: pos1 = gl_in[0].gl_Position; break;
    case 3: pos1 = center4d; break;
    case 4: pos1 = projected_point; break;
    }

    switch(pos2_flag)
    {
    case 1: pos2 = gl_in[0].gl_Position; break;
    case 2: pos2 = gl_in[0].gl_Position; break;
    case 3: pos2 = center4d; break;
    case 4: pos2 = projected_point; break;
    }

   vec4 new_position = mix(pos1 , pos2, position_intp);


   gl_Position = new_position;
   EmitVertex();
}



