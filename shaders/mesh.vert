#version 330 core

in vec3 mesh_vx;
in vec3 skeleton_vx;
in int ID;

out vec4 Vskeleton_vx;
out int V_ID;


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
}
