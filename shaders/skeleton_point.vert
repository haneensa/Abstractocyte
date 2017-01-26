#version 430

layout(location = 0) in vec4 posAttr;
layout(location = 1) in int ID;
out vec4 posAttrV;

// World transformation
uniform mat4 mMatrix;
// View Transformation
uniform mat4 vMatrix;
// Projection transformation
uniform mat4 pMatrix;

out vec4        V_color;
out int         V_ID;

// get ID, to color this thing

struct SSBO_datum {
    vec4 color;
    vec4 center;
};

layout (std430, binding=2) buffer shader_data
{
    SSBO_datum SSBO_data[];
};

void main(void)
{
    posAttrV = posAttr;
    mat4 pvmMatrix = pMatrix * vMatrix * mMatrix;
    gl_Position =  pvmMatrix * vec4(posAttr.xyz, 1.0);
    V_color = SSBO_data[ID].color;
    V_ID = ID;
}
