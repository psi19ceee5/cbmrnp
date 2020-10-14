#version 400

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;


// get position from vertex array object
layout(location = 0) in vec3 vpos;
layout(location = 1) in vec3 vertex_normals;
layout(location = 2) in vec2 texCoords;

// send color to fragment shader
//out vec3 vcolor;

smooth out vec2 st;
smooth out vec3 normal;
out vec3 pos;

void main(void)
{
    // calculate position in model view projection space
    gl_Position = projection_matrix * modelview_matrix * vec4(vpos, 1);

    // Texture coordinates
    st = texCoords;

    // normals
    normal = vertex_normals;

    pos=vpos;
}
