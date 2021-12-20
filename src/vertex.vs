#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

uniform mat4 transform_matrix;

out vec2 Texcoord;

void main()
{
    Texcoord = texcoord;

    gl_Position = transform_matrix * vec4(position, 1.0);
}
