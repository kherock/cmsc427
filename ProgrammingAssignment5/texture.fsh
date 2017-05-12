#version 330

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 Kd;            // Diffuse reflectivity
uniform vec3 Ka;            // Ambient reflectivity
uniform vec3 Ks;            // Specular reflectivity
uniform float Shininess;    // Specular shininess factor

uniform bool useTexture; // Flag set to true if texture should be used.
uniform sampler2D Tex1;  // Texture sampler

uniform vec4 LightDirection; // Lighting parameters.
uniform vec3 LightIntensity;

layout( location = 0 ) out vec4 FragColor; // Output fragment color.

void main() {
    vec3 e = vec3(0,0,0); // Camera position.
    vec3 l = vec3(LightDirection); // Set light direction
    vec3 p = Position; 
    vec3 Li = LightIntensity;

    vec3 n = normalize(Normal); // Unit normal vector.
    vec3 s = normalize(l); // Light direction
    vec3 v = normalize(e - p);    // Viewing direction.

    vec3 r = -s + 2 * dot(s, n) * n; // Mirror reflection direction

    vec3 Kd_tex = Kd; // Diffuse.
    vec3 Ka_tex = Ka; // Ambient.
    if(useTexture) {
        Kd_tex = vec3(texture( Tex1, TexCoord )); // Diffuse/ambient from texture
    }
    
    vec3 L = Li * (Ka_tex +                             // ambient
                   Kd_tex * max(0, dot(n, s)) +         // diffuse
                   Ks * pow( max(0, dot(r,v)), Shininess) ); // specular

    FragColor = vec4(L, 1.0); // no transparancy
}



