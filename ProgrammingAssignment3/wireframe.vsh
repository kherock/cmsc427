#version 330

// Shader adapted from: Florian Boesch
// http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 Barycentric;

out vec3 vBC;

uniform mat4 ModelViewMatrix;
uniform mat4 MVP;

void main()
{
    vBC = Barycentric;
    gl_Position = MVP * vec4(VertexPosition,1.0);
}
