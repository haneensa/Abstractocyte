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
out vec4        outcol;
// textures
uniform sampler3D   astro_tex;
uniform sampler3D   gly_tex;
uniform sampler3D   mito_tex;

in vec3             G_fragTexCoord;

//uniform sampler1D   tf;

//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
uniform vec3 diffuseLightDirection; //QVector3D(-2.5f, -2.5f, -0.9f);
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
float getSplattedTexture(in sampler3D texture_toSplat, in vec3 coord);
vec4 red_color = vec4(1.0, 0.0, 0.0, 1.0); //astro
vec4 blu_color = vec4(0.21, 0.56, 0.75, 1.0); //mito
vec4 pnk_color = vec4(0.968, 0.4, 0.63, 1.0);//glyco

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
	
	if (otype != ASTRO) //add flag1 if enabled too
	{
		float splat = getSplattedTexture(astro_tex, G_fragTexCoord);
		vec4 mix_color = mix(color, red_color, splat);
		mix_color.a = 1;
		color = mix_color;
	}

	if (otype != AMITO && otype != NMITO) //add flag2 if enabled too
	{
		float splat = getSplattedTexture(mito_tex, G_fragTexCoord);
		vec4 mix_color = mix(color, blu_color, splat);
		mix_color.a = 1;
		color = mix_color;
	}

	if (otype == NMITO || otype == SPINE || otype == BOUTN) //add flag3 if enabled too
	{
		float splat = getSplattedTexture(gly_tex, G_fragTexCoord);
		vec4 mix_color = mix(color, pnk_color, splat);
		mix_color.a = 1;
		color = mix_color;
	}
	
		
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
	float border_value = abs(dot(V, N));
	float edgeDetection = (border_value > 0.02) ? 1 : 0;
	// interpolate between two colors
	// todo: based on the mesh type (astro, neurite)
    outcol = phong_color * color_intp + (1.0 - color_intp) /* edgeDetection*/ * toon_color;
	//float al = 0;
	//if (alpha < 1.0 && edgeDetection < 0.5)
	//{
	//	al = max(1.0 - border_value, alpha);
	//}
	//else
	//{
	//	al = alpha;
	//}
    outcol.a = alpha;

}

float getSplattedTexture(in sampler3D texture_toSplat, in vec3 coord)
{
	float step = 0.001; //change later

	vec3 idx_offset = vec3(0.0, 0.0, 0.0);
	float weight = 1;
	float tex_value = 1.0;
	float sum = 0;
	float result = 0;
	for (int k = 0; k < 5; k++)
	{
		idx_offset.z = gaussian_steps[k] * step;
		for (int j = 0; j < 5; j++)
		{
			idx_offset.y = gaussian_steps[j] * step;
			for (int i = 0; i < 5; i++)
			{
				idx_offset.x = gaussian_steps[i] * step;
				weight = gaussian_kernel[i + j * 5 + k * 25];
				vec3 new_coord = coord.xyz + idx_offset.xyz;
				vec4 tex_value4 = texture(texture_toSplat, new_coord);
				//sum = sum + (0.008 * tex_value);
				result = result + (weight * tex_value4.r);
			}
		}
	}
	return result;
}

