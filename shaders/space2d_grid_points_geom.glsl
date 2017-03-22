#version 430

out vec4 color_val;
out float alpha;


layout (points) in;
layout (points, max_vertices = 1) out;

void main() {
    gl_PointSize = 5;
    for (int i = 0; i < 1; ++i) {
        color_val = vec4(0.8, 0.8, 0.8, 1);
        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
    }

    EndPrimitive();

}
