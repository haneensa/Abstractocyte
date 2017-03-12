#version 430

layout(location = 9) uniform int   max_astro_coverage;


in float        alpha;
in vec4         color_val;
in float        G_ID;

out vec4        outcol;

float           Ns = 250;
vec4            mat_specular = vec4(1);
vec4            light_specular = vec4(1);


struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

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
        outcol = vec4(color_val.rgb, alpha) * diffuse + S;

        if (color_val.w <= 0.0001) {
            float coverage = SSBO_data[int(G_ID)].info.y / float(max_astro_coverage);
            // darken this color or make it lighter?
            if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.25)
               discard;
            else if (dot(gl_PointCoord-0.5,gl_PointCoord-0.5)>0.14)
            {
               vec2 pos = mod(outcol.xy, vec2(50.0)) - vec2(25.0);

               float dist_squared = dot(pos, pos);

               outcol = mix(outcol, vec4(0.1, 0.0, 0.1, 1),
                                  smoothstep(380.25, 420.25, dist_squared));
            }

        }


        outcol.a = alpha;

}
