/*
**  TAMIC_soft(R) 1989,1990,1991,1992 v1.10
**
**  postpro.
**
**  FILE : a:\include\postpro.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file Postpro.h
/// @brief Постобработка TAMIC: графики S-параметров, кривые, меню вывода, таблица S.
///
/// Включается из s_file.h и sfile95. Используются структуры `INPUT_DATA` и `S_INDEX`
/// (описание входов и S-файлов) и функция `put_S_table` (sfile95, s_table.cpp).
/// Графические структуры и функции — наследие DOS-версии: в исходниках пакета
/// не определены.
/// @warning Защиты от повторного включения нет: структуры `graph_curve`, `_real_point`,
/// `curve_data`, `graph_wnd_config` при втором включении в один файл дают ошибку
/// «redefinition». s_file.h включает этот заголовок, только если `INPUT_DATA` ещё
/// не определена.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#postpro
#include <Typedef.h>
#include <COMPLEX1.H>
#include <FRQDATA.H>
#include <TMCGRAPH.H>






/// Режим кривой: последняя.
#define LAST               2





#ifndef _INPUT_DATA_DEF
 /// Признак того, что `INPUT_DATA` уже определён (защита от повторного определения).
 #define _INPUT_DATA_DEF
 /// @brief Таблица входов устройства: число входов и номера мод на каждом.
 ///
 /// Хранится в заголовке файла S-матрицы (.s) и используется sfile95 для индексации
 /// элементов. Тип полей — `INT_1` (`short`, 2 байта).
 typedef struct
          {
           /// Число входов (портов).
           INT_1 num;
           /// Число мод на каждом входе.
           INT_1 array[MAX_INPUT_NUM];
           /// Размерность S-матрицы — суммарное число мод.
           INT_1 dim;
           }INPUT_DATA;
#endif


#ifndef _S_INDEX_DEF
 /// Признак того, что `S_INDEX` уже определён (защита от повторного определения).
 #define _S_INDEX_DEF
   /// @brief Описание S-файла для поиска и интерполяции: имена и шаги сетки.
   typedef struct
	   {
	    /// Имя файла задания.
	    char *tpl_name;
	    /// Имя файла S-матрицы.
	    char *S_name;
	    /// Первый шаг сетки, единицы длины задания.
	    double delta1;
	    /// Второй шаг сетки.
	    double delta2;
	    /// Третий шаг сетки.
	    double delta3;
	    } S_INDEX;
#endif

#ifndef _MARKER
  #define _MARKER

/// @brief Маркер точки графика. В коде Qt6-версии не используется.
typedef struct
            {
             /// Вид маркера.
             int type;
             /// Толщина линии.
             int width;
             /// Цвет.
             int color;
             /// Размер.
             int size;
             } MARKER;
#endif

#ifndef _CURVE_INDEX
 /// Признак того, что `GRAPH_CURVE` уже определён (защита от повторного определения).
 #define _CURVE_INDEX

/// @brief Стиль кривой графика. В коде Qt6-версии не используется.
typedef struct
            {
             /// Стиль линии.
             int style;
             /// Цвет.
             int color;
             /// Толщина.
             int width;
             } CURVE;

/// @brief Индекс кривой: элемент S-матрицы (вход и мода «откуда» и «куда»). В коде Qt6-версии не используется.
typedef struct
          {
           /// Номер первого входа.
           int input1;       /* input 1 number in device    */
           /// Номер моды на первом входе.
           int mode1;        /* mode 1 number in this input */
           /// Номер второго входа.
           int input2;       /* input 2 number in device    */
           /// Номер моды на втором входе.
           int mode2;        /* mode 2 number in this input */
           } CURVE_INDEX;
#endif

