#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 V_fragTexCoord[];
out vec2 G_fragTexCoord;


void main() {
    for(int i = 0; i < 3; i++) { // You used triangles, so it's always 3
      gl_Position = gl_in[i].gl_Position;
      G_fragTexCoord = V_fragTexCoord[i];
      EmitVertex();
    }
    EndPrimitive();
}
