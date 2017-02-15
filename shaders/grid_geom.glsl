#version 430

//layout (lines) in;
//layout (line_strip, max_vertices = 250) out;
 layout (points) in;
 layout (points, max_vertices = 1) out;

// ID for each line from 0 until n where n is number of lines there
// access buffer, and get control points for the curves

vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
{
    float B0 = (1.-u)*(1.-u)*(1.-u);
    float B1 = 3.*u*(1.-u)*(1.-u);
    float B2 = 3.*u*u*(1.-u);
    float B3 = u*u*u;

    vec3 p = B0*p0 + B1*p1 + B2+p2 + B3*p3;
    return p;
}


void main() {
  /*  vec4 A = gl_in[0].gl_Position;
    vec4 B = gl_in[1].gl_Position;

    // I need two control points
    int j = 0;
    for (int i = 0; i < 200; i++ ) {
         float u = float(i) / float(200);
         vec3 new_point = bezier(u, A.xyz, vec3(0.0, 0.1, 0.0), vec3(0.0, -0.1, 0.0),  B.xyz);
         gl_PointSize = 10;
         gl_Position = vec4(new_point, 1.0);
         EmitVertex();
         j++;
         if (j  == 2) {
           j = 0;
           EndPrimitive();
        }
    }
*/
    gl_Position = gl_in[0].gl_Position;
    gl_PointSize = 10;
    EmitVertex();

  //  gl_Position = gl_in[1].gl_Position;
   // gl_PointSize = 10;
   // EmitVertex();
    EndPrimitive();
}
