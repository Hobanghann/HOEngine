#version 460 core

const int TARGET_ACCUMULATION_BINDING = 1;

layout(location = TARGET_ACCUMULATION_BINDING) out vec4 out_FragColor;

void main()
{
    out_FragColor = vec4(1.0, 0.3, 0.1, 1.0);
}