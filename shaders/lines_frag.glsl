#version 430

in vec4  color_val;
out vec4 outcol;

void main() {
    outcol = color_val;
    outcol.a = 1.0;
}
