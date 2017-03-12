#version 430

in float        G_ID;
out vec4        outcol;

void main() {
    float blue = int(G_ID)%256;
    float grean = int(G_ID / 256) % 256;
    int red = int(G_ID / 65536) % 256;

    // deal with parent and child overlapping IDs
    outcol =   vec4(blue/255.0, grean/255.0, 0, 1.0);
}
