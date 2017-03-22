#version 430

out vec4 color_val;
out float alpha;


layout (lines) in;
layout (line_strip, max_vertices = 2) out;

void main() {

    gl_PointSize = 5;

    for (int i = 0; i < 2; ++i) {
        color_val = vec4(0.8, 0.8, 0.8, 1);
        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
    }

    EndPrimitive();

}
