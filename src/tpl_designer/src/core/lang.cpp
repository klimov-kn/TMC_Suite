/// @file lang.cpp
/// @brief Хранение флага языка текстов TPL Designer.
///
/// Часть ядра генератора; объявления и описание — в lang.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#translation
#include "lang.h"

namespace tpl {

namespace {
/// Флаг действующего языка: true — английский, false — русский (по умолчанию).
bool g_english = false;
}

void setEnglish(bool on) { g_english = on; }
bool english() { return g_english; }

} // namespace tpl
