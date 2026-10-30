/*
**  TAMIC_soft(R) 1989,1990,1991 v1.00
**
** Macros and declarations for Tamic_H graphics program
**
** FILE: a:\include\tmcgraph.h>
**
*/

/// @file TMCGRAPH.H
/// @brief Графика DOS-версии TAMIC: объявления Borland BGI и библиотеки меню и окон TAMIC.
///
/// Наследие пакета TAMIC (DOS). Первая часть — копия объявлений Borland BGI (`graphics.h`):
/// коды ошибок, драйверы, режимы, цвета, стили линий и заливки, структуры настроек и
/// функции рисования (`arc`, `bar`, `line`, `outtextxy`, `initgraph` …). Вторая часть —
/// собственная DOS-библиотека TAMIC: меню (`bar_menu`, `window_menu`), редактор строки
/// (`string_to_edit`), нормирующие преобразования (`norm_transf`), маркеры и кнопки.
/// Включается из `postpro.h` и из sfile95 (`read_s.cpp`, `write_s.cpp`, `wrrd_s.cpp`).
/// Ни одна функция заголовка в исходниках пакета не определена и не вызывается; в
/// Qt6-версии рисование идёт через слой GDI поверх QPainter (`compat/`).
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _TMC_GRPH_H
  #define _TMC_GRPH_H


#if __STDC__
/// Модификатор соглашения о вызове C; при `__STDC__` пустой.
#define _Cdecl
#else
/// Модификатор соглашения о вызове C: ключевое слово `cdecl` старых компиляторов.
#define _Cdecl	cdecl
#endif

#if     !defined(__GRAPHX_DEF_)
/// Признак того, что объявления BGI (перечисления, структуры, функции) уже включены.
#define __GRAPHX_DEF_

/// @brief Коды ошибок графики BGI, возвращаемые `graphresult`: `grOk` (0) — успех, отрицательные — ошибка.
enum graphics_errors
  {    /* graphresult error return codes */
   grOk               =   0,
   grNoInitGraph      =  -1,
   grNotDetected      =  -2,
   grFileNotFound     =  -3,
   grInvalidDriver    =  -4,
   grNoLoadMem        =  -5,
   grNoScanMem        =  -6,
   grNoFloodMem       =  -7,
   grFontNotFound     =  -8,
   grNoFontMem        =  -9,
   grInvalidMode      = -10,
   grError            = -11,   /* generic error */
   grIOerror          = -12,
   grInvalidFont      = -13,
   grInvalidFontNum   = -14,
   grInvalidVersion   = -18
   };

/// @brief Коды графических драйверов BGI (аргумент `initgraph`, `detectgraph`); `DETECT` — автоопределение адаптера.
enum graphics_drivers
  {   /* define graphics drivers */
   DETECT    = 0,   /* requests autodetection */
   CGA       = 1,   /* IBM CGA in 320*200 color (mode 0x04)         */
   MCGA      = 2,   /* IBM CGA in 640*200 (mode 0x06)               */
   EGA       = 3,   /* EGA                                          */
   EGA64     = 4,
   EGAMONO   = 5,
   IBM8514   = 6,        /* 1 - 6 */
   HERCMONO  = 7,
   ATT400    = 8,
   VGA       = 9,
   PC3270    = 10,                  /* 7 - 10 */
   ORCHIDPRO = 11,  /* ORCHID PRO (obsolete use FG_TSENGHIRES).   */
   TSENG     = 12,  /* Tseng Labs 800 x 600, 16 colors.           */
   PARADISE  = 13,  /* PARADISE VGA 800 x 600, 16 colors          */
   TRIDENT   = 14,  /* TRIDENT VGA 800 x 600, 16 colors           */
   ATI       = 15,  /* ATI                                        */
   VEGAVGA   = 16,  /* VIDEO 7 VEGA VGA in 800 x 600, 16 colors   */
   DFI       = 17,  /* Diamond Flower Instruments 800 x 600 x 16  */
   VESA      = 18,  /* VESA                                       */
   EVGA      = 19,  /* Everex EVGA in 800 x 600, 16 colors        */
   TOSHIBA   = 20,  /* Toshiba J-3100                             */
   TIGA      = 21,  /* Texas Instruments TMS340 Family - TIGA 2.0 */
   CURRENT_DRIVER = -1
   };

