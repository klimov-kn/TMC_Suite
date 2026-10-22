// pathcheck.h — можно ли передать путь коду Windows-версии.
//
// Код, перенесённый из Windows-версии, открывает файлы ANSI-функциями (fopen),
// поэтому путь переводится в кодовую страницу Windows (toLocal8Bit). Буквы вне
// неё Windows молча заменяет похожими (é -> e) или знаком «?», и программа
// открыла бы файл из ДРУГОЙ папки (café\ -> cafe\) или не нашла бы его (Т-20).
// Поэтому такой путь не передаём, а говорим об этом человеку.

/// @file pathcheck.h
/// @brief Проверка, что путь можно без потерь перевести в кодовую страницу Windows.
///
/// Часть модуля «Общий слой интерфейса» (только заголовок).
/// @see https://www.tamic.ru/docs/code/gui-common.html#path-check
#pragma once

#include <QString>
#include <QByteArray>

/// @brief Мелкие средства надёжности интерфейса.
namespace tmcui {

/// Путь переводится в кодовую страницу Windows без потерь.
/// @param[in] path  путь
/// @return true — `toLocal8Bit` и обратное преобразование дают исходную строку.
inline bool pathFitsCodePage(const QString &path)
{
    return QString::fromLocal8Bit(path.toLocal8Bit()) == path;
}

} // namespace tmcui
