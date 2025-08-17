#version 150 core

in vec2 pos;
in vec3 color;

out vec3 Color;

void main()
{
    Color = color;
    gl_Position = vec4(pos, 0.0, 1.0);
};
