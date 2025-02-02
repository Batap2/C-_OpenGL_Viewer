#version 330 core

layout (location = 0) in vec3 ipos;
layout (location = 1) in vec2 itexCoords;

out vec2 texCoords;

void main() {
    gl_Position = vec4(ipos.x, ipos.y, 0.0, 1.0);
    texCoords = itexCoords;
}