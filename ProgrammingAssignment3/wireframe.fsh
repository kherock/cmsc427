#version 330

in vec3 vBC;

layout( location = 0 ) out vec4 FragColor;

float edgeFactor(){
    vec3 d = fwidth(vBC);
    vec3 a3 = smoothstep(vec3(0.0), d*1.5, vBC);
    return min(min(a3.x, a3.y), a3.z);
}

void main(){
    // coloring by edge
    FragColor.rgb = mix(vec3(0.0), vec3(0.8), edgeFactor());
    FragColor.a = 1.0;

    // alpha by edge
    if(gl_FrontFacing){
        FragColor.a = (1.0-edgeFactor())*0.95;
    }
    else{
       FragColor.a = (1.0-edgeFactor())*0.7;
    }
}
