#version 430

out vec4  outcol;

void main() {
    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;
    else
        outcol = vec4( 0.690, 0.878, 0.902, 1.0 );
}
