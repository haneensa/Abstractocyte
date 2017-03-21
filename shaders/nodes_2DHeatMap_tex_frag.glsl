#version 430

out vec4            outcol;


uniform vec2        dim;

uniform sampler2D   tex;// nodes
uniform sampler1D   tf;

in vec2             G_fragTexCoord;

const float gaussian_steps[5] = float[5](0.0, 1.0, -1.0, 2.0, -2.0);

const float gaussian_kernel[25] = float[25](0.0598, 0.0379, 0.0379, 0.0102, 0.0102,
        0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
        0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
        0.0102, 0.0058, 0.0058, 0.0014, 0.0014,
        0.0102, 0.0058, 0.0058, 0.0014, 0.0014);

const float gaussian_kernel12[25] = float[25](	0.040804, 0.040601, 0.040601, 	0.039997,  0.039997,
        0.040601, 0.040399, 0.040399, 0.039798, 0.039798,
        0.040601, 0.040399, 0.040399, 0.039798, 0.039798,
        0.039997, 0.039798, 0.039798, 0.039206, 0.039206,
        0.039997, 0.039798, 0.039798, 0.039206, 0.039206);

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

    for (int j = 0; j < 5; j++)
    {
        idx_offset.y = gaussian_steps[j] * stepXY.y; //step_y
        for (int i = 0; i < 5; i++)
        {
            idx_offset.x = gaussian_steps[i] * stepXY.x; //step_x
            weight = gaussian_kernel12[i + j * 5];
            vec2 new_coord = coord.xy + idx_offset.xy;
            vec4 tex_value4 = texture(texture_toSplat, new_coord);
            result = result + (weight * tex_value4.r);
        }
   }
	
    return result;
}
