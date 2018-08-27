#version 400
layout (triangles) in;
layout (triangle_strip, max_vertices = 80) out;
uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 normMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;
out float lightTerm;
out vec2 uv;
out vec4 texWeights;
out float waterDepthFactor;


float interp(float min, float max, float val)
{
    return (val - min) / (max - min);
}

void calcWeights(float y) {
    if (y < 5) {
        texWeights = vec4(0, 1, 0, 0);
    } else if (y < 6) {
        float t = interp(5, 6, y);
        texWeights = vec4(0, 1 - t, t, 0);
    } else {
        texWeights = vec4(0, 0, 1, 0);
    }

    if (y <= waterLevel) {
        texWeights = vec4(1, 0, 0, 0);
    } else if (y >= snowLevel) {
        texWeights = vec4(0, 0, 0, 1);
    } else if (y >= snowLevel - 1) {
        float t = interp(snowLevel - 1, snowLevel, y);
        texWeights[3] = t;
        if (texWeights[1] > 0) {
            texWeights[1] = (1 - t) * texWeights[1];
        }
        if (texWeights[2] > 0) {
            texWeights[2] = (1 - t) * texWeights[2];
        }
    }
}

void main() {
    vec4 v1 = (gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec4 v2 = (gl_in[2].gl_Position - gl_in[0].gl_Position);
    vec4 norm = (normMatrix * normalize(vec4(cross(v1.xyz, v2.xyz), 0)));
    for (int i = 0; i < gl_in.length(); i++)
    {
        vec4 pos = gl_in[i].gl_Position;
        float y = pos.y;
        waterDepthFactor = min(1, pow(waterLevel - y, -0.8));
        if (waterLevel > y) {
            y = waterLevel;
            vec4 posEye = mvMatrix * gl_in[i].gl_Position;
            vec4 lightVec = normalize(lightPos - posEye);
            norm = normMatrix * vec4(0, 1, 0, 0);
            lightTerm = max(0, dot(lightVec, norm)) + 0.2;
        } else {
            vec4 posEye = mvMatrix * gl_in[i].gl_Position;
            vec4 lightVec = normalize(lightPos - posEye);
            lightTerm = max(0, dot(lightVec, norm)) + 0.2;
        }
        pos.y = y;
        uv = vec2(pos.x, pos.z);
        calcWeights(y);
        gl_Position = mvpMatrix * pos;
        EmitVertex();
    }
    EndPrimitive();
}
