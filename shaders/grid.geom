#version 430

layout (points) in;
//layout (line_strip, max_vertices = 2) out;
layout (points, max_vertices = 1) out;

void build_grid(vec4 position)
{
    // if x == 0, then add to the x position
    // else if y == 0, then add to the y position
    if (position.x <= 0) {
    gl_Position = position;
    EmitVertex();
    gl_Position = position + vec4(1.0, 0.0, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
    } else {
    gl_Position = position;
    EmitVertex();
    gl_Position = position + vec4(0.0, 1.0, 0.0, 0.0);
    EmitVertex();
    EndPrimitive();
    }
}

void main() {
    //build_grid(gl_in[0].gl_Position);
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 10;
    EmitVertex();
    EndPrimitive();
}
