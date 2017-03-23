#version 430


layout(location = 0) out vec4        outcol;

in vec4 color_val;


void main() {



    vec3 normal;
    normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
    float mag = dot(normal.xy, normal.xy);
    if (mag > 1.0) discard;

    float a = 1.0 - mag;
    outcol = vec4(color_val.rgba) * a;
}
