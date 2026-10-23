#version 440
/// @file color.frag
/// @brief Фрагментный шейдер цветной геометрии: цвет вершины с предумноженной альфой.
///
/// Пара — `color.vert`. Смешивание в конвейере настроено на предумноженную альфу (src = 1, dst = 1 - a),
/// поэтому RGB умножается на альфу здесь.
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders

layout(location = 0) in vec4 vCol;
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(vCol.rgb * vCol.a, vCol.a);   // предумноженная альфа
}
