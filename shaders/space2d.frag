#version 430

in float G_ID;

out vec4        outcol;

void main() {
    if (G_ID == 0.0)
        outcol = vec4( 0.0, 0.545, 0.405, 1.0 );
    else if (G_ID == 1.0)
        outcol = vec4( 1, 0.545, 0.545, 1.0 );
    else
        outcol = vec4( 0, 0.9, 0.545, 1.0 );

}
