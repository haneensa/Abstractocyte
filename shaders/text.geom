#version 330 core

in vec2 vertUV[];
out vec2 vertUVg;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

void main() {
  for(int i = 0; i < 3; i++) { // You used triangles, so it's always 3
    gl_Position = gl_in[i].gl_Position;
    vertUVg = vertUV[i];
    EmitVertex();
  }
  EndPrimitive();
}

