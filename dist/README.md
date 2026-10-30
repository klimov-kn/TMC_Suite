# dist/ — готовые сборки

Собранные программы **не хранятся в репозитории**. Они публикуются в разделе
[**Releases**](https://github.com/klimov-kn/TMC_Suite/releases): git хранит каждую версию каждого
двоичного файла навсегда, и при каждой пересборке репозиторий рос бы на десятки мегабайт.

## Если хотите собрать сами

Порядок сборки и команды описаны в [`BUILD_ORDER.md`](../BUILD_ORDER.md); подробная пошаговая
инструкция — на сайте проекта [www.tamic.ru](https://www.tamic.ru).

Результаты сборки появятся здесь:

```
dist/
├── win64/
│   ├── bin/                  planarrt_h.exe, planarrt_x.exe, tmcgrout.exe, tmcros.exe,
│   │                         tmc_dn.exe, fieldview.exe, TplDesigner.exe, gen_lr2.exe
│   └── lib/                  библиотеки пакета
└── TMC_Suite_Qt6_win64/      готовая папка для запуска (собирает build/package_win.sh)
```
