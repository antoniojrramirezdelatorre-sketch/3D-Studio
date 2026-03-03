#version 330

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main() {

    fragPos = vec3(M * vec4(vPosition, 1.0));
    fragNormal = mat3(M) * vNormal;
    fragTexCoord = vTexCoord;

    gl_Position = P * V * M * vec4(vPosition, 1.0);
}