#version 150

out vec4  outcol;
in vec4 colAttrg;

void main() {
    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;

    else if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.16)
    {
        outcol = vec4( 1.0, 0.0, 0.0, 0.0 );
    }
     else
        outcol = vec4( 1.0, 0.0, 0.0, 0.0 );
}
