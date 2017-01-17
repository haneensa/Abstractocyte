#version 330 core

in vec4 Vskeleton_vx[];
out vec4 Gskeleton_vx;

in int V_ID[];
out float G_ID;

out vec4        color_val;
out float   color_intp;
out vec3 normal_out;
out float alpha;

layout(triangles) in;
layout(points, max_vertices = 1) out;

uniform int     y_axis;
uniform int     x_axis;
uniform int     state;

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
    int i = 0;
      G_ID = float(V_ID[i]);
      Gskeleton_vx = vec4(Vskeleton_vx[i].xyz, 1.0);
      float val;
      if (G_ID <= 0.0) {
          val = translate(y_axis, 20, 100, 0.0, 1.0);
          alpha = translate(y_axis, 40, 60, 0.0, 1.0);
          color_intp = translate(x_axis, 0, 20, 0.0, 1.0);
          color_val = vec4(1.0,  0.0, 0.0, 1.0);
          vec4 new_position = mix(gl_in[i].gl_Position , Gskeleton_vx, val);
          gl_Position = new_position;
          gl_PointSize = 3;
          EmitVertex();
          EndPrimitive();
      }
}


