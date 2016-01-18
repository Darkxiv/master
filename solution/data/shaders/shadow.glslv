#version 330

layout(location = 0) in vec3 position;

uniform mat4 modelToClipMatrix;

void main()
{
    gl_Position = modelToClipMatrix * vec4(position, 1.0);
}
