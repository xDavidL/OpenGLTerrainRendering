#version 330

layout (location = 0) in vec4 position;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform int isPoint;
uniform sampler2D heightMap;
uniform float waterLevel;
out vec4 gl_Position;


void main()
{
    if (isPoint == 1) {
        vec4 pos = position;
        vec4 posEye = mvMatrix * position;
        gl_PointSize = 6000.0 / pow(pow(posEye.x, 2) + pow(posEye.z, 2), 0.5);
        float u = (pos.x + 45) / 90.0;
        float v = pos.z / -100.0;
        float height = texture(heightMap, vec2(u, v)).x;
        pos.y = max(waterLevel, height * 10.0) + 1.5;
        gl_Position = mvpMatrix * pos;
    } else {
        gl_Position = position;
    }
}
