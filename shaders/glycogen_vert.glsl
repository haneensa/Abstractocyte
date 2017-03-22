#version 430

// in: per vertex data
//layout (location = 0) in int ID;
//layout (location = 1) in vec4 center;

layout(location = 0) in vec4    center;
layout(location = 1) in vec4    info;

out int         V_ID;
out int			C_ID;
out int			V_State;
out vec2		V_Radius_volume;

uniform mat4 mMatrix;
uniform mat4 m_noRartionMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;


void main(void)
{
	//if (int(info.w) == 0)
	//	discard;

	V_ID = int(center.w);
	C_ID = int(info.z);
	V_State = int(info.w);
	V_Radius_volume.xy = info.xy;

	mat4 mvpMatrix = pMatrix * vMatrix * mMatrix;

    gl_Position =  mvpMatrix * vec4(center.xyz, 1.0);

	//replace 449 with verticle size of window?
	gl_PointSize = 449 * pMatrix[1][1] * info.x * m_noRartionMatrix[0][0] / gl_Position.w;
}
