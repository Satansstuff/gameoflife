#version 430 core
// the position variable has attribute position 0
layout (std430, binding = 1) buffer ip
{ 
    vec3 positions[];
};
layout (std430, binding = 2) buffer iv
{ 
    mat4 matrices[];
};
layout (std430, binding = 3) buffer ic
{ 
    vec4 colors[];
};
out vec4 vertexColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * matrices[gl_InstanceID] * vec4(positions[gl_VertexID], 1.0);
    vertexColor = colors[gl_VertexID];
    //gl_Position = vec4(positions[gl_VertexID], 1.0); // set the output variable to a dark-red color
}
