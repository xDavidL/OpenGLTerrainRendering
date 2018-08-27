#version 330
uniform sampler2D waterSampler;
uniform sampler2D grassSampler;
uniform sampler2D rockSampler;
uniform sampler2D snowSampler;
uniform sampler2D pointSampler;
uniform int isPoint;
in float lightTerm;
in vec2 uv;
in vec4 texWeights;
in float waterDepthFactor;

void main()
{
    if (isPoint == 1) {
        vec4 col = texture(pointSampler, gl_PointCoord);
        if (col.a < 0.1) discard;
        gl_FragColor = col;
        return;
    }
    vec4 color = texture(waterSampler, uv) * texWeights[0] * waterDepthFactor
            + texture(grassSampler, uv) * texWeights[1]
            + texture(rockSampler, uv) * texWeights[2]
            + texture(snowSampler, uv) * texWeights[3];
    gl_FragColor = lightTerm * color;
}
