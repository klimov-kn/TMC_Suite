#version 440
/// @file sprite.frag
/// @brief Фрагментный шейдер картинок поверх сцены: цвет из текстуры (картинка уже с предумноженной альфой).
///
/// Пара — `sprite.vert`. Текстура `tex` (привязка 1) — надпись или шкала, нарисованная QPainter.
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders

layout(location = 0) in vec2 vUv;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;   // картинка надписи или шкалы

void main()
{
    fragColor = texture(tex, vUv);   // предумноженная альфа
}