/// @brief Графические режимы драйверов BGI (аргумент `initgraph`, `setgraphmode`); разрешение и число страниц — в пояснениях к элементам.
enum graphics_modes
   {   /* graphics modes for each driver */
    CGAC0      = 0,  /* 320x200 palette 0; 1 page	*/
    CGAC1      = 1,  /* 320x200 palette 1; 1 page	*/
    CGAC2      = 2,  /* 320x200 palette 2: 1 page	*/
    CGAC3      = 3,  /* 320x200 palette 3; 1 page	*/
    CGAHI      = 4,  /* 640x200 1 page			*/
    MCGAC0     = 0,  /* 320x200 palette 0; 1 page	*/
    MCGAC1     = 1,  /* 320x200 palette 1; 1 page	*/
    MCGAC2     = 2,  /* 320x200 palette 2; 1 page	*/
    MCGAC3     = 3,  /* 320x200 palette 3; 1 page	*/
    MCGAMED    = 4,  /* 640x200 1 page			*/
    MCGAHI     = 5,  /* 640x480 1 page			*/
    EGALO      = 0,  /* 640x200 16 color 4 pages	*/
    EGAHI      = 1,  /* 640x350 16 color 2 pages	*/
    EGA64LO    = 0,  /* 640x200 16 color 1 page 	*/
    EGA64HI    = 1,  /* 640x350 4 color  1 page 	*/
    EGAMONOHI  = 0,  /* 640x350 64K on card, 1 page - 256K on card, 4 pages */
    HERCMONOHI = 0,  /* 720x348 2 pages 		*/
    HERCHALF   = 1,  /* Hercules monochrome, 1 display page  */
    ATT400C0   = 0,  /* 320x200 palette 0; 1 page	*/
    ATT400C1   = 1,  /* 320x200 palette 1; 1 page	*/
    ATT400C2   = 2,  /* 320x200 palette 2; 1 page	*/
    ATT400C3   = 3,  /* 320x200 palette 3; 1 page	*/
    ATT400MED  = 4,  /* 640x200 1 page              */
    ATT400HI   = 5,  /* 640x400 1 page              */
    VGALO      = 0,  /* 640x200 16 color 4 pages	*/
    VGAMED     = 1,  /* 640x350 16 color 2 pages	*/
    VGAHI      = 2,  /* 640x480 16 color 1 page 	*/
    VGAMONO    = 3,  /* IBM VGA in 640*480, 1 color */
    PC3270HI   = 0,  /* 720x350 1 page              */
    IBM8514LO  = 0,  /* 640x480 256 colors          */
    IBM8514HI  = 1,  /*1024x768 256 colors          */

    ORCHIDPROHIRES = 0, /* ORCHID PRO (obsolete use FG_TSENGHIRES).   */
    TSENGHIRES     = 0, /* Tseng Labs 800 x 600, 16 colors.           */
    PARADISEHIRES  = 0, /* PARADISE VGA 800 x 600, 16 colors          */
    TRIDENTHIRES   = 0, /* TRIDENT VGA 800 x 600, 16 colors           */
    ATI61          = 0, /* ATI 640 x 400, 256 colors.                 */
    ATI62          = 1, /* ATI 640 x 480, 256 colors.                 */
    ATI63          = 2, /* ATI 800 x 600, 256 colors.                 */
    VEGAVGAHIRES   = 0, /* VIDEO 7 VEGA VGA in 800 x 600, 16 colors   */
    DFIHIRES       = 0, /* Diamond Flower Instruments 800 x 600 x 16  */
    VESA0          = 0, /* VESA mode 0x100, 640 x 400, 256 colors.    */
    VESA1          = 1, /* VESA mode 0x101, 640 x 480, 256 colors.    */
    VESA2          = 2, /* VESA mode 0x102, 800 x 600, 16 colors.     */
    VESA3          = 3, /* VESA mode 0x103, 800 x 600, 256 colors.    */
    VESA4          = 4, /* VESA mode 0x104, 1024 x 768, 16 colors.    */
    VESA5          = 5, /* VESA mode 0x105, 1024 x 768, 256 colors.   */
    VESA6          = 6, /* VESA mode 0x106, 1280 x 1024, 16 colors.   */
    VESA7          = 7, /* VESA mode 0x107, 1280 x 1024, 256 colors.  */
    VESA6A         = 8, /* VESA mode 0x6A, 800 x 600, 16 colors.      */
    EVGAHIRES      = 0, /* Everex EVGA in 800 x 600, 16 colors        */
    TOSHIBAJ_3100  = 0, /* Toshiba J-3100                             */
    TIGA20         = 0  /* Texas Instruments TMS340 Family - TIGA 2.0 */
    };

/* Colors for setpalette and setallpalette */

#if	!defined(__COLORS)
/// Признак того, что имена цветов уже объявлены.
#define __COLORS


#endif


  /// @brief Стили линии BGI для `setlinestyle` и `getlinesettings`; `USERBIT_LINE` — образец пользователя из 16 бит.
  enum line_styles 
   {      /* Line styles for get/setlinestyle */
	SOLID_LINE   = 0,
	DOTTED_LINE  = 1,
	CENTER_LINE  = 2,
	DASHED_LINE  = 3,
	USERBIT_LINE = 4,	/* User defined line style */
    };

/// @brief Толщина линии для `setlinestyle`: 1 или 3 пикселя.
enum line_widths 
   {      /* Line widths for get/setlinestyle */
	NORM_WIDTH  = 1,
	THICK_WIDTH = 3,
    };

/// @brief Шрифты BGI для `settextstyle`: растровый 8x8 и векторные шрифты.
enum font_names {
  DEFAULT_FONT    = 0,  /* 8x8 bit mapped font */
  TRIPLEX_FONT    = 1,  /* "Stroked" fonts */
  SMALL_FONT      = 2,
  SANS_SERIF_FONT = 3,
  GOTHIC_FONT     = 4
  };

/// @name Направление вывода текста (аргумент `direction` функции `settextstyle`)
///@{
/// Направление текста: `HORIZ_DIR` — слева направо, `VERT_DIR` — снизу вверх.
#define HORIZ_DIR	0	/* left to right */
#define VERT_DIR	1	/* bottom to top */
///@}

/// Размер символов для `settextstyle`: 0 — размер задаётся функцией `setusercharsize`.
#define USER_CHAR_SIZE	0	/* user-defined char size */

  /// @brief Образцы заливки BGI для `setfillstyle` и `getfillsettings`; `USER_FILL` — образец пользователя (`setfillpattern`).
  enum fill_patterns 
   {        /* Fill patterns for get/setfillstyle */
    EMPTY_FILL,     /* fills area in background color */
    SOLID_FILL,     /* fills area in solid fill color */
    LINE_FILL,      /* --- fill */
    LTSLASH_FILL,   /* /// fill */
    SLASH_FILL,     /* /// fill with thick lines */
    BKSLASH_FILL,   /* \\\ fill with thick lines */
    LTBKSLASH_FILL, /* \\\ fill */
    HATCH_FILL,     /* light hatch fill */
    XHATCH_FILL,    /* heavy cross hatch fill */
    INTERLEAVE_FILL,/* interleaving line fill */
    WIDE_DOT_FILL,  /* Widely spaced dot fill */
    CLOSE_DOT_FILL, /* Closely spaced dot fill */
    USER_FILL       /* user defined fill */
    };

