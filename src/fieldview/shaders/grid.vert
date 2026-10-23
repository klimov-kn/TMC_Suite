#version 440
/// @file grid.vert
/// @brief Вершинный шейдер слоя сетки FieldView: поле или eps, рельеф и цвет по шкале.
///
/// Пара — `grid.frag`. Узлы сетки (u, v от 0 до 1) лежат в видеопамяти один раз, значение узла `value`
/// приходит каждый кадр. Высота h = value*k + off (как ZtoGLWinCoord Windows-FieldView), параметр цвета
/// t от 0 до 1 — (h + 1)/2 или |h| (как SetColor), при заданном числе уровней — ступенчатый.
/// Выход: t (для шкалы во фрагментном шейдере) и точка сцены (для затенения склонов).
/// @see https://www.tamic.ru/docs/code/fieldview.html#shaders
// Слой сетки (поле или ε): координаты узла (u, v) лежат в видеопамяти один раз,
// значение узла приходит каждый такт. Высота и цвет — как ZtoGLWinCoord и SetColor
// Windows-FieldView.

layout(location = 0) in vec2 uv;      // положение узла в сетке, 0..1 по X и Y
layout(location = 1) in float value;  // значение поля (или eps) в узле в этом кадре

layout(location = 0) out float vT;    // параметр цвета 0..1
layout(location = 1) out vec3 vPos;   // точка сцены (для нормали в grid.frag)

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    vec4 rect;   // x0, y0, sx, sy — размещение сетки в сцене
    vec4 map;    // k, off (h = v·k + off); режим цвета (0: (h+1)/2, 1: |h|); уровней (0 — плавно)
    vec4 misc;   // множитель высоты (0 — плоско); подъём; непрозрачность; 1 — цвет по шкале
    vec4 color;  // цвет каркаса (если не по шкале)
    vec4 extra;  // x — сила затенения склонов
};

void main()
{
    float h = value * map.x + map.y;
    float t = map.z > 0.5 ? abs(h) : (h + 1.0) * 0.5;
    t = clamp(t, 0.0, 1.0);
    if (map.w > 1.5) {
        // Дискретные уровни, как 20 цветов Windows-версии: i = (int)(t·19).
        float i = min(floor(t * (map.w - 1.0)), map.w - 1.0);
        t = i / (map.w - 1.0);
    }
    vT = t;
    vec3 p = vec3(rect.x + uv.x * rect.z, rect.y + uv.y * rect.w, clamp(h, -1.0001, 1.0001) * misc.x + misc.y);
    vPos = p;
    gl_Position = mvp * vec4(p, 1.0);
}
