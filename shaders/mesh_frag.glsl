#version 430
in vec3         normal_out;

in float        alpha;
in float        color_intp;
in vec4         color_val;
in vec3			vposition;
in vec3			eye;
out vec4        outcol;

// textures
uniform sampler3D   astro_tex;
uniform sampler3D   gly_tex;
uniform sampler3D   mito_tex;

in vec3             G_fragTexCoord;

uniform sampler1D   tf;

//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
uniform vec3 diffuseLightDirection; //QVector3D(-2.5f, -2.5f, -0.9f);


// ------------------- TOON SHADER PROPERTIES ----------------------
// vec3 lineColor = vec4(0.0, 0.0,  0.0, 1.0); -> color to draw the lines in  (black)
// float lineThickness = 0.03

vec3 lightDir2 = vec3(2.5f, 2.5f, 1.0f);
//vec3 E = vec3(0.5, 0.5, -1.0);
vec3 N = normalize(normal_out);
vec3 L = normalize(diffuseLightDirection);
vec3 L2 = normalize(lightDir2);

float ambiance = 0.3;

void main() {
	vec3 E = normalize(eye);
	//N.z = -1 * N.z;
	float cosTheta = clamp(dot(N, L), 0, 1);
	float cosTheta2 = clamp(dot(N, L2), 0, 1);

	
	float intensity = clamp(max(cosTheta, cosTheta2), 0, 1);
	vec3 V = normalize(E - vposition);
	//vec3 E = normalize(E);
	vec3 H = normalize(L + E);
	float sf = max(0.0, dot(N, H));
	sf = pow(sf, 3.0);

	vec4 color = vec4(color_val.rgb, 1.0);

        // mark astrocytes on neurites
//        vec4 astro_volume = texture(astro_tex, G_fragTexCoord);
//        if (astro_volume.r > 0) {
//            color = texture(tf, astro_volume.r);
//        }

//        vec4 gly_volume = texture(gly_tex, G_fragTexCoord);
//        if (gly_volume.r > 0) {
//            color = texture(tf, gly_volume.r);
//        } else {
//            color = vec4(1, 1, 1, 0.5);
//        }


        // mark astrocytes on neurites
        vec4 mito_volume = texture(mito_tex, G_fragTexCoord);
        if (mito_volume.r > 0) {
            color = texture(tf, mito_volume.r);
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
	float edgeDetection = (border_value > 0.05) ? 1 : 0;
	// interpolate between two colors
	// todo: based on the mesh type (astro, neurite)
        outcol = phong_color * color_intp + (1.0 - color_intp) /** edgeDetection*/ * toon_color;
	float al = 0;
	if (alpha < 1.0 && edgeDetection < 0.5)
	{
		al = max(1.0 - border_value, alpha);
	}
	else
	{
		al = alpha;
	}

//        if (gly_volume.r == 0) {
//            color.a = 0.5;
//        } else
          outcol.a = alpha;

}
