#version 300 es

precision highp float;

uniform vec3 uSolidColor;
uniform float uShininess;

in vec3 vPosition;
in vec3 vNormal;

out vec4 FragColor;

void main() {
    vec3 lightAmbt = vec3(0.1, 0.1, 0.1);
    vec3 lightDiff = vec3(1.0, 1.0, 1.0);
    vec3 lightPos = vec3(1.0, 1.8, 8.0);

    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos);
    vec3 R = reflect(-L, N);
    vec3 V = normalize(-vPosition);

    vec3 ka = clamp(uSolidColor, 0.0, 1.0);
    vec3 kd = clamp(uSolidColor, 0.0, 1.0);
    vec3 ks = vec3(0.0, 0.0, 0.0);
    float s = 1.0;
    if (dot(N, L) > 0.0 && dot(V, R) > 0.0) {
        ks = vec3(1.0, 1.0, 1.0);
        s = clamp(uShininess, 0.0, 1024.0);
    }

    vec3 ambient = lightAmbt * ka;
    vec3 diffuse = lightDiff * kd * max(dot(N, L), 0.0);
    vec3 specular = lightDiff * ks * pow(max(dot(V, R), 0.0), s);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

