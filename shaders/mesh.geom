#version 430

in vec4         Vskeleton_vx[];
vec4            pos2;

in  int         V_ID[];
in  vec4        V_color[];
in  vec4        V_center[];

out float       G_ID;
out float       color_intp;
out vec4        color_val;

out vec3        normal_out;
out float       alpha;
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


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
  // precompute this once?
  // face normal per triangle
  vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  normal_out = normalize(cross(A,B));

  for(int i = 0; i < 3; i++) {
    G_ID = float(V_ID[i]);
    color_val = V_color[i];
    float val;
    vec4 pos1 = gl_in[i].gl_Position;
    pos2 = vec4(Vskeleton_vx[i].xyz, 1.0);

    if (G_ID <= 0.0) {
        val = translate(y_axis, 20, 100, 0.0, 1.0);
        alpha =  translate(y_axis, 20, 30, 1.0, 0.0);
        color_intp = translate(y_axis, 0, 20, 0.0, 1.0);
        if (alpha <= 0.5)
            break;
    } else {
        int div = 50;
        if (x_axis < div) {
            pos2 = vec4(Vskeleton_vx[i].xyz, 1.0);
            val = translate(x_axis, 20, div, 0.0, 1.0);
            alpha = translate(x_axis, 40, div, 1.0, 0.0);
        } else {
            pos1 = vec4(Vskeleton_vx[i].xyz, 1.0);
            pos2 = vec4(V_center[i].xyz, 1.0);
            val = translate(x_axis, div, 100, 0.0, 1.0);
            alpha = 0.0;
        }

        color_intp = translate(x_axis, 0, 20, 0.0, 1.0);

        if (alpha <= 0.8)
            break;

    }

    vec4 new_position = mix(pos1 , pos2, val);
    gl_Position = new_position;
    EmitVertex();
  }

  EndPrimitive();
}


