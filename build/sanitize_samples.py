# -*- coding: utf-8 -*-
"""Образцы в поставке без личных путей машины сборки (Т-35, п. 4).

Ядра и вьюверы записывают в результаты и документы полный путь файла — комментарием
(`!File name I:\\...\\demo_h.t`, `!file name : I:\\...\\r7.dop`) или строкой в заголовке
двоичного `.s`. В поставке этот путь — путь машины разработчика. Скрипт заменяет его
именем файла; числа и остальные строки не меняются. Служебные файлы, которые вьюверы
создают сами при открытии (`*.$os`, `*.$oc`, `*.$op`, `*_Temp0`, `*_Temp1`), и резервные
копии `*.bak` из поставки убираются — программа создаст их заново.

    python -X utf8 sanitize_samples.py <папка samples поставки>
"""
import os, re, sys

SERVICE = re.compile(r'(\.\$os|\.\$oc|\.\$op|_temp0|_temp1|\.bak)$', re.I)
TEXT_EXT = ('.t', '.tos', '.toc', '.soc', '.dop')
COMMENT = re.compile(rb'^(!\s*file name\s*:?\s*)([A-Za-z]:\\[^\r\n]*?)(?=\r?$)', re.I | re.M)


def fix_text(path):
    data = open(path, 'rb').read()
    new = COMMENT.sub(lambda m: m.group(1) + os.path.basename(m.group(2).decode('cp1251')).encode('cp1251'), data)
    if new != data:
        open(path, 'wb').write(new)
        return True
    return False


def fix_s(path):
    """Двоичный .s: записи «длина (2 байта) + байты + \\r\\n». Заменяется запись,
    в которой лежит полный путь самого файла."""
    data = open(path, 'rb').read()
    if not data.startswith(b'SMT\x00'):
        return False
    # Есть и числовые записи, поэтому файл не разбирается целиком: ищется полный
    # путь, перед которым стоит его же длина.
    out, last, changed = [], 0, False
    for m in re.finditer(rb'[A-Za-z]:\\[^\r\n\x00]*?\.s(?=\r\n)', data, re.I):
        a = m.start()
        if a < 2 or (data[a - 2] | (data[a - 1] << 8)) != len(m.group(0)):
            continue
        # Замена ТОЙ ЖЕ длины: при более короткой записи файл перестаёт читаться
        # (проверено sdump). Относительный путь «..\.\.\имя.s» указывает на тот же файл.
        base = os.path.basename(m.group(0).decode('cp1251')).encode('cp1251')
        pad = len(m.group(0)) - len(base)
        prefix = (b'..\\' if pad % 2 else b'') + b'.\\' * ((pad - 3 * (pad % 2)) // 2)
        out += [data[last:a], prefix + base]
        last, changed = m.end(), True
    if changed:
        out.append(data[last:])
        open(path, 'wb').write(b''.join(out))
    return changed


def main():
    root = sys.argv[1]
    removed = fixed = 0
    for d, _, files in os.walk(root):
        for f in files:
            p = os.path.join(d, f)
            if SERVICE.search(f):
                os.remove(p)
                removed += 1
            elif f.lower().endswith(TEXT_EXT):
                fixed += fix_text(p)
            elif f.lower().endswith('.s'):
                fixed += fix_s(p)
    left = []
    for d, _, files in os.walk(root):
        for f in files:
            if b'Python Project' in open(os.path.join(d, f), 'rb').read():
                left.append(os.path.join(d, f))
    print('samples: убрано служебных %d, путей заменено %d, осталось с личным путём %d %s'
          % (removed, fixed, len(left), left))
    sys.exit(1 if left else 0)


if __name__ == '__main__':
    main()
