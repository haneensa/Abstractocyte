#version 150
in vec4 posAttrV[];
out vec4 posAttrG;

out vec3 normal_out;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main() {
    // precompute this once?
    // face normal per triangle
    vec3 A = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 B = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    normal_out = normalize(cross(A,B));

  for(int i = 0; i < 3; i++) {
    posAttrG = posAttrV[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}

