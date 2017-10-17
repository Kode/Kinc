#version 450

in vec3 pos;
in vec2 tex;
out vec2 texCoord;

void main() {
	gl_Position = vec4(pos.x, pos.y, 0.5, 1.0);
	texCoord = tex;
}
