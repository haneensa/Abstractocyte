#version 150
in vec4 posAttrV[];
out vec4 posAttrG;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main() {
  for(int i = 0; i < 3; i++) {
    posAttrG = posAttrV[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}

