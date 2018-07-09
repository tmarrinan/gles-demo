#version 300 es

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

in vec3 aVertexPosition;
in vec3 aVertexNormal;

out vec3 vPosition;
out vec3 vNormal;

void main() {
    vPosition = vec3(uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0));
    vNormal = normalize(mat3(transpose(inverse(uViewMatrix * uModelMatrix))) * aVertexNormal);

    gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
}