/// @brief Способ наложения изображения в `putimage` (и режим `setwritemode`): копирование, XOR, OR, AND, инверсия.
enum putimage_ops {		/* BitBlt operators for putimage */
	COPY_PUT,		/* MOV */
	XOR_PUT,		/* XOR */
	OR_PUT, 		/* OR  */
	AND_PUT,		/* AND */
	NOT_PUT 		/* NOT */
};

/// @brief Выравнивание текста для `settextjustify`: по горизонтали (`LEFT_TEXT` … `RIGHT_TEXT`) и по вертикали (`BOTTOM_TEXT` … `TOP_TEXT`).
enum text_just {		/* Horizontal and vertical justification
				   for settextjustify */
  LEFT_TEXT     = 0,
  CENTER_TEXT	= 1,
  RIGHT_TEXT	= 2,

  BOTTOM_TEXT   = 0,
/*CENTER_TEXT   = 1,  already defined above */
  TOP_TEXT      = 2
  };


/// Наибольший номер элемента палитры; размер массива `palettetype::colors` — `MAXCOLORS+1`.
#define MAXCOLORS 15

/// @brief Палитра BGI (`getpalette`, `setallpalette`, `getdefaultpalette`).
struct palettetype {
        /// Число элементов палитры.
        unsigned char size;
	/// Коды цветов элементов палитры; -1 в `setallpalette` — элемент не меняется.
	signed char colors[MAXCOLORS+1];
};

/// @brief Настройки линии (`getlinesettings`).
struct linesettingstype {
	/// Стиль линии, значение `line_styles`.
	int linestyle;
	/// Образец линии из 16 бит; действует при `USERBIT_LINE`.
	unsigned upattern;
	/// Толщина линии, значение `line_widths` (пиксели).
	int thickness;
};

/// @brief Настройки вывода текста (`gettextsettings`).
struct textsettingstype {
	/// Шрифт, значение `font_names`.
	int font;
	/// Направление: `HORIZ_DIR` или `VERT_DIR`.
	int direction;
	/// Множитель размера символов; `USER_CHAR_SIZE` — размер из `setusercharsize`.
	int charsize;
	/// Выравнивание по горизонтали, значение `text_just`.
	int horiz;
	/// Выравнивание по вертикали, значение `text_just`.
	int vert;
};

/// @brief Настройки заливки (`getfillsettings`).
struct fillsettingstype {
	/// Образец заливки, значение `fill_patterns`.
	int pattern;
	/// Цвет заливки.
	int color;
};

/// @brief Окно вывода (`getviewsettings`): границы в координатах экрана и флаг отсечения.
struct viewporttype {
	/// @name Границы окна вывода
	///@{
	/// Границы окна, пиксели экрана.
	int left, top, right, bottom;
	///@}
	/// Флаг отсечения: ненулевое — рисунок обрезается по границам окна.
	int clip;
};

/// @brief Координаты последней нарисованной дуги (`getarccoords`).
struct arccoordstype 
   {
	/// @name Центр дуги
	///@{
	/// Центр дуги, пиксели.
	int x, y;
	///@}
	/// @name Концы дуги
	///@{
	/// Начальная (`xstart`, `ystart`) и конечная (`xend`, `yend`) точки дуги, пиксели.
	int xstart, ystart, xend, yend;
	///@}
    };

/// @brief Рисует дугу окружности текущим цветом.
/// @param[in] x  центр по X, пиксели
/// @param[in] y  центр по Y, пиксели
/// @param[in] stangle  начальный угол, градусы (отсчёт от направления вправо против часовой стрелки)
/// @param[in] endangle  конечный угол, градусы
/// @param[in] radius  радиус, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl arc( int x, int y, int stangle, int endangle, int radius);
/// @brief Закрашивает прямоугольник текущим образцом и цветом заливки, без рамки.
/// @param[in] left  левая граница, пиксели
/// @param[in] top  верхняя граница, пиксели
/// @param[in] right  правая граница, пиксели
/// @param[in] bottom  нижняя граница, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void  _Cdecl bar( int left, int top, int right, int bottom );
/// @brief Рисует окружность текущим цветом.
/// @param[in] x  центр по X, пиксели
/// @param[in] y  центр по Y, пиксели
/// @param[in] radius  радиус, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl circle( int x, int y, int radius);
/// @brief Определяет установленный графический адаптер и наибольший режим для него.
/// @param[out] graphdriver  код драйвера (`graphics_drivers`); при неудаче — `grNotDetected`
/// @param[out] graphmode  код режима (`graphics_modes`)
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl detectgraph(int *graphdriver,int *graphmode);
/// @brief Возвращает координаты последней нарисованной дуги.
/// @param[out] arccoords  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getarccoords(struct arccoordstype *arccoords);
/// @brief Возвращает палитру драйвера по умолчанию.
/// @return Указатель на палитру; принадлежит графической системе, не освобождать.
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
struct palettetype * _Cdecl getdefaultpalette( void );
/// @brief Возвращает текущие образец и цвет заливки.
/// @param[out] fillinfo  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getfillsettings(struct fillsettingstype *fillinfo);
/// @brief Сохраняет прямоугольную область экрана в буфер.
/// @param[in] left  левая граница, пиксели
/// @param[in] top  верхняя граница, пиксели
/// @param[in] right  правая граница, пиксели
/// @param[in] bottom  нижняя граница, пиксели
/// @param[out] bitmap  буфер размером `imagesize(left, top, right, bottom)` байт
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getimage( int left, int top, int right, int bottom,
                            void *bitmap);
