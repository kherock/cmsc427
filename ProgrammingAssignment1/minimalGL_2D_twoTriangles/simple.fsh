#version 330

uniform float uVertexScale;

in vec3 vColor;

layout(location = 0, index = 0) out vec4 fragColor;

void main( void ) {
  fragColor = vec4(vColor.x, vColor.y, vColor.z, 1);
}
