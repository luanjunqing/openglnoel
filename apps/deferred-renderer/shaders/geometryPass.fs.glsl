#version 330

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

void main() {

    vec3 Ka = uKa * texture(uKaSampler, vTexCoords).rgb;
    vec3 Kd = uKd * texture(uKdSampler, vTexCoords).rgb;
    vec3 Ks = uKs * texture(uKsSampler, vTexCoords).rgb;
    float shininess = uShininess * texture(uShininessSampler, vTexCoords).r;

    fPosition = vViewSpacePosition;
    fNormal = normalize(vViewSpaceNormal);
    fAmbient = Ka;
    fDiffuse = Kd;
    fGlossyShininess = vec4(Ks, shininess);
}
