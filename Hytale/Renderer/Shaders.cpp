/*
 * Copyright (c) FishPlusPlus.
 */
#include "Shaders.h"
#include <string>

static const std::string POSCOLOR_VERT = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

out vec4 vertexColor;

uniform mat4 viewMat;

void main() {
    gl_Position = viewMat * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

static const std::string POSCOLOR_FRAG = R"(#version 330 core

out vec4 FragColor;
in vec4 vertexColor;

void main() {
    FragColor = vertexColor;
}
)";

static const std::string TEXT_VERT = R"(#version 330 core
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

static const std::string TEXT_FRAG = R"(#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0,
                        texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

static const std::string ALPHA_CHECKER_FRAG = R"(#version 330 core

in vec4 vertexColor;

out vec4 FragColor;

uniform float alphaCheckerX;
uniform float alphaCheckerY;
uniform float alphaCheckerSize;

void main()
{
    vec2 coord = vec2(alphaCheckerX, alphaCheckerY) - gl_FragCoord.xy;
    int cellX = int(floor(coord.x / alphaCheckerSize));
    int cellY = int(floor(coord.y / alphaCheckerSize));

    if (mod((cellX + cellY), 2) == 0)
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    else
        FragColor = vec4(0.8, 0.8, 0.8, 1.0);
}
)";

static const std::string HUE_FRAG = R"(#version 330 core

in vec4 vertexColor;

out vec4 FragColor;

uniform float yPos;
uniform float height;

vec3 hsv_to_rgb(vec3 hsv) {
    float h = hsv.x, s = hsv.y, v = hsv.z;
    float c = v * s;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;
    vec3 rgb;

    if (h < 60.0)      rgb = vec3(c, x, 0);
    else if (h < 120.0) rgb = vec3(x, c, 0);
    else if (h < 180.0) rgb = vec3(0, c, x);
    else if (h < 240.0) rgb = vec3(0, x, c);
    else if (h < 300.0) rgb = vec3(x, 0, c);
    else rgb = vec3(c, 0, x);

    return rgb + vec3(m);
}

void main()
{
    float uv = (gl_FragCoord.y - yPos) / height;
    uv = clamp(uv, 0.0, 1.0);


    float hue = 360 - uv * 360.0;
    vec3 rgb = hsv_to_rgb(vec3(hue, 1.0, 1.0));
    FragColor = vec4(rgb, 1.0);
}
)";

static const std::string POSTPROCESS_VERT = R"(#version 330 core
layout (location = 0) in vec4 vertex;

uniform vec2 u_Size;

out vec2 v_TexCoord;
out vec2 v_OneTexel;

void main()
{
    gl_Position = vec4(vertex.xy, 0.0, 1.0);
    v_TexCoord = vertex.zw;
    v_OneTexel = 1.0 / u_Size;
}
)";

/*
static const std::string POSTPROCESS_FRAG = R"(#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D screenTexture;
uniform vec2 u_Size;
uniform float u_Time;

vec2 directionalWaveNormal(vec2 p, float amp, vec2 dir, float freq, float speed, float time, float k) {
    float a = dot(p, dir) * freq + time * speed;
    float b = 0.5 * k * freq * amp * pow((sin(a) + 1.0) * 0.5, k - 1.0) * cos(a);
    return vec2(dir.x * b, dir.y * b);
}

vec3 summedWaveNormal(vec2 p) {

    vec2 sum = vec2(0.0);
    sum += directionalWaveNormal(p, 0.5, normalize(vec2(1, 1)), 5.0, 1.5, u_Time, 1.0);
    sum += directionalWaveNormal(p, 0.25, normalize(vec2(1.4, 1.0)), 11.0, 2.4, u_Time, 1.5);
    sum += directionalWaveNormal(p, 0.125, normalize(vec2(-0.8, -1.0)), 10.0, 2.0, u_Time, 2.0);
    sum += directionalWaveNormal(p, 0.0625, normalize(vec2(1.3, 1.0)), 15.0, 4.0, u_Time, 2.2);
    sum += directionalWaveNormal(p, 0.03125, normalize(vec2(-1.7, -1.0)), 5.0, 1.8, u_Time, 3.0);
    return normalize(vec3(-sum.x, -sum.y, 1.0));
}

void main() {

    vec4 mask = texture(screenTexture, TexCoords);
    if (mask == 0)
        discard;

    vec2 p = (2.0 * gl_FragCoord.xy - u_Size) / min(u_Size.x, u_Size.y);

    vec3 normal = summedWaveNormal(p);

    vec3 c = mix(vec3(0.31, 0.15, 0.1), vec3(0.2, 0.25, 0.4), dot(normal, normalize(vec3(0.1, 0.2, 0.5))) * 0.5 + 0.5);
    c = mix(c, vec3(0.7, 0.9, 1.0), pow(dot(normal, normalize(vec3(-0.4, 0.1, 1.0))) * 0.5 + 0.5, 2.0));
    c = mix(c, vec3(0.9, 0.98, 1.0), pow(dot(normal, normalize(vec3(-0.1, -0.3, 0.5))) * 0.5 + 0.5, 10.0));
    
    FragColor = vec4(c, 0.5);
}
)";
*/

