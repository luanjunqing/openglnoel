#version 430

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f) uniform readonly restrict image2D uInputImage;
uniform writeonly restrict image2D uOutputImage;
uniform float uGammaExponent;
const int uBlurMatrixHalfSide = 4;

#define MODE 2

void main() {
    ivec2 inputImageSize = imageSize(uInputImage);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    if(pixelCoords.x >= inputImageSize.x || pixelCoords.y >= inputImageSize.y)
        return;

    vec4 pixel = vec4(0);

#if MODE==0
    pixel = imageLoad(uInputImage, pixelCoords);
#elif MODE==1
    vec4 accum = vec4(0);
    int s = uBlurMatrixHalfSide;

    for(int dy=-s ; dy <= s ; ++dy) {
        for(int dx=-s ; dx <= s ; ++dx) {
            ivec2 coords = clamp(pixelCoords + ivec2(dx, dy), ivec2(0), inputImageSize-ivec2(1));
            accum += imageLoad(uInputImage, coords);
        }
    }
    pixel = accum / ((s+s+1)*(s+s+1));
#elif MODE==2

    vec2 coords = vec2(pixelCoords);
    const uint numSamples = 32;
    vec2 center = vec2(inputImageSize)/2.0f;
    vec2 velocityFromCenter = (center - coords) / float(numSamples);
    float upperLength = 20.0f / float(numSamples);
    vec2 velocity = length(velocityFromCenter) < upperLength ? velocityFromCenter : upperLength * normalize(velocityFromCenter);
    for(uint i = 0; i < numSamples; ++i, coords += velocity) {
        pixel += imageLoad(uInputImage, ivec2(round(coords)));
    }
    pixel /= float(numSamples);
#else
#error Invalid MODE value
#endif

    pixel.rgb = pow(pixel.rgb, vec3(uGammaExponent));
    imageStore(uOutputImage, pixelCoords, pixel);
}
