#version 430

in	int		V_ID[];
in	int		C_ID[];
in  int		V_State[];
in  vec2	V_Radius_volume[];


layout (points) in;
layout (points, max_vertices = 1) out;

out float        alpha;
out vec4         color_val;
out float        node2D_alpha; /*1 -> 3D, 0 -> 2D*/

struct properties {
    vec2 pos_alpha;
    vec2 trans_alpha;
    vec2 color_alpha;
    vec2 point_size;
    vec2 interval;
    vec2 positions;
    vec4 render_type; // mesh triangles, mesh points, points skeleton, graph (points, edges)
    vec4 extra_info;  // x: axis type, y, z, w: empty slots
};

struct ast_neu_properties {
    properties ast;
    properties neu;
};

layout (std430, binding=3) buffer space2d_data
{
    ast_neu_properties space2d;
};

const vec3 test_colors[12] = vec3[12](
	vec3(0.65, 0.8, 0.89),
	vec3(31.0/255, 120.0/255, 180.0/255),
	vec3(178.0/255, 223.0/255, 138.0/255),
	vec3(51.0/255, 160.0/255, 44.0/255),

	vec3(251.0/255, 154.0/255, 153.0/255),
	vec3(227.0/255, 26.0/255, 28.0/255),
	vec3(253.0/255, 191.0/255, 111.0/255),
	vec3(1, 0.49, 0),

	vec3(202.0/255, 178.0/255, 214.0/255),
	vec3(106.0/255, 61.0/255, 154.0/255),
	vec3(1, 1, 153.0/255),
	vec3(177.0/255.0, 89/255, 40.0/255)
	);

const vec3 selection_color = vec3(221.0 / 255, 52.0 / 255, 151.0 / 255);


void main() {

    node2D_alpha = 1;

    vec4 render_type = space2d.ast.render_type; // additional info


	if (!(render_type.x == 1 || render_type.y == 1) || V_State[0] == 0) {
        return;
    }


    gl_Position = gl_in[0].gl_Position;
	gl_PointSize = gl_in[0].gl_PointSize;// V_Radius_volume[0].x * 500;//10; //figure out actual way (and input zoom to be used)
    alpha = 0.7;
   // color_val = vec4(1, 0, 0.5, 1); //determine by using cluster ID
	if (V_State[0] > 1)
		color_val = vec4(selection_color, 1);
	else if (C_ID[0] > 0)
		color_val = vec4(test_colors[C_ID[0]%12], 1);
	else
		color_val = vec4(0.7, 0.7, 0.7, 1);


    EmitVertex();
    EndPrimitive();
}