/// @brief Возвращает текущие настройки линии.
/// @param[out] lineinfo  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getlinesettings(struct linesettingstype *lineinfo);
/// @brief Возвращает текущую палитру.
/// @param[out] palette  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getpalette(struct palettetype *palette);
/// @brief Возвращает текущие настройки вывода текста.
/// @param[out] texttypeinfo  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl gettextsettings(struct textsettingstype *texttypeinfo);
/// @brief Возвращает текущее окно вывода и флаг отсечения.
/// @param[out] viewport  структура для результата
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl getviewsettings(struct viewporttype *viewport);
/// @brief Возвращает код ошибки последней графической операции и сбрасывает его в `grOk`.
/// @return Код ошибки (`graphics_errors`).
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int	   _Cdecl graphresult(void);
/// @brief Возвращает размер буфера для сохранения области экрана функцией `getimage`.
/// @param[in] left  левая граница, пиксели
/// @param[in] top  верхняя граница, пиксели
/// @param[in] right  правая граница, пиксели
/// @param[in] bottom  нижняя граница, пиксели
/// @return Размер, байты.
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
unsigned   _Cdecl imagesize(int left, int top, int right, int bottom);
/// @brief Включает графический режим: загружает драйвер и устанавливает режим; итог — в `graphresult`.
/// @param[in,out] graphdriver  код драйвера (`graphics_drivers`); `DETECT` — автоопределение; на выходе — выбранный драйвер или код ошибки
/// @param[in,out] graphmode  код режима (`graphics_modes`); при `DETECT` на выходе — выбранный режим
/// @param[in] pathtodriver  каталог файлов драйверов (.BGI); пустая строка — текущий каталог
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl initgraph(int  *graphdriver, int  *graphmode, char *pathtodriver);
/*int        far _Cdecl installuserdriver( char far *name, int huge (*detect)(void) );*/
/// @brief Подключает векторный шрифт из файла (.CHR).
/// @param[in] name  имя файла шрифта
/// @return Номер шрифта для `settextstyle`.
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int	   _Cdecl installuserfont( char *name );
/// @brief Рисует отрезок текущим цветом и стилем линии; текущая позиция не меняется.
/// @param[in] x1  начало по X, пиксели
/// @param[in] y1  начало по Y, пиксели
/// @param[in] x2  конец по X, пиксели
/// @param[in] y2  конец по Y, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl line(int x1, int y1, int x2, int y2);
/// @brief Выводит строку текста в точке (x, y) текущим шрифтом.
/// @param[in] x  координата X, пиксели
/// @param[in] y  координата Y, пиксели
/// @param[in] textstring  строка с нулём в конце
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl outtextxy(int x, int y, char *textstring);
/// @brief Выводит на экран изображение, сохранённое функцией `getimage`.
/// @param[in] left  левый край изображения, пиксели
/// @param[in] top  верхний край изображения, пиксели
/// @param[in] bitmap  буфер, заполненный `getimage`
/// @param[in] op  способ наложения, значение `putimage_ops`
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl putimage(int left, int top, void *bitmap, int op);
/// @brief Рисует рамку прямоугольника текущим цветом и стилем линии.
/// @param[in] left  левая граница, пиксели
/// @param[in] top  верхняя граница, пиксели
/// @param[in] right  правая граница, пиксели
/// @param[in] bottom  нижняя граница, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl rectangle(int left, int top, int right, int bottom);
/// @brief Рисует эллиптический сектор и закрашивает его текущим образцом и цветом заливки.
/// @param[in] X  центр по X, пиксели
/// @param[in] Y  центр по Y, пиксели
/// @param[in] StAngle  начальный угол, градусы (отсчёт от направления вправо против часовой стрелки)
/// @param[in] EndAngle  конечный угол, градусы
/// @param[in] XRadius  полуось по X, пиксели
/// @param[in] YRadius  полуось по Y, пиксели
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl sector( int X, int Y, int StAngle, int EndAngle,
          int XRadius, int YRadius );
/// @brief Задаёт всю палитру.
/// @param[in] palette  новая палитра; элемент -1 не меняется
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setallpalette(struct palettetype *palette);
/// @brief Задаёт образец заливки пользователя и цвет заливки.
/// @param[in] upattern  образец 8x8 точек, 8 байт
/// @param[in] color  цвет заливки
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setfillpattern(char *upattern, int color);
/// @brief Задаёт образец и цвет заливки.
/// @param[in] pattern  образец, значение `fill_patterns` (кроме `USER_FILL`)
/// @param[in] color  цвет заливки
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setfillstyle(int pattern, int color);
/// @brief Переключает графический режим, очищает экран и сбрасывает настройки к значениям по умолчанию.
/// @param[in] mode  код режима текущего драйвера (`graphics_modes`)
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setgraphmode(int mode);
/// @brief Задаёт стиль, образец и толщину линии.
/// @param[in] linestyle  стиль, значение `line_styles`
/// @param[in] upattern  образец из 16 бит; действует при `USERBIT_LINE`
/// @param[in] thickness  толщина, значение `line_widths` (пиксели)
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setlinestyle(int linestyle, unsigned upattern,
         int thickness);
