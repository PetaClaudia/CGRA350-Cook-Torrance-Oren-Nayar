#version 330 core
#define PI 3.14159
// uniform data
uniform mat4 uProjectionMatrix;
uniform mat4 uModelViewMatrix;
uniform vec3 uColor;

// viewspace data (this must match the output of the fragment shader)
in VertexData {
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
} f_in;

float roughness = 0.1;
vec3 ambientCol = vec3(0.2, 0.1, 0.1);
vec3 diffuseCol = vec3(1.0, 0.0, 0.0);
vec3 specularCol    = vec3(1.0, 1.0, 1.0);
// framebuffer output
out vec4 fb_color;

float microfacet(vec3 v, vec3 norm, vec3 eye, vec3 halfAngle) {
    return (2 * dot(halfAngle, norm) * dot(v, norm)) / dot(eye, norm);
}


void main() {
    
    vec3 eye = normalize(-f_in.position);
    vec3 norm = normalize(f_in.normal);
    vec3 light = vec3(0.25, 0.25, -1);
    vec3 lightDir = normalize(-light);
    vec3 halfAngle = normalize(lightDir + eye);
    float specularity = 0.0;
    float lambert = max(dot(lightDir, norm), 0.0);
    
    //beckmann
    float alpha = acos(dot(norm, halfAngle));
    float beckmann = (exp((-pow(tan(alpha), 2) / pow(roughness, 2)))) /
            (PI * pow(roughness, 2) * pow(cos(alpha), 4));
 
    //attenuation
    float attenuation = min(1, min(microfacet(eye, norm, eye, halfAngle), microfacet(lightDir, norm, eye, halfAngle)));
    
    //schlick
    float n1 = 1.0;
    float n2 = 1.5;
    float f0 = pow((n1 - n2) / (n1 + n2), 2);
    float schlick = f0 + (1 - f0) * (1 - dot(eye, norm));
    
    if (lambert > 0) {
        specularity = (schlick * beckmann * attenuation)/ 4 * dot(eye, norm) * dot(lightDir, norm);
    }
    vec3 fragColour = ambientCol + lambert * diffuseCol + specularity * specularCol;
    // output to the frambuffer
    fb_color = vec4(fragColour, 1);
}

