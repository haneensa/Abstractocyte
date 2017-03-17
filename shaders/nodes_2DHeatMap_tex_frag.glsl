#version 430

out vec4            outcol;

//layout (location = 1) uniform float       ResS;
//layout (location = 2) uniform float       ResT;
uniform sampler2D   tex;
uniform sampler1D   tf;

in vec2             G_fragTexCoord;

void main(void)
{
    float ResS = 240.0;
    float ResT = 240.0;

    vec3 irgb = texture2D(tex, G_fragTexCoord).rgb;
    vec2 stp0 = vec2(1./ResS, 0.); // texel offsets
    vec2 st0p = vec2(0., 1./ResT);
    vec2 stpp = vec2(1./ResS, 1./ResT);
    vec2 stpm = vec2(1./ResS, -1./ResT);

    // 3 x 3 pixel colors next

    vec3 i00 = texture2D(tex, G_fragTexCoord).rgb;
    vec3 im1m1 = texture2D(tex, G_fragTexCoord - stpp ).rgb;
    vec3 ip1p1 = texture2D(tex, G_fragTexCoord + stpp ).rgb;
    vec3 im1p1 = texture2D(tex, G_fragTexCoord - stpm ).rgb;
    vec3 ip1m1 = texture2D(tex, G_fragTexCoord + stpm).rgb;
    vec3 im10 = texture2D(tex, G_fragTexCoord - stp0 ).rgb;
    vec3 ip10 = texture2D(tex, G_fragTexCoord + stp0 ).rgb;
    vec3 i0m1 = texture2D(tex, G_fragTexCoord - st0p ).rgb;
    vec3 i0p1 = texture2D(tex, G_fragTexCoord + st0p ).rgb;

    vec3 target = vec3(0., 0., 0.);
    target += 1. * (im1m1 + ip1m1, + ip1p1, + im1p1); // apply blur filter
    target += 2. * (im10 + ip10 + i0m1 + i0p1);
    target += 4.*(i00);
    target /= 16.;

    float t = target.r;
    vec4 t_color = texture(tf, t);

    //outcol = vec4( target, 1. );
    outcol = t_color;
    outcol.b = target.r;
}
