#version 150

out vec4  outcol;
in vec4 posAttrG;

void main() {
    outcol = posAttrG;
}
