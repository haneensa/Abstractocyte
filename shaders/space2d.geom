#version 430

in int V_ID[];
out float G_ID;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// todo: ssbo, based on the ID we get interval and special color for debuging
void main() {
    for(int i = 0; i < 3; i++) {
        G_ID = float(V_ID[i]);
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
