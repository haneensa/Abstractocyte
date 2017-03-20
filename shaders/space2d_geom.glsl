#version 430

in int V_ID[];
out vec4 color_val;
out float alpha;

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
        float b = ID/255.0;
        if (is_selection_shader == 1)
             color_val = vec4(b,  0.0,  0.0, 0.0);
        else
            color_val = vec4(1, 1, 1, 1);//SSBO_data[ID-1];

        gl_Position = gl_in[i].gl_Position;
        alpha = 1.0;
        EmitVertex();
    }
    EndPrimitive();
}
