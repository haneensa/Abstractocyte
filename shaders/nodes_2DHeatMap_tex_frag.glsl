#version 430

out vec4            outcol;

//declare uniforms
uniform float       resolution;
uniform float       radius;
uniform vec2        dir;

uniform sampler2D   tex;
uniform sampler1D   tf;
uniform sampler3D   astro_tex;

in vec2             G_fragTexCoord;

const float gaussian_steps[5] = float[5](0.0, 1.0, -1.0, 2.0, -2.0);

const float gaussian_kernel[125] = float[125](0.0598, 0.0379, 0.0379, 0.0102, 0.0102,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0102, 0.0058, 0.0058, 0.0014, 0.0014,
	0.0102, 0.0058, 0.0058, 0.0014, 0.0014);

void main() {
    //this will be our RGBA sum
    vec4 sum = vec4(0.0);

    //our original texcoord for this fragment
    vec2 tc = G_fragTexCoord;

    //the amount to blur, i.e. how far off center to sample from
    //1.0 -> blur by one pixel
    //2.0 -> blur by two pixels, etc.
    float blur = radius/resolution;

    //the direction of our blur
    //(1.0, 0.0) -> x-axis blur
    //(0.0, 1.0) -> y-axis blur
    float hstep = dir.x;
    float vstep = dir.y;

    //apply blurring, using a 9-tap filter with predefined gaussian weights

    sum += texture2D(tex, vec2(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep)) * 0.0162162162;
    sum += texture2D(tex, vec2(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep)) * 0.0540540541;
    sum += texture2D(tex, vec2(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep)) * 0.1216216216;
    sum += texture2D(tex, vec2(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep)) * 0.1945945946;

    sum += texture2D(tex, vec2(tc.x, tc.y)) * 0.2270270270;

    sum += texture2D(tex, vec2(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep)) * 0.1945945946;
    sum += texture2D(tex, vec2(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep)) * 0.1216216216;
    sum += texture2D(tex, vec2(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep)) * 0.0540540541;
    sum += texture2D(tex, vec2(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep)) * 0.0162162162;

    //discard alpha for our simple demo, multiply by vertex color and return
    float t = sum.r;
    vec4 t_color = texture(tf, t);

    vec4 volume_color = texture(astro_tex, vec3(G_fragTexCoord, 0.5));
  //  outcol = vec4(t_color.rgba);
    outcol = volume_color;
    outcol.a = 1;

}

float getSplattedTexture(in sampler2D texture_toSplat, in vec2 coord)
{
	float step = 0.001; //change later 1/size_x .. 1/size_y
	// step_x; step_y

	vec2 idx_offset = vec3(0.0, 0.0);
	float weight = 1;
	float tex_value = 1.0;
	float sum = 0;
	float result = 0;

		for (int j = 0; j < 5; j++)
		{
			idx_offset.y = gaussian_steps[j] * step; //step_y
			for (int i = 0; i < 5; i++)
			{
				idx_offset.x = gaussian_steps[i] * step; //step_x
				weight = gaussian_kernel[i + j * 5];
				vec2 new_coord = coord.xy + idx_offset.xy;
				vec4 tex_value4 = texture(texture_toSplat, new_coord);
				//sum = sum + (0.008 * tex_value);
				result = result + (weight * tex_value4.r);
			}
		}
	
	return result;
}