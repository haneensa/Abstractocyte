#version 430

// in: per vertex data
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec2 layout1;
layout (location = 2) in vec2 layout2;
layout (location = 3) in vec2 layout3;

out int V_ID;

out vec4 v_vertex;
out vec4 v_layout1;
out vec4 v_layout2;
out vec4 v_layout3;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

uniform int is2D;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
    vec2 layout1;   // neurite nodes
    vec2 layout2;   // neurites nodes + astrocyte skeleton
};

layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

void main(void)
{
    int ID = int(vertex.w);

    // two positions to interpolate between:
    // 3D with rotation
    // projected 2D without rotation
    mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;

    // between (60, 60) and (80, 80)
    // vec4 new_position = mix(vertex , layout1, position_intp);

    // between  (80, 80) and (100, 80)
    // vec4 new_position = mix(layout1 , layout2, position_intp);
    // vec4 new_position = mix(layout1 , center, position_intp);

    // between (80, 80) and (80, 100)
    // vec4 new_position = mix(layout1 , layout3, position_intp);

    // between (80, 100) and (100, 100)
    // vec4 new_position = mix(layout3 , center, position_intp);

    // between (100, 80) and (100, 100)
    // vec4 new_position = mix(layout2 , center, position_intp);


    // todo: interpolate between different layouts based on state

    v_vertex =  mvpMatrix * vec4(vertex.xyz, 1); // original position
    v_layout1 =  mvpMatrix * vec4(layout1, 0, 1); // original position
    v_layout2 =  mvpMatrix * vec4(layout2, 0, 1); // original position
    v_layout3 =  mvpMatrix * vec4(layout3, 0, 1); // original position

    gl_Position = v_vertex;
    V_ID = ID;
}
