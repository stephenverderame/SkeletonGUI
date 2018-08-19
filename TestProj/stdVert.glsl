#version 330 core
layout (location = 0) in vec2 position;

out vec2 pos;
out vec2 texCoords;

uniform mat4 model;
uniform mat4 projection;

void main(){
	texCoords = position;
	gl_Position = projection * model * vec4(position, 1.0, 1.0);
	pos = vec2(gl_Position.x, gl_Position.y);
} 
