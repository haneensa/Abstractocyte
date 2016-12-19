#version 150
out vec4  outcol;

void main() {
    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;
     else if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.16)
       outcol = vec4( 0.0, 0.502, 0.502, 1.0 );
     else
        outcol = vec4( 0.0, 0.545, 0.545, 1.0 );
}
