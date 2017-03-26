#version 430

#define astrocyte   6
#define spine       5
#define bouton      3
#define mito        1


#define points_size   60
layout (lines) in;
layout(points, max_vertices = points_size) out;

in int          V_ID[];
in float        V_alpha[];
in int          V_render[];
in int          V_connectivity[];
in float        V_node2D_alpha[];
flat out int    hasMito;
in vec3         V_fragTexCoord[];

out vec4        color_val;
out float       alpha;
out float       G_ID;
out float       node2D_alpha; /*1 -> 3D, 0 -> 2D*/
out vec3        G_fragTexCoord;

uniform int     hoveredID;
uniform int     x_axis;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;
    vec2 layout2;
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

void main() {
    int ID = V_ID[0] ;
    G_ID = float(ID);

    for (int i = 0; i < 2; ++i) {
        int filter_value = int(SSBO_data[V_ID[i]].info.w);
        int visibility = (filter_value >> 0) & 1;

        if ( visibility == 1 )
            return;
    }


    alpha = V_alpha[0];
    if (alpha < 0.01){
        return;
    }

    if ( V_render[0] == 0  ) {
        return;
    }

    int type = int(SSBO_data[ID].center.w); // 0: astrocyte, 1: neurite
    if (SSBO_data[ID].color.w == 1.0 && x_axis >= 99 && (type == spine || type == bouton))
        hasMito = 1;
    else
        hasMito = 0;

    node2D_alpha =  V_node2D_alpha[0];


    gl_PointSize =  gl_in[0].gl_PointSize;

    if (V_connectivity[0] == 1)
        color_val = vec4(0.8, 0.8, 0.8, 1);
    else
        color_val = SSBO_data[ID].color;

    if (hoveredID == ID) {
        color_val += vec4(0.2, 0.2, 0.2, 0);
    }

    vec4 start = gl_in[0].gl_Position;
    vec4 end = gl_in[1].gl_Position;

    vec4 startCoord  = vec4(V_fragTexCoord[0], 1);
    vec4 endCoord  = vec4(V_fragTexCoord[1], 1);

    for (int i = 0; i < points_size; i++ ) {
         float u = float(i) / float(points_size);
         float x = (end.x - start.x) * u + start.x;
         float y = (start.y - end.y) / (start.x - end.x) * (x - start.x) + start.y;
         gl_Position = vec4(x, y, 0, 1.0);

         float xCoord =  (endCoord.x - startCoord.x) * u + startCoord.x;
         float yCoord =  (startCoord.y - endCoord.y)  / (startCoord.x - endCoord.x) * (x - startCoord.x) + startCoord.y;
         G_fragTexCoord = vec3(xCoord, yCoord, (startCoord.z+endCoord.z)/2.0);

         EmitVertex();
         EndPrimitive();

    }
}
