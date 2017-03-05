#version 430

in vec4  color_val;
in float alpha;
out vec4 outcol;

in vec2 v_start;
in vec2 v_line;
in float v_l2;

void main() {
    float t = dot(gl_FragCoord.xy - v_start, v_line) / v_l2;
    vec2 projection = v_start + clamp(t, 0, 1) * v_line;
    vec2 delta = gl_FragCoord.xy - projection;
    float d2 = dot(delta, delta);
    float k = clamp(5 - d2,  0.0, 1.0);
    float endWeight = step(abs(t * 2 - 1), 1);

    outcol = color_val;
    outcol.a = mix(k, 1, endWeight);
    if (outcol.a  < 0.1)
        discard;
}