#ifndef GRAPH_CURVE
/// @brief Кривая графика в двусвязном списке. В коде Qt6-версии не используется.
struct graph_curve
            {
             /// Состояние: -1 — удалена, 0 — не используется, > 0 — используется.
             int    code;        /* -1 - deleted, 0 - not used, >0 current used*/
             /// Маркер.
             MARKER m;
             /// Стиль кривой.
             CURVE  crv;
             /// Элемент S-матрицы.
             CURVE_INDEX ci;
             /// Таблица входов.
             INPUT_DATA id;
             /// Файл S-матрицы.
             char   filename[FILENAME_MAX];
             /// Следующая кривая.
             struct graph_curve *next;
             /// Предыдущая кривая.
             struct graph_curve *prev;
             };
/// Синоним `struct graph_curve`.
typedef struct graph_curve GRAPH_CURVE;
#endif

/// @brief Точка с координатами `_real`.
struct _real_point { _real x, y; };
/// Синоним `struct _real_point`.
typedef struct _real_point FLOAT_POINT;
/// @var _real_point::x
/// Координата x.
/// @var _real_point::y
/// Координата y.



/// @brief Точки одной кривой. В коде Qt6-версии не используется.
struct curve_data
            {
             /// Число точек.
             int num;                   /* number of point on curve*/
             /// Массив точек.
             FLOAT_POINT *p;
             };


/// @brief Настройки окна графика DOS-версии: шкалы, сетка, цвета, список кривых. В коде Qt6-версии не используется.
struct graph_wnd_config
            {
             /// Вид шкалы по x (частота, длина волны) и по y (КСВ, потери …).
             int x_scale, y_scale;       /* F, Wl, for x, Kctu, L .. for y*/
             /// Сетка: линейная или логарифмическая.
             int x_style, y_style;       /* graph net style line or log*/
             /// Тип линий сетки.
             int x_type, y_type;         /* graph net line type*/
             /// Толщина линий сетки.
             int x_width, y_width;       /* graph net width*/
             /// Цвет сетки.
             int x_color, y_color;       /* graph net color*/
             /// Цвет подписей шкал.
             int x_sc_color, y_sc_color; /* graph scale color*/
             /// Цвет, стиль и толщина рамки.
             int border_color, border_style, border_width;  /* border color*/
             /// Фон окна и поля графика.
             int general_back_color, graph_back_color;      /* graph feald color*/
             /// Окно графика, экранные координаты.
             WINDOW w;                                      /* graph int window*/
             /// Окно графика, координаты данных.
             struct float_window fw;                        /* graph float window*/
             /// Число кривых.
             int curve_num;                   /* number of graph curve*/
             /// Список кривых.
             GRAPH_CURVE *gc;            /* pointer to graph curve list*/
             /// Список нулевых кривых.
             GRAPH_CURVE *zerro;         /* pointer to graph curve zerro list*/
             /// S-файлы для экстраполяции.
             S_INDEX si[3];                   /* s-matrix index for extrapolation*/
             /// Размер шрифта подписей шкал.
             struct set_char_size *ss;   /* scale char size*/
             /// @var graph_wnd_config::y_scale
             /// Вид шкалы по y (КСВ, потери …).
             /// @var graph_wnd_config::y_style
             /// Сетка по y: линейная или логарифмическая.
             /// @var graph_wnd_config::y_type
             /// Тип линий сетки по y.
             /// @var graph_wnd_config::y_width
             /// Толщина линий сетки по y.
             /// @var graph_wnd_config::y_color
             /// Цвет сетки по y.
             /// @var graph_wnd_config::y_sc_color
             /// Цвет подписей шкалы y.
             /// @var graph_wnd_config::border_style
             /// Стиль рамки.
             /// @var graph_wnd_config::border_width
             /// Толщина рамки.
             /// @var graph_wnd_config::graph_back_color
             /// Фон поля графика.
             };

/// @cond
// Печатает таблицу S-матрицы в листинг. Повторное объявление; описание — в proc_s.h.
// Определена в sfile95 (s_table.cpp); вызывает функции листинга, которых в сборке нет, и в Qt6-версии не вызывается.
/// @endcond

