#version 430
in vec4         Gskeleton_vx;
in vec3         normal_out;
in float        G_ID;

in float        alpha;
in float        color_intp;
in vec4         color_val;
out vec4        outcol;

uniform vec3 diffuseLightDirection;

float cosTheta = clamp( dot( normal_out, diffuseLightDirection ), 0, 1 );
float intensity = dot(diffuseLightDirection, normal_out);

float Ns = 250;
vec4 mat_specular = vec4(1);
vec4 light_specular = vec4(1);

void main() {
    vec3 normal;
    normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;
    normal.z = -sqrt(1.0-mag);

    float diffuse = clamp( dot(normal, diffuseLightDirection), 0, 1);
    vec3 eye = vec3(0.0, 0.0, 1.0);
    vec3 halfVector = normalize( eye + diffuseLightDirection );
    float spec = max( pow( dot(normal, halfVector), Ns ), 0. );
    vec4 S = light_specular * mat_specular * spec;
    outcol = vec4(color_val.rgb, 1.0) * diffuse + S;
    outcol.a = alpha;
}
