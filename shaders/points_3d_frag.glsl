#version 430

in float        node2D_alpha; /*1 -> 3D, 0 -> 2D*/
in float        alpha;
in vec4         color_val;

out vec4        outcol;

float           Ns = 250;
vec4            mat_specular = vec4(1);
vec4            light_specular = vec4(1);

void main() {
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
        vec4 node3d = vec4(color_val.rgb, alpha) * diffuse + S;
        vec4 node2d = vec4(color_val.rgb, alpha);

        if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.20)
        {
           node2d -= vec4( 0.3, 0.3, 0.3, 1.0 );
        }

        // if has mitochondria then render it with special symbol

        outcol = mix( node2d, node3d, node2D_alpha);
        outcol.a = alpha;
}
