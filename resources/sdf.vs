#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// NOTE: Add your custom variables here

void main()
{
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}