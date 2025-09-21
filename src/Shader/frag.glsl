#version 460 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = texture(uTexture, TexCoord);
    //FragColor = vec4(1,0,0,1);
}