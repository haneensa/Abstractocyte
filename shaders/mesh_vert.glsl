#version 430

layout(location = 0) in vec4    mesh_vx;
layout(location = 1) in vec4    skeleton_vx;

out vec4        Vskeleton_vx;
out int         V_ID;
out int         V_bleeding;


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
    int ID = int(mesh_vx.w);
    V_ID = ID;
    int bleeding =  int(skeleton_vx.w);
    V_bleeding = bleeding;
}
