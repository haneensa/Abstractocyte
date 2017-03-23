#version 430

out vec4 color_val;
out float alpha;



void main() {
    gl_PointSize = 5;
    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
    }

    EndPrimitive();

}
