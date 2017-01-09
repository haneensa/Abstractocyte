#version 150
out vec4        outcol;
in vec4         posAttrG;
uniform vec3    color;
void main() {
    outcol = vec4(color.rgb, 1.0);
}
