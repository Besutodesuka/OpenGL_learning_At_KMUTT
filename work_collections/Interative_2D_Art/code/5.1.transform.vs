#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 transform;

out vec2 TexCoord;
uniform float pointSize;

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    // Generate texture coordinates from position assuming unit circle in NDC
    TexCoord = aPos.xy * 0.5 + 0.5;
    gl_PointSize = pointSize;
}