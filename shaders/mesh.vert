#version 430

layout(location = 0) in vec3 mesh_vx;
layout(location = 1) in vec3 skeleton_vx;
layout(location = 2) in int ID;

out vec4 Vskeleton_vx;
out int V_ID;
out vec4 V_color;


layout (std430, binding=2) buffer shader_data
{
    vec3 colors[];
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

    if (ID <= 10)
        V_color = vec4(1.0, 1.0, 0.0, 1.0);
    else
        V_color = vec4(colors[ID].rgb, 1.0);

}
