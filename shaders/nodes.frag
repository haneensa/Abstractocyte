#version 430

out vec4 outcol;

void main() {
    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
       discard;
    else if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.16)
    {
       outcol = vec4( 0.0, 0.502, 0.502, 1.0 );

       vec2 pos = mod(outcol.xy, vec2(50.0)) - vec2(25.0);

       float dist_squared = dot(pos, pos);

       outcol = mix(vec4(.0, .510, .510, 1.0), vec4(.0, .502, .502, 1.0),
                          smoothstep(380.25, 420.25, dist_squared));
    }
     else
        outcol = vec4( 0.0, 0.545, 0.545, 1.0 );
}
