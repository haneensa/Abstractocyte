#version 430

in float                node2D_alpha; /*1 -> 3D, 0 -> 2D*/
in float                alpha;
in vec4                 color_val;
flat in int             hasMito;
in vec3                 G_fragTexCoord;

out vec4                outcol;

float Ns                = 250;
vec4  mat_specular      = vec4(1);
vec4  light_specular    = vec4(1);
vec3  lightDir          = vec3(0.5, 0.0, -0.9);

uniform sampler3D   splat_tex; //r=astro g=astro-mito b=neurite-mito
uniform sampler3D   gly_tex;
uniform ivec4     splat_flags;
uniform sampler1D	gly_tf;

float triangleArea(vec2 p1, vec2 p2, vec2 p3)
{
    float area = 0.5 * (p1.x * (p2.y - p3.y) +
                        p2.x * ( p3.y - p1.y) +
                        p3.x * (p1.y - p2.y));
    return abs(area);
}

void main() {
    if (  hasMito == 1 && node2D_alpha == 0 )  {
        vec4 node2d = vec4(color_val.rgb, alpha);
        vec3 normal;
        normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
        float mag = abs(normal.x) + abs(normal.y);
        if (mag > 1.0) discard;
        if (mag > 0.8) node2d -= vec4( 0.3, 0.3, 0.3, 1.0 );
        outcol = node2d;
    } else {
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

        outcol = mix( node2d, node3d, node2D_alpha);

        if ( node2D_alpha > 0 ) {
            if (splat_flags.w > 0.0) {
                float tex_splat = texture(splat_tex, G_fragTexCoord).g;
                if (tex_splat > 0) {
                    vec4 mito_color = vec4(0.482f, 0.408f, 0.933f, 1);
                    outcol = mix(outcol, mito_color, tex_splat);
                }
            }

            if (splat_flags.y > 0.0) {
                float gly_splat = texture(gly_tex, G_fragTexCoord).r;
                if (gly_splat > 0) {
                    vec4 pnk_color = texture(gly_tf, gly_splat * 10.0);
                    outcol =  mix( outcol, pnk_color,  gly_splat * 10.0);
                }
            }
        }
    }



    outcol.a = alpha;
}
