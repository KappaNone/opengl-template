#version 150 core

uniform float time;

in vec3 Color;

out vec4 out_color;

const int blink_speed = 4;

void main()
{
    out_color = vec4(
            (cos(time * blink_speed) + 1) / 2 * Color.r, 
            (cos(time * blink_speed) + 1) / 2 * Color.g,
            (cos(time * blink_speed) + 1) / 2 * Color.b,
            1.0f);
};