/// @brief Меняет один элемент палитры.
/// @param[in] colornum  номер элемента палитры
/// @param[in] color  код цвета адаптера
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setpalette(int colornum, int color);
/// @brief Задаёт выравнивание текста относительно точки вывода.
/// @param[in] horiz  по горизонтали: `LEFT_TEXT`, `CENTER_TEXT`, `RIGHT_TEXT`
/// @param[in] vert  по вертикали: `BOTTOM_TEXT`, `CENTER_TEXT`, `TOP_TEXT`
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl settextjustify(int horiz, int vert);
/// @brief Задаёт шрифт, направление и размер текста.
/// @param[in] font  шрифт, значение `font_names` или номер из `installuserfont`
/// @param[in] direction  `HORIZ_DIR` или `VERT_DIR`
/// @param[in] charsize  множитель размера 1–10; `USER_CHAR_SIZE` — размер из `setusercharsize`
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl settextstyle(int font, int direction, int charsize);
/// @brief Задаёт масштаб векторного шрифта: ширина умножается на multx/divx, высота — на multy/divy.
/// @param[in] multx  множитель по ширине
/// @param[in] divx  делитель по ширине
/// @param[in] multy  множитель по высоте
/// @param[in] divy  делитель по высоте
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setusercharsize(int multx, int divx, int multy, int divy);
/// @brief Задаёт окно вывода; координаты рисования отсчитываются от его левого верхнего угла.
/// @param[in] left  левая граница, пиксели
/// @param[in] top  верхняя граница, пиксели
/// @param[in] right  правая граница, пиксели
/// @param[in] bottom  нижняя граница, пиксели
/// @param[in] clip  `CLIP_ON` — обрезать рисунок по окну, `CLIP_OFF` — не обрезать
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setviewport(int left, int top, int right, int bottom, int clip);
/// @brief Задаёт режим рисования линий.
/// @param[in] mode  `COPY_PUT` — замена точек, `XOR_PUT` — исключающее ИЛИ
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void   _Cdecl setwritemode( int mode );

/***** graphics drivers *****/

/// @brief Регистрирует графический драйвер, скомпонованный с программой (вместо загрузки файла .BGI).
/// @param[in] driver  точка входа драйвера, например `EGAVGA_driver`
/// @return Номер драйвера; отрицательное значение — код ошибки (`graphics_errors`).
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int    _Cdecl registerbgidriver(void (*driver)(void));
/// @brief Регистрирует графический драйвер, скомпонованный с программой в отдельном (дальнем) сегменте.
/// @param[in] driver  адрес образа драйвера, например `EGAVGA_driver_far`
/// @return Номер драйвера; отрицательное значение — код ошибки (`graphics_errors`).
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int	   _Cdecl registerfarbgidriver(void *driver);
/// @brief Точка входа драйвера EGA и VGA для компоновки с программой; не вызывается, передаётся в `registerbgidriver`.
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void	       _Cdecl EGAVGA_driver(void);
/// Образ драйвера EGA и VGA в дальнем сегменте для `registerfarbgidriver`. Определения в исходниках пакета нет.
extern int EGAVGA_driver_far[];

/***** graphics fonts *****/

/// @brief Регистрирует векторный шрифт, скомпонованный с программой (вместо загрузки файла .CHR).
/// @param[in] font  точка входа шрифта, например `triplex_font`
/// @return Номер шрифта; отрицательное значение — код ошибки (`graphics_errors`).
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int	       _Cdecl registerbgifont(void (*font)(void));
/// @brief Регистрирует векторный шрифт, скомпонованный с программой в отдельном (дальнем) сегменте.
/// @param[in] font  адрес образа шрифта, например `triplex_font_far`
/// @return Номер шрифта; отрицательное значение — код ошибки (`graphics_errors`).
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
int	   _Cdecl registerfarbgifont(void *font);

/* !!	These "functions" are NOT user-callable,   !! */
/* !! they are there just so you can link in fonts !! */

/// @brief Точка входа шрифта Triplex для компоновки с программой; не вызывается, передаётся в `registerbgifont`.
/// @note Определения в исходниках пакета нет (объявление BGI); в Qt6-версии не вызывается.
void	       _Cdecl	triplex_font(void);

/// Образ шрифта Triplex в дальнем сегменте для `registerfarbgifont`. Определения в исходниках пакета нет.
extern int triplex_font_far[];
/// Образ шрифта Small в дальнем сегменте для `registerfarbgifont`. Определения в исходниках пакета нет.
extern int small_font_far[];
/// Образ шрифта Sans Serif в дальнем сегменте для `registerfarbgifont`. Определения в исходниках пакета нет.
extern int sansserif_font_far[];
/// Образ шрифта Gothic в дальнем сегменте для `registerfarbgifont`. Определения в исходниках пакета нет.
extern int gothic_font_far[];


#endif

#ifndef _VIDEOCONFIG_DEFINED

                /* structure for _getvideoconfig() as visible to user */
/// @brief Параметры видеосистемы (по образцу `_getvideoconfig`): размеры экрана, цвета, соотношение сторон, адаптер.
struct videoconfig {
        /// Число точек по оси X.
        int numxpixels;       /* number of pixels on X axis */
        /// Число точек по оси Y.
        int numypixels;       /* number of pixels on Y axis */
        /// Число текстовых столбцов.
        int numtextcols;      /* number of text columns available */
        /// Число текстовых строк.
        int numtextrows;      /* number of text rows available */
        /// Число цветов.
        int numcolors;        /* number of actual colors */
        /// Палитра.
        struct palettetype pt;/* palette                 */
        /// Коэффициент соотношения сторон по X.
        int xasp;             /* x aspect */
        /// Коэффициент соотношения сторон по Y.
        int yasp;             /* y aspect */
        /// Программный коэффициент по X (используется макросом `x_ftoint`).
        float x_asp;          /* software x aspect */
        /// Программный коэффициент по Y (используется макросом `y_ftoint`).
        float y_asp;          /* software y aspect */
        /// Соотношение сторон экрана.
        double AspectRatio;   /* aspect ratio  */
        /// Число бит на точку.
        int bitsperpixel;     /* number of bits per pixel */
        /// Число видеостраниц.
        int numvideopages;    /* number of available video pages */
        /// Текущий видеорежим.
        int mode;             /* current video mode */
        /// Действующий видеоадаптер.
        int adapter;          /* active display adapter */
        /// Действующий монитор.
        int monitor;          /* active display monitor */
        /// Видеопамять адаптера, Кбайт.
        int memory;           /* adapter video memory in K bytes */
        /// Код ошибки.
        int error;            /* erro code    */
};
/// Признак того, что структура `videoconfig` уже объявлена.
#define _VIDEOCONFIG_DEFINED
#endif

