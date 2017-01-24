#version 430

in vec4         Vskeleton_vx[];

in int          V_ID[];
in vec4         V_color[];
in vec4         V_center[];
vec4            pos2;

out float       G_ID;

out vec4        color_val;
out float       color_intp;
out vec3        normal_out;
out float       alpha;

layout(triangles) in;
layout(points, max_vertices = 1) out;

uniform int     y_axis;
uniform int     x_axis;

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

    int i = 0;
    color_val = V_color[i];
    G_ID = float(V_ID[i]);
    float val;
    vec4 pos1 = gl_in[i].gl_Position;
    pos2 = vec4(Vskeleton_vx[i].xyz, 1.0);

    if (G_ID <= 0.0) {
        val = translate(y_axis, 20, 100, 0.0, 1.0);
        alpha = translate(y_axis, 90, 100, 0.0, 1.0);
        color_intp = translate(y_axis, 0, 20, 0.0, 1.0);
        vec4 new_position = pos2 * val + (1.0 - val) * pos1;
        gl_Position = new_position;
        gl_PointSize = 4;
        EmitVertex();
        EndPrimitive();
   } else {
        int div = 50;
        if (x_axis < div) {
            gl_PointSize =  5;
            val = translate(x_axis, 0, div, 0.0, 1.0);
            alpha = translate(x_axis, 20, div, 0.0, 1.0);
        } else {
            pos1 = vec4(Vskeleton_vx[i].xyz, 1.0);
            pos2 = vec4(V_center[i].xyz, 1.0);
            gl_PointSize =  translate(x_axis, div, 100, 5, 15);
            val = translate(x_axis, div, 100, 0.0, 1.0);
            alpha = 1.0;
        }

        color_intp = translate(x_axis, 0, 20, 0.0, 1.0);
        vec4 new_position = pos2 * val + (1.0 - val) * pos1;
        gl_Position = new_position;
        EmitVertex();
        EndPrimitive();
   }
}


