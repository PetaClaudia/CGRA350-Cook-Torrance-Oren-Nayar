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

float shininess = 25.0;
float albedo = 3.0;
float roughness = 0.1;
vec3 ambientCol = vec3(0.2, 0.1, 0.1);
vec3 diffuseCol = vec3(1.0, 0.0, 0.0);
vec3 specularCol    = vec3(1.0, 1.0, 1.0);
// framebuffer output
out vec4 fb_color;

float t(float s, vec3 lightDir, vec3 norm, vec3 eye) {
    if (s > 0) {
       return max(dot(norm, lightDir), dot(norm, eye));
    }
    else{
        return 1.0;
    }
}

void main() {
    
    vec3 eye = normalize(-f_in.position);
    vec3 norm = normalize(f_in.normal);
    vec3 light = vec3(0.25, 0.25, -1);
    vec3 lightDir = normalize(-light);
    float specularity = 0.0;
    
    float a = (1/PI) * (1.0 - (0.5 * ((roughness * roughness) / ((roughness * roughness) + 0.33))) +
                (0.17 * albedo * ((roughness * roughness) / ((roughness * roughness) + 0.13))));
    float b = (1/PI) * (0.45 * ((roughness * roughness) / ((roughness * roughness) + 0.09)));
    float s = dot(lightDir, eye) - (dot(norm, lightDir) * dot(norm, eye));
    float lambert = albedo * dot(norm, lightDir) * (a + (b * (s / t(s, lightDir, norm, eye))));
    
    if (lambert > 0) {
        vec3 h = normalize(lightDir+eye);
        float specularAngle = max(dot(h, norm), 0.0);
        specularity = pow(specularAngle, shininess);
    }
    vec3 fragColour = ambientCol + lambert * diffuseCol + specularity * specularCol;
    // output to the frambuffer
    fb_color = vec4(fragColour, 1);
}


