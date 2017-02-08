#version 430

layout(location = 0) in vec3    mesh_vx;
layout(location = 1) in vec3    skeleton_vx;
layout(location = 2) in int     ID;

out vec4        Vskeleton_vx;
out int         V_ID;
out vec4        V_center;

struct SSBO_datum {
    vec4 color;
    vec4 center;
    vec4 info;
};


layout (std430, binding=2) buffer mesh_data
{
    SSBO_datum SSBO_data[];
};

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

void main(void)
{
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(mesh_vx.xyz , 1.0);
    Vskeleton_vx = pvmMatrix * vec4(skeleton_vx.xyz, 1.0);
    V_ID = ID;
    vec3 center3d = SSBO_data[ID].center.xyz;
    V_center =  pvmMatrix * vec4(center3d, 1.0);
}