static const std::string POSTPROCESS_FRAG = R"(#version 330 core
in vec2 v_TexCoord;
in vec2 v_OneTexel;

out vec4 FragColor;
uniform sampler2D u_Texture;
uniform vec2 u_Size;
uniform float u_Time;
uniform vec3 u_OutlineColor;
uniform bool u_Glow;
uniform int u_GlowSize;

vec2 directionalWaveNormal(vec2 p, float amp, vec2 dir, float freq, float speed, float time, float k) {
    float a = dot(p, dir) * freq + time * speed;
    float b = 0.5 * k * freq * amp * pow((sin(a) + 1.0) * 0.5, k - 1.0) * cos(a);
    return vec2(dir.x * b, dir.y * b);
}

vec3 summedWaveNormal(vec2 p) {

    vec2 sum = vec2(0.0);
    sum += directionalWaveNormal(p, 0.5, normalize(vec2(1, 1)), 5.0, 1.5, u_Time, 1.0);
    sum += directionalWaveNormal(p, 0.25, normalize(vec2(1.4, 1.0)), 11.0, 2.4, u_Time, 1.5);
    sum += directionalWaveNormal(p, 0.125, normalize(vec2(-0.8, -1.0)), 10.0, 2.0, u_Time, 2.0);
    sum += directionalWaveNormal(p, 0.0625, normalize(vec2(1.3, 1.0)), 15.0, 4.0, u_Time, 2.2);
    sum += directionalWaveNormal(p, 0.03125, normalize(vec2(-1.7, -1.0)), 5.0, 1.8, u_Time, 3.0);
    return normalize(vec3(-sum.x, -sum.y, 1.0));
}

void main() {

    vec4 mask = texture(u_Texture, v_TexCoord);
    float dist = 5 * 5 * 4.0;

    bool isMask = length(mask.rgb) > 0.1;
    if (isMask)
        discard;

    if (u_Glow) {
        for (int x = -u_GlowSize; x <= u_GlowSize; x++) {
            for (int y = -u_GlowSize; y <= u_GlowSize; y++) {
                vec4 neighbor = texture(u_Texture, v_TexCoord + v_OneTexel * vec2(x, y));
                if (mask != neighbor) {
                    float ndist = x * x + y * y - 1.0;
                    dist = min(dist, ndist);
                }
            }
        }
    }

    

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec4 neighbor = texture(u_Texture, v_TexCoord + v_OneTexel * vec2(x, y));

            if (mask != neighbor) {
                FragColor = vec4(u_OutlineColor.x, u_OutlineColor.y, u_OutlineColor.z, 1.0f);
                return;
            }
        }
    }

    float minDist = float(5 * 5);
    float glow = 1.0 - clamp(dist / minDist, 0.0, 1.0);
    if (!u_Glow)
        glow = 0.0;

    FragColor = vec4(u_OutlineColor.x, u_OutlineColor.y, u_OutlineColor.z, glow * 0.5);
})";



void Shaders::initShaders() {
    posColor = std::make_unique<Shader>(POSCOLOR_VERT, POSCOLOR_FRAG);
    text = std::make_unique<Shader>(TEXT_VERT, TEXT_FRAG);
    alphaChecker = std::make_unique<Shader>(POSCOLOR_VERT, ALPHA_CHECKER_FRAG);
    hue = std::make_unique<Shader>(POSCOLOR_VERT, HUE_FRAG);
	postProcess = std::make_unique<Shader>(POSTPROCESS_VERT, POSTPROCESS_FRAG);
}