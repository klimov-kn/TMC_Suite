# Консольная генерация задания (входит в поставку): проверка эмиттера прогоном ядра без окон.
add_executable(gen_lr2 gen_lr2.cpp)
target_link_libraries(gen_lr2 PRIVATE tpl_core)
