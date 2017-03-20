#version 430

out vec4 color_val;
out float alpha;


layout (triangles) in;
layout (points, max_vertices = 3) out;

void main() {

    gl_PointSize = 10;

    for(int i = 0; i < 3; i++) {
        color_val = vec4(1, 0, 0, 1);
        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
   }

    EndPrimitive();

}
