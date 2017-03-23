#version 430

out vec4 color_val;
out float alpha;


layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main() {
    gl_PointSize = 5;
    for (int i = 0; i < 3; ++i) {
        color_val = vec4(0.96, 0.96, 0.96, 1);
        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
    }

    EndPrimitive();

}
