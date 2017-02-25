#version 430

in vec4  color_val;
in float alpha;
out vec4 outcol;

void main() {
    outcol = color_val;
    outcol.a = alpha;
}
