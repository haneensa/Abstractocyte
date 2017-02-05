#version 430

in int V_ID[];
out vec4 color;

layout (std430, binding=1) buffer state_color
{
    vec4 SSBO_data[];
};

uniform int is_selection_shader;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// todo: ssbo, based on the ID we get interval and special color for debuging
void main() {
    for(int i = 0; i < 3; i++) {
        int ID  = V_ID[i];
        if (is_selection_shader == 1)
             color = vec4(1.0, 0.0, 0.0, 0.0);
        else
            color = SSBO_data[ID];

        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
