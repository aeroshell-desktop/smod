#version 300 es

precision highp float;

uniform sampler2D sampler;
uniform float opacity;
uniform float bordertop;
uniform float borderleft;

uniform vec2 targetrect;
uniform vec2 texturerect;
uniform mat4 colorMatrix;

in vec2 texcoord0;
out vec4 fragColor;


float map(float value, float originalMin, float originalMax, float newMin, float newMax) {
    return (value - originalMin) / (originalMax - originalMin) * (newMax - newMin) + newMin;
}

float processAxis(float coord, float textureBorder, float windowBorder) {
    if (coord < windowBorder) {
        return map(coord, 0.0f, windowBorder, 0.0f, textureBorder);
    }
    else if (coord < 1.0f - windowBorder) {
        return map(coord,  windowBorder, 1.0f - windowBorder, textureBorder, 1.0f - textureBorder);
    }
    else {
        return map(coord, 1.0f - windowBorder, 1.0f, 1.0f - textureBorder, 1.0f);
    }
}

void main()
{
    vec2 u_dimensions = vec2(borderleft / targetrect.x, bordertop / targetrect.y); // Borders normalized for the scaled texture
    vec2 u_borders = vec2(borderleft / texturerect.x, bordertop / texturerect.y); // Borders normalized for the original texture
    vec2 newUV = vec2(
        processAxis(texcoord0.x, u_borders.x, u_dimensions.x),
                      processAxis(texcoord0.y, u_borders.y, u_dimensions.y)
    );
    fragColor = texture(sampler, newUV) * opacity;
    fragColor *= colorMatrix;
}
