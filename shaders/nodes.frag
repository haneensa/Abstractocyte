#version 430

out vec4 outcol;

float           Ns = 250;
vec4            mat_specular = vec4(1);
vec4            light_specular = vec4(1);


void main() {
//    if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
//       discard;
//    else if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.16)
//    {
//       outcol = vec4( 0.0, 0.502, 0.502, 1.0 );

//       vec2 pos = mod(outcol.xy, vec2(50.0)) - vec2(25.0);

//       float dist_squared = dot(pos, pos);

//       outcol = mix(vec4(.0, .510, .510, 1.0), vec4(.0, .502, .502, 1.0),
//                          smoothstep(380.25, 420.25, dist_squared));
//    }
//     else
//        outcol = vec4( 0.0, 0.545, 0.545, 1.0 );

    vec4 color_val =  vec4( 0.0, 0.545, 0.545, 1.0 );
    vec3 lightDir =  vec3(0.5, 0.0, -0.9);
    vec3 normal;
    normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;
    normal.z = -sqrt(1.0-mag);
    normal = normalize(normal);
    float diffuse = clamp( dot(normal, lightDir), 0, 1);
    vec3 eye = vec3(0.0, 0.0, 1.0);
    vec3 halfVector = normalize( eye + lightDir );
    float spec = max( pow( dot(normal, halfVector), Ns ), 0. );
    vec4 S = light_specular * mat_specular * spec;
    outcol = color_val  * diffuse + S;
}
