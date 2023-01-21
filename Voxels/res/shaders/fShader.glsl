#version 450 core

in vec2 fTexCoord;
out vec4 Color;

uniform sampler2D uTexture;

void main() {
    Color = texture(uTexture, fTexCoord);
}