/// @name Флаги отсечения окна вывода
///@{
/// Флаг отсечения для `setviewport`: `CLIP_ON` (1) — рисунок обрезается по окну, `CLIP_OFF` (0) — нет.
#define CLIP_ON   1
#define CLIP_OFF  0
///@}

/// @name Стиль выбора графического режима TAMIC
///@{
/// Аргумент `set_init_style`: `MAX_COLOR_STYLE` — по имени режим с наибольшим числом цветов, `HIGH_RES_STYLE` — с наибольшим разрешением.
#define MAX_COLOR_STYLE  0
#define HIGH_RES_STYLE   1
///@}

///@}

/// @brief Переводит вещественную величину в целые пиксели по X: `(int)(f * vc.x_asp)`.
/// @param f  величина в программных единицах
/// @note Переменная `vc` (`struct videoconfig`) в этом заголовке не объявлена.
#define x_ftoint( f )  ( (int)( f * vc.x_asp ) )
/// @brief Переводит вещественную величину в целые пиксели по Y: `(int)(f * vc.y_asp)`.
/// @param f  величина в программных единицах
/// @note Переменная `vc` (`struct videoconfig`) в этом заголовке не объявлена.
#define y_ftoint( f )  ( (int)( f * vc.y_asp ) )

/// @name Флаг тени окна
///@{
/// Флаг тени (аргумент `shadow_flag` функции `text_in_box`): `NO_SHADOW` (0) — без тени, `YES_SHADOW` (1) — с тенью.
#define NO_SHADOW   0
#define YES_SHADOW  1
///@}


#ifndef _MENU_STRUCT
 /// @brief Пункт меню TAMIC (элемент массива пунктов для `bar_menu`).
 struct menu_items
          {
           /// Текст пункта.
           char *item;        /* pointer to item string*/
	   /// Номер символа строки, выбирающего пункт с клавиатуры.
	   int position;      /* position in string of active char*/
           /// Дополнительная клавиша выбора пункта (код клавиши).
           int add_key;       /* aditional active char key*/
           /// 1 — после пункта рисуется разделительная линия.
           int line;          /* line after item drow if 1*/
           /// 0 — меню остаётся на экране после выбора, 1 — меню закрывается.
           int exit_code;     /* 0 stay menu at screen,
                                 1 just close menu          */
           /// Номер страницы справки.
           int help_page;     /* help_page number*/
           };

 /// @brief Пара цветов элемента меню: в обычном состоянии и выбранного.
 struct alt_colors
           {
            /// Цвет в обычном состоянии.
            int Normal;
            /// Цвет выбранного элемента.
            int Select;
            };

 /// @brief Цвета меню TAMIC.
 struct color_menu_attrib
           {
            /// Цвета активных пунктов.
            struct alt_colors act_item;
            /// Цвета неактивных пунктов.
            struct alt_colors no_act_item;
            /// Цвета фона пунктов.
            struct alt_colors bg_item;
            /// Цвет рамки.
            int BorderColor;
            /// Цвет полосы выбора.
            int BarColor;
            };

 /// @brief Атрибуты меню TAMIC для `bar_menu`: заголовок, цвета, рамка, состояние.
 struct menu_attrib
           {
            /// Заголовок меню.
            char *titel;
            /// Цвет заголовка.
            int fgTitel;
            /// Цвета меню.
            struct color_menu_attrib *c_attr;
            /// Положение меню.
            int position;
            /// Код типа рамки.
            int border_type;   /* border type code*/
            /// Код состояния меню.
            int menu_code;
            /// Номер текущего пункта.
            int curr;
            /// Буфер изображения экрана.
            void *screen_buff;
            };

 /// @brief Прямоугольное окно в координатах экрана.
 struct window
          {
           /// Левая граница.
           int left;
           /// Верхняя граница.
           int top;
           /// Правая граница.
           int right;
           /// Нижняя граница.
           int bottom;
           };
 /// Краткое имя `struct window`.
 typedef struct window WINDOW;

/// @brief Пункт меню в окне сообщения (`messege_with_menu`).
struct messege_menu
            {
             /// Текст пункта.
             char *item;
             /// Символ-клавиша выбора пункта.
             char letter;
             /// Окно пункта на экране.
             struct window mw;
             };

/// Признак того, что структуры меню уже объявлены.
#define _MENU_STRUCT
#endif

/// @brief Масштаб символов: множители и делители по X и Y (по образцу `setusercharsize`).
struct set_char_size
                {
                 /// @name Масштаб по X
                 ///@{
                 /// Множитель и делитель по X.
                 int xm, xd;
                 ///@}
                 /// @name Масштаб по Y
                 ///@{
                 /// Множитель и делитель по Y.
                 int ym, yd;
                 ///@}
                 };
/// @brief Задаёт стиль выбора графического режима для `init_graph`.
/// @param[in] style  `MAX_COLOR_STYLE` или `HIGH_RES_STYLE`
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void _Cdecl set_init_style( int style );
/// @brief Включает графический режим TAMIC.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void _Cdecl init_graph( void);
/// @brief Показывает меню с полосой выбора и обрабатывает выбор пункта.
/// @param[in] col  столбец левого верхнего угла меню
/// @param[in] row  строка левого верхнего угла меню
/// @param[in] mi  массив пунктов
/// @param[in,out] mA  атрибуты и состояние меню
/// @return Результат работы меню (по имени — выбранный пункт).
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int bar_menu( int col, int row, struct menu_items *mi, struct menu_attrib *mA );
/// @brief Выводит сообщение из нескольких строк с меню выбора.
/// @param[in] messege  массив строк сообщения
/// @param[in] x_pos  положение по X
/// @param[in] y_pos  положение по Y
/// @param[in] msm  пункты меню
/// @param[in] BorderColor  цвет рамки
/// @param[in] TextColor  цвет текста
/// @param[in] BackColor  цвет фона
/// @param[in] border_flag  ненулевое — рисовать рамку
/// @return Результат выбора (по имени — выбранный пункт).
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  _Cdecl messege_with_menu( char **messege, int x_pos, int y_pos,
                            struct messege_menu *msm,
                            int BorderColor, int TextColor, int BackColor,
                            int border_flag);
