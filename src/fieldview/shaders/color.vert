#version 440
/// @file color.vert
/// @brief Вершинный шейдер цветной геометрии FieldView (топология блоков, оси, размерные линии, курсор).
///
/// Часть FieldView на Qt RHI; компилируется в .qsb (`qt_add_shaders`, CMakeLists.txt). Пара — `color.frag`.
/// Вход: позиция вершины в координатах сцены (`pos`) и её цвет RGBA (`col`); выход — цвет во фрагментный
/// шейдер. Единственный параметр — матрица вида-проекции `mvp` (uniform-блок 0).
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders
// Цветная геометрия: топология, оси, размерные линии, курсор. Цвет — у вершины.

layout(location = 0) in vec3 pos;   // точка в координатах сцены
layout(location = 1) in vec4 col;   // цвет вершины RGBA (альфа не умножена)

layout(location = 0) out vec4 vCol;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;   // вид * проекция камеры сцены
};

void main()
{
    vCol = col;
    gl_Position = mvp * vec4(pos, 1.0);
}
