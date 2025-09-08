#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D orbTexture;
uniform bool useTexture;
uniform vec4 baseColor;
// Star coloring controls
uniform float omega;       // current angular velocity
uniform float omegaMax;    // value at which color reaches full intensity
uniform vec4 starMaxColor; // bright purple target color
uniform bool drawFirefly;  // toggle for point-sprite firefly rendering

void main()
{
    if (useTexture)
        FragColor = texture(orbTexture, TexCoord);
    else {
        if (drawFirefly) {
            // Radial glow using gl_PointCoord (only valid for points)
            vec2 p = gl_PointCoord * 2.0 - 1.0; // [-1,1]
            float r = length(p);
            float alpha = exp(-6.0 * r * r);
            vec3 glow = vec3(0.6, 1.0, 0.3); // greenish
            FragColor = vec4(glow, alpha);
        } else {
            // Map omega → [0,1] and ease it for stars
            float t = clamp(omega / max(omegaMax, 1e-5), 0.0, 1.0);
            t = smoothstep(0.0, 1.0, t);
            // Blend from black to bright purple
            vec4 starColor = mix(vec4(0.0, 0.0, 0.0, 1.0), starMaxColor, t);
            // Optionally multiply by baseColor (keeps API compatibility)
            FragColor = starColor * baseColor;
        }
    }
}