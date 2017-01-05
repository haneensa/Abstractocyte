#version 150

out vec4  outcol;
in vec4 posAttrG;

in vec3 normal_out;

// Cosine of the angle between the normal and the light direction,
// clamped above 0
//  - light is at the vertical of the triangle -> 1
//  - light is perpendicular to the triangle -> 0
//  - light is behind the triangle -> 0

//-------------------- AMBIENT LIGHT PROPERTIES --------------------
vec4 ambient = vec4(0.10, 0.10, 0.10, 1.0); // sets lighting level, same across many vertices
/*
Glfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
Glgloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
Glfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
Glfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
*/

//-------------------- DIFFUSE LIGHT PROPERTIES --------------------
vec3 diffuseLightDirection = vec3(-1.5f, 1.0f, -0.5f);
vec4 diffuseColor = vec4(1.0, 1.0, 1.0, 1.0);
float diffuseIntersity = 1.0;

// ------------------- TOON SHADER PROPERTIES ----------------------
// vec3 lineColor = vec4(0.0, 0.0,  0.0, 1.0); -> color to draw the lines in  (black)
// float lineThickness = 0.03

float cosTheta = clamp( dot( normal_out, diffuseLightDirection ), 0, 1 );
float intensity = dot(diffuseLightDirection, normal_out);

void main() {
    vec4 color = vec4(1.0, 0.0, 0.0, 1.0) * diffuseColor * diffuseIntersity + ambient;
    //vec4 color = posAttrG;
    outcol = vec4(color.r, color.g, color.b, 1.0);
    outcol.a = 1.0;

    if (intensity > 0.95)
        outcol = vec4(1.0, 1.0, 1.0, 1.0) * outcol;
    else if (intensity > 0.5)
        outcol = vec4(0.7, 0.7, 0.7, 1.0) * outcol;
    else if (intensity > 0.05)
        outcol = vec4(0.35, 0.35, 0.35, 1.0) * outcol;
    else
        outcol = vec4(0.1, 0.1, 0.1, 1.0) * outcol;

    //outcol = vec4(color.r, color.g, color.b, 1.0) * cosTheta  * diffuseColor * diffuseIntersity + ambient;

    // todo: interpolate between two colors
}