/// @brief Выводит строки текста в прямоугольнике с рамкой.
/// @param[in] fgTextColor  цвет текста
/// @param[in] fgBorderColor  цвет рамки
/// @param[in] bgColor  цвет фона
/// @param[in] x_pos  положение по X
/// @param[in] y_pos  положение по Y
/// @param[in] pause_flag  ненулевое — пауза после вывода
/// @param[in] shadow_flag  `NO_SHADOW` или `YES_SHADOW`
/// @param[in] border_flag  ненулевое — рисовать рамку
/// @param[in] string  массив строк
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void _Cdecl text_in_box( int fgTextColor, int fgBorderColor, int bgColor,
                      int x_pos, int y_pos, int pause_flag, int shadow_flag,
                      int border_flag, char *string[]);

/// Наибольшее число нормирующих преобразований (размер массивов `norm_transf`).
#define MAX_TRANS_NUM        10

#ifndef _FLOAT_WINDOW
 /// Признак того, что структура `float_window` уже объявлена.
 #define _FLOAT_WINDOW

/// @brief Прямоугольник в вещественных координатах.
struct float_window
          {
           /// Левая граница.
           float left;
           /// Нижняя граница.
           float bottom;
           /// Правая граница.
           float right;
           /// Верхняя граница.
           float top;
           };

#endif

/// @brief Набор нормирующих преобразований: окна пользователя `fwn` отображаются в области вывода `fvp` и `vp`.
struct norm_transf
         {
          /// Номер действующего преобразования.
          int active_num;
          /// Области вывода в пикселях экрана.
          struct viewporttype vp[MAX_TRANS_NUM];
          /// Области вывода в вещественных координатах.
          struct float_window fvp[MAX_TRANS_NUM];
          /// Окна пользователя в вещественных координатах.
          struct float_window fwn[MAX_TRANS_NUM];
          /// Масштаб по X действующего преобразования.
          float x_wn_scale;
          /// Масштаб по Y действующего преобразования.
          float y_wn_scale;
          };

#ifndef _FLOAT_POINTTYPE
  /// Признак того, что структура `float_pointtype` уже объявлена.
  #define _FLOAT_POINTTYPE

/// @brief Точка в вещественных координатах (`_real`).
struct float_pointtype
	  {
	   /// Координата X.
	   _real x;
	   /// Координата Y.
	   _real y;
	   };

#endif



#ifndef _STRING_EDIT_STRUCT
  /// Признак того, что структуры редактора строки уже объявлены.
  #define _STRING_EDIT_STRUCT



/// @brief Строка, редактируемая в графическом окне (поле ввода окна-меню).
struct string_to_edit
         {
          /// @name Положение строки редактирования
          ///@{
          /// Положение строки на экране.
          int x_pos, y_pos; /* x,y string position*/
          ///@}
          /// Признак активной строки.
          int active_flag;  /* string active flag*/
          /// Буфер редактируемой строки.
          char *string;/* pointer to edit string buffer*/
          /// Наибольшая длина строки, байты.
          int max_s_len;    /* max string length in byte*/
          /// Длина строки, пиксели.
          int s_len;        /* string lengs in pixels*/
          /// Длина окна.
          int w_len;        /* window length*/
          /// Номер первого показываемого символа строки.
          int start_num;    /* start number of char to draw string*/
          /// Вид курсора: вставка, замена или без курсора.
          int cursor_flag;  /* INS, OVER, NO*/
          /// Регистр: 1 — верхний, 0 — нижний.
          int case_flag;    /* upper case = 1, lower case = 0 */
          };
/// Краткое имя `struct string_to_edit`.
typedef struct string_to_edit STRING_TO_EDIT;

/// @brief Строка текста для вывода в окне-меню.
struct string_to_draw
         {
          /// @name Положение выводимой строки
          ///@{
          /// Положение строки на экране.
          int x_pos, y_pos; /* x,y string position*/
          ///@}
          /// Выводимая строка.
          char *string;/* pointer to edit string*/
          };
/// Краткое имя `struct string_to_draw`.
typedef struct string_to_draw STRING_TO_DRAW;

#endif


#ifndef _ACTIVE_CHAR_STRUCT
  /// Признак того, что структура `active_char` уже объявлена.
  #define _ACTIVE_CHAR_STRUCT

/// @brief Специальная клавиша, обрабатываемая редактором строки и окном-меню.
struct active_char
         {
          /// Код клавиши.
          int ch;
          /// Код, сопоставленный клавише.
          int code;
          /// Флаг «клавиша + Enter»: 1 — включён.
          int flag;     /* key +enter if == 1*/
          };
/// Краткое имя `struct active_char`.
typedef struct active_char ACTIVE_CHAR;

#endif



#ifndef _WINDOW_MENU_STRUCT
  /// Признак того, что структуры окна-меню уже объявлены.
  #define _WINDOW_MENU_STRUCT



/// @brief Меню «точек»: набор окон-пунктов, из которых выбирается один.
struct _point_menu
         {
          /// Число активных точек.
          int num;          /* number of active points*/
          /// Окна точек.
          WINDOW *pw;  /* pointer to windows of point*/
          /// Номер активного пункта.
          int item_num;     /* active item number*/
          };
/// Краткое имя `struct _point_menu`.
typedef struct _point_menu POINT_MENU;

/// @brief Пункт-флажок окна-меню.
struct _cross_menu
         {
          /// Признак: 0 — не активен, 1 — активен.
          int flag;         /* active flag 0 not active,  1 active*/
          /// Окно пункта.
          WINDOW xw;        /* window of point*/
          };
