#version 430
#define NMITO 1
#define AXONS 2
#define BOUTN 3
#define DENDS 4
#define SPINE 5
#define ASTRO 6
#define SYNPS 7
#define AMITO 9

precision highp float;
in vec3         normal_out;
in float        alpha;
in float        color_intp;
in vec4         color_val;
in vec3			vposition;
in vec3			eye;
flat in int			otype;
layout (location = 0) out vec4        outcol;
// textures
uniform sampler3D   splat_tex; //r=astro g=astro-mito b=neurite-mito
uniform sampler3D   gly_tex;

in vec3             G_fragTexCoord;
//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
vec3 diffuseLightDirection = vec3(-2.5f, -2.5f, -0.9f);
vec3 lightDir2 = vec3(2.5f, 2.5f, 1.0f);
vec3 N = normalize(normal_out);
vec3 L = normalize(diffuseLightDirection);
vec3 L2 = normalize(lightDir2);
float ambiance = 0.3;
//-------------------- SPLATTING -----------------------------------
const float gaussian_steps[5] = float[5](0.0,1.0, -1.0,2.0, -2.0);
const float gaussian_kernel[125] = float[125](0.0598, 0.0379, 0.0379, 0.0102, 0.0102,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0102, 0.0058, 0.0058, 0.0014, 0.0014,
	0.0102, 0.0058, 0.0058, 0.0014, 0.0014,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0058, 0.0014, 0.0014, 0, 0,
	0.0058, 0.0014, 0.0014, 0, 0,
	0.0379, 0.0233, 0.0233, 0.0058, 0.0058,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0058, 0.0014, 0.0014, 0, 0,
	0.0058, 0.0014, 0.0014, 0, 0,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0102, 0.0058, 0.0058, 0, 0,
	0.0102, 0.0058, 0.0058, 0, 0,
	0, 0, 0.0014, 0, 0,
	0, 0, 0.0014, 0, 0,
	0.0233, 0.0102, 0.0102, 0.0014, 0.0014,
	0.0102, 0.0058, 0.0058, 0, 0,
	0.0102, 0.0058, 0.0058, 0, 0,
	0, 0, 0.0014, 0, 0,
	0, 0, 0.0014, 0, 0);
//float getSplattedTexture(in sampler3D texture_toSplat, in vec3 coord, in vec3 step_size, in int tex_idx);
vec3 getSplattedTexture3(in sampler3D texture_toSplat, in vec3 coord, in vec3 step_size);
vec4 red_color = vec4(1.0, 0.0, 0.0, 1.0); //astro
vec4 blu_color = vec4(0.21, 0.56, 0.75, 1.0); //mito
vec4 pnk_color = vec4(0.97, 0.4, 0.63, 1.0);//glyco
vec4 lpnk_color = vec4(0.99, 0.88, 0.86, 1.0);
vec4 dpnk_color = vec4(0.29, 0.0, 0.42, 1.0);
vec4 pur_color = vec4(0.33, 0.15, 0.56, 1.0);
// ------------------- TOON SHADER PROPERTIES ----------------------
//vec3 E = vec3(0.5, 0.5, -1.0);
void main() {
	vec3 E = normalize(eye);
	float cosTheta = clamp(dot(N, L), 0, 1);
	float cosTheta2 = clamp(dot(N, L2), 0, 1);
	float intensity = clamp(max(cosTheta, cosTheta2), 0, 1);
	vec3 V = normalize(E - vposition);
	vec3 H = normalize(L + E);
	float sf = max(0.0, dot(N, H));
	sf = pow(sf, 3.0);
	vec4 color = vec4(color_val.rgb, 1.0);
	vec4 mix_color = vec4(1.0, 0.0, 0.0, 1.0);
	vec3 splat = getSplattedTexture3(splat_tex, G_fragTexCoord, vec3(0.001, 0.001, 0.002));

	switch (otype)
	{
	case ASTRO:
		mix_color = mix(color, pur_color, splat.b);
		color = mix_color;
		break;
	case AXONS:
	case BOUTN:
	case DENDS:
	case SPINE:
	case SYNPS:
		mix_color = mix(color, red_color, splat.r);
		color = mix_color;
		mix_color = mix(color, blu_color, splat.g);
		color = mix_color;
		break;
	}
	
	//float splat2 = texture(gly_tex, G_fragTexCoord).r; //getSplattedTexture(gly_tex, G_fragTexCoord, vec3(0.001), 0);
	
	vec4 toon_color = vec4(color.rgb, 1.0);
	vec4 diffuse_color = max((color * cosTheta2), (color * cosTheta));
	vec4 phong_color = (ambiance *  color) + diffuse_color;// (color * cosTheta2);//mix(vec4(color.rgb, 1.0) * cosTheta, vec4(color.rgb, 1.0) * cosTheta2, 0.3);

	if (intensity > 0.8)
		toon_color = vec4(1.0, 1.0, 1.0, 1.0) *toon_color;
	else if (intensity > 0.25)
		toon_color = vec4(0.7, 0.7, 0.7, 1.0) *toon_color;
	else if (intensity > 0.05)
		toon_color = vec4(0.35, 0.35, 0.35, 1.0) *toon_color;
	else
		toon_color = vec4(0.1, 0.1, 0.1, 1.0) *toon_color;
	//borders
	//float border_value = abs(dot(V, N));
	//float edgeDetection = (border_value > 0.02) ? 1 : 0;
	// interpolate between two colors
    outcol = phong_color * color_intp + (1.0 - color_intp) /* edgeDetection*/ * toon_color;
	
    outcol.a = alpha;

}

vec3 getSplattedTexture3(in sampler3D texture_toSplat, in vec3 coord, in vec3 step_size)
{
	vec3 idx_offset = vec3(0.0, 0.0, 0.0);
	float weight = 1;
	float tex_value = 1.0;
	float sum = 0;
	vec3 result = vec3(0);
	for (int k = 0; k < 5; k++)
	{
		idx_offset.z = gaussian_steps[k] * step_size.z;
		for (int j = 0; j < 5; j++)
		{
			idx_offset.y = gaussian_steps[j] * step_size.y;
			for (int i = 0; i < 5; i++)
			{
				idx_offset.x = gaussian_steps[i] * step_size.x;
				weight = gaussian_kernel[i + j * 5 + k * 25];
				vec3 new_coord = coord.xyz + idx_offset.xyz;
				vec4 tex_value4 = texture(texture_toSplat, new_coord);
				result = result + (weight * tex_value4.rgb);
			}
		}
	}
	return result;
}

