# -*- coding: utf-8 -*-
#
# Конфигурация Sphinx для документа
# «TMC Suite — Инструкция по сборке».
#
# Содержимое перенесено 1:1 из Markdown-инструкций docs/build-guide/.
# HTML собирается командой sphinx-build (выполняет оператор).

project = 'TMC Suite — Инструкция по сборке'
author = 'К.Н. Климов'
copyright = '2026, К.Н. Климов'

language = 'ru'

# Базовых возможностей Sphinx достаточно — дополнительные расширения не нужны.
extensions = []

# Корневой документ (toctree верхнего уровня).
master_doc = 'index'
root_doc = 'index'

# Тема оформления входит в состав Sphinx, доустанавливать ничего не нужно.
html_theme = 'alabaster'

# Папку статических файлов не используем, чтобы Sphinx не предупреждал
# об отсутствующей директории _static.
html_static_path = []

# Кодировка исходных файлов — UTF-8.
source_encoding = 'utf-8'

# Шаблоны (не используются, но путь стандартный).
templates_path = ['_templates']

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
