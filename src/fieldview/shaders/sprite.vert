#version 440
/// @file sprite.vert
/// @brief Вершинный шейдер картинок поверх сцены (надписи, шкала цвета): координаты — пиксели окна.
///
/// Пара — `sprite.frag`. Прямоугольник картинки задаётся в пикселях окна, `proj` — ортографическая
/// проекция окна (uniform-блок 0); `uv` — координаты в текстуре картинки.
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders
// Картинка поверх сцены (надписи, шкала): координаты — пиксели окна.

layout(location = 0) in vec2 pos;   // угол картинки, пиксели окна
layout(location = 1) in vec2 uv;    // координаты в текстуре 0..1

layout(location = 0) out vec2 vUv;

layout(std140, binding = 0) uniform buf {
    mat4 proj;   // пиксели окна -> координаты отсечения
};

void main()
{
    vUv = uv;
    gl_Position = proj * vec4(pos, 0.0, 1.0);
}
