#version 440
/// @file grid.frag
/// @brief Фрагментный шейдер слоя сетки: цвет по шкале (текстура 256 x 1) или цвет каркаса, затенение склонов.
///
/// Пара — `grid.vert` (тот же uniform-блок, поля описаны там). misc.w > 0,5 — цвет из шкалы `scaleTex`
/// по параметру vT; иначе — единый цвет `color`. extra.x > 0 — склоны рельефа темнеют по нормали из
/// производных экранных координат (ровное поле не темнеет). misc.z — непрозрачность слоя.
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders

layout(location = 0) in float vT;
layout(location = 1) in vec3 vPos;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    vec4 rect;
    vec4 map;
    vec4 misc;
    vec4 color;
    vec4 extra;
};

layout(binding = 1) uniform sampler2D scaleTex;   // шкала цвета 256 x 1 («Волна», «Свечение» и др.)

void main()
{
    vec4 c = misc.w > 0.5 ? texture(scaleTex, vec2((vT * 255.0 + 0.5) / 256.0, 0.5)) : color;
    if (extra.x > 0.0) {
        // Мягкое затенение склонов рельефа (нормаль по производным): ровное поле не темнеет.
        vec3 n = normalize(cross(dFdx(vPos), dFdy(vPos)));
        c.rgb *= 1.0 - extra.x * (1.0 - abs(n.z));
    }
    float a = c.a * misc.z;   // непрозрачность слоя
    // предумноженная альфа, как в color.frag
    fragColor = vec4(c.rgb * a, a);
}
