#version 430

layout (lines) in;
layout (line_strip, max_vertices = 2) out;

// ID for each line from 0 until n where n is number of lines there
// access buffer, and get control points for the curves

/*
double bezier(float A, float B, float C, float D, float t)
{
    float s = 1 - t;
    float AB = A * s + B * t;
    float BC = B * s + C * t;
    float CD = C * s + D * t;
    float ABC = AB * s + BC * t;
    flaot BCD = BC * s + CD * t;
    return ABC * s + BCD * t;
}*/

void main() {
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = gl_in[0].gl_PointSize;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    gl_PointSize = gl_in[1].gl_PointSize;
    EmitVertex();
    EndPrimitive();
}
