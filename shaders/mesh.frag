#version 150

out vec4  outcol;
in vec4 posAttrG;

void main() {
    //outcol = posAttrG;
    outcol = vec4(posAttrG.r, 0.5, posAttrG.b, 1.0);
}
