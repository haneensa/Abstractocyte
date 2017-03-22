#version 430

out vec4            outcol;


uniform vec2        dim;

uniform sampler2D   tex;// nodes
uniform sampler1D   tf;

in vec2             G_fragTexCoord;

const float gaussian_steps[7] = float[7](0.0, 1.0, -1.0, 2.0, -2.0, 3.0, -3.0);

const float gaussian_kernel15[49] = float[49](
            0.020773, 0.020727, 0.020727,  0.020589, 0.020589, 0.020362, 0.020362,
            0.020727, 0.020681, 0.020681,  0.020543, 0.020543, 0.020316, 0.020316,
            0.020727, 0.020681, 0.020681,  0.020543, 0.020543, 0.020316, 0.020316,
            0.020589	, 0.020543, 0.020543,  0.020407, 0.020407, 0.020182, 0.020182,
            0.020589	, 0.020543, 0.020543,  0.020407, 0.020407, 0.020182, 0.020182,
            0.020362, 0.020316, 0.020316,  0.020182, 0.020182, 0.019959, 0.019959,
            0.020362, 0.020316, 0.020316,  0.020182, 0.020182, 0.019959, 0.019959
 );

float getSplattedTexture(in sampler2D texture_toSplat, in vec2 coord);

void main() {
    float splat = getSplattedTexture(tex, G_fragTexCoord);
    vec4 red_color = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 t_color = texture(tf, splat);
    vec4 mix_color = mix(t_color, red_color, splat);
    outcol = mix_color;
}

float getSplattedTexture(in sampler2D texture_toSplat, in vec2 coord)
{
    vec2 stepXY = 1.0/dim;
    // step_x; step_y
    vec2 idx_offset = vec2(0.0, 0.0);
    float weight = 1;
    float tex_value = 1.0;
    float sum = 0;
    float result = 0;

    for (int j = 0; j < 7; j++)
    {
        idx_offset.y = gaussian_steps[j] * stepXY.y; //step_y
        for (int i = 0; i < 7; i++)
        {
            idx_offset.x = gaussian_steps[i] * stepXY.x; //step_x
            weight = gaussian_kernel15[i + j * 7];
            vec2 new_coord = coord.xy + idx_offset.xy;
            vec4 tex_value4 = texture(texture_toSplat, new_coord);
            result = result + (weight * tex_value4.r);
        }
   }

    return result;
}
