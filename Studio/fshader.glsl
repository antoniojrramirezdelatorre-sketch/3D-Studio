#version 330

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoord; //incoming from vertex shader

out vec4 fColor;

//uniforms from C++
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform vec3 camPos;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform float matShininess;

//texture uniforms
uniform sampler2D texUnit;
uniform bool useTexture;

void main()
{
    //ambient
    vec3 ambient = ambientColor * matAmbient;

    //diffuse
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * matDiffuse;

    //specular
    vec3 specular = vec3(0.0);
    if (diff > 0.0) {
        vec3 viewDir = normalize(camPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
        specular = spec * lightColor * matSpecular;
    }

    vec3 result = ambient + diffuse + specular;

    //apply texture if enabled
    if (useTexture) {
        vec4 texColor = texture(texUnit, fragTexCoord);
        //modulate lighting result with texture color
        result = result * texColor.rgb;
    }

    fColor = vec4(result, 1.0);
}