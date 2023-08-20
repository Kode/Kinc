#version 450

uniform sampler2D texy;
in vec2 texCoord;
out vec4 FragColor;

void main() {
	FragColor = texture(texy, texCoord);
}
