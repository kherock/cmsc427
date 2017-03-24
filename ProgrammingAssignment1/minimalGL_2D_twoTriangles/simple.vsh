#version 330

uniform float uVertexScale;

in vec2 aVertex;
in vec3 aColor;

out vec3 vColor;

void main( void )
{
  gl_Position = vec4(aVertex.x * uVertexScale, aVertex.y * uVertexScale, 0, 1);
  vColor = aColor;
}