/// Краткое имя `struct _cross_menu`.
typedef struct _cross_menu CROSS_MENU;


/// @brief Линия в окне-меню; координаты концов заданы парами чисел `m` и `d`.
struct window_line
	   {
	    /// @name Начало линии
	    ///@{
	    /// Начало линии: `x0m`, `x0d`, `y0m`, `y0d`.
	    int x0m, x0d, y0m, y0d; /* line coord*/
	    ///@}
	    /// @name Конец линии
	    ///@{
	    /// Конец линии: `x1m`, `x1d`, `y1m`, `y1d`.
	    int x1m, x1d, y1m, y1d; /* line coord*/
	    ///@}
	    /// Цвет линии.
	    int color;              /* color of line*/
	    };
/// Краткое имя `struct window_line`.
typedef struct window_line WINDOW_LINE;

/// @brief Описание окна-меню TAMIC для `window_menu`: окна пунктов, строки ввода, меню точек и флажков, линии.
struct window_menu_tabl
	   {
      /// @name Положение окна-меню
      ///@{
      /// Положение окна: центр верхнего края.
      int x_pos, y_pos;        /* senter upper position*/
      ///@}
      /// Код меню: -1 — закрыть открытое меню, 0 — открыть впервые.
      int menu_code;           /* menu code: -1 - close open menu, 0 - open first time*/
      /// Буферы изображения экрана.
      void **bit_map;     /* pointer to screen buffer*/
      /// Номер текущего пункта.
      int current;            /* current item number*/
      /// Общее число пунктов.
      int summ_number;         /* summary number of items*/
      /// Главное окно.
      WINDOW *l0_wndw;    /* pointer to main window*/
      /// Массив окон пунктов.
      WINDOW *l1_wndw;    /* pointer to window array*/
      /// Строки редактирования.
      STRING_TO_EDIT *ps; /* pointer to string egitor struct*/
      /// Меню точек.
      POINT_MENU *pm;     /* pointer to point menu structure*/
      /// Пункты-флажки.
      CROSS_MENU *px;     /* pointer to cross menu structures*/
      /// Первый набор специальных клавиш.
      ACTIVE_CHAR *ach;   /* first spetial character codes set*/
      /// Второй набор специальных клавиш.
      ACTIVE_CHAR *ach1;  /* second spetial character codes set*/
      /// Массив выводимых строк.
      STRING_TO_DRAW *ds; /* strings array pointer*/
	    /// Линии окна.
	    WINDOW_LINE *ln;    /* pointer to lines*/
      /// Функция пользователя для дорисовки окна.
      void (*drow_handler)( void ); /* pointer to user drow function*/
	    };
/// Краткое имя `struct window_menu_tabl`.
typedef struct window_menu_tabl WINDOW_MENU;

/// @brief Пункт окна-меню: вид, номер и код выхода (аргумент `window_menu`).
struct window_menu_items
           {
            /// Вид пункта: 1 — строка редактирования, 2 — меню точек, 3 — флажок.
            int item_code; /* menu item code: 1 - edit string, 2 - point menu, 3 - cross menu*/
            /// Номер пункта в окне.
            int item_num;  /* item number in window*/
            /// Относительный номер пункта в окне.
            int relativ_num;  /* relativ item number in window*/
            /// 0 — окно остаётся на экране, иное значение — окно закрывается.
            int exit_code; /* 0 - stay window resident, othe close*/
            };
/// Краткое имя `struct window_menu_items`.
typedef struct window_menu_items WINDOW_MENU_ITEM;

#endif

/// @brief Показывает окно-меню и обрабатывает ввод.
/// @param[in,out] wmt  описание окна-меню
/// @param[in,out] wmi  сведения о пункте
/// @return Код результата.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  window_menu( WINDOW_MENU *wmt, WINDOW_MENU_ITEM *wmi );
/// @brief Показывает меню с полосой выбора и обрабатывает выбор пункта (повторное объявление).
/// @param[in] col  столбец левого верхнего угла меню
/// @param[in] row  строка левого верхнего угла меню
/// @param[in] mi  массив пунктов
/// @param[in,out] mA  атрибуты и состояние меню
/// @return Результат работы меню (по имени — выбранный пункт).
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  bar_menu( int col, int row, struct menu_items *mi, struct menu_attrib *mA );


/* marker functions*/
                                    /* marker size */
/// @name Размеры маркера
///@{
/// Размер маркера (поле `MARKER::size`): от `_SMALL` (1) до `_LARGE` (4).
#define _SMALL              1
#define _LARGE              4
///@}

#ifndef _MARKER
  /// Признак того, что структура `MARKER` уже объявлена.
  #define _MARKER

/// @brief Вид маркера точки для `_set_marker`.
typedef struct
            {
             /// Тип маркера.
             int type;
             /// Ширина.
             int width;
             /// Цвет.
             int color;
             /// Размер: от `_SMALL` до `_LARGE`.
             int size;
             } MARKER;
#endif

/// @brief Рисует маркер в точке (x, y) с видом, заданным `set_marker_type`.
/// @param[in] x  координата X, пиксели
/// @param[in] y  координата Y, пиксели
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void  _set_marker( int x, int y );
/// @brief Задаёт вид маркера.
/// @param[in] mt  вид маркера
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void  set_marker_type( MARKER *mt );

/// @name Состояние кнопки
///@{
/// Аргумент `flag` функции `button`: `UNPRESSED` (1) — кнопка отжата, `PRESSED` (0) — нажата.
#define UNPRESSED  1
#define PRESSED    0

/// @brief Рисует кнопку с надписью.
/// @param[in] x  положение по X
/// @param[in] y  положение по Y
/// @param[in] str  надпись
/// @param[in] flag  `PRESSED` или `UNPRESSED`
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void button( int x, int y, char *str, int flag );

#endif

