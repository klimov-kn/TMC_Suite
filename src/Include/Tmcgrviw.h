/*
**  TAMIC_soft(R) 1999 v2.00
**
** Macros and decloration for  TAMIC graphics programm
**
** FILE: ..\include\tmc3dgr.h
**
*/

/// @file Tmcgrviw.h
/// @brief Общие константы и структуры вьюверов и ядер: типы графиков, метки файлов, теги блоков задания, коды узлов.
///
/// Заголовок наследия TAMIC, включается вьюверами (TMCGROUT, TMCROS, TMC_DN), ядрами
/// PlanarRT_H/X, библиотекой TMCIndan и FieldView. Содержит:
/// типы величин по осям графиков и метки строк файлов документов вьюверов;
/// метки заголовков выходных файлов ядер (`.t`, файлы полей, топология);
/// ключевые слова блоков задания `.tpl` и коды типов узлов сетки;
/// структуры документа и окна графика (`TMC_GR_DOC1`, `TMC_GR_DOC`, `TMC_GR_WINDOW`).
/// Значения констант записаны в файлы пользователей и в настройки программ — менять нельзя.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#tmcgrviw
#ifndef _TMC_GRPH_WIN_H
  #define _TMC_GRPH_WIN_H
#include <Typedef.h>
#include <TMCGrExpression.h>

			//type graphics - KSV
/// @name Величина по оси Y графика S-параметров (TMCGROUT)
/// Номер совпадает с номером переключателя в окне Viewport и записывается в файл
/// документа (`#nYType`).
///@{
/// Величина по оси Y TMCGROUT: 0 — КСВ (1+|S|)/(1-|S|), 1 — потери L = -20*lg|S|, дБ,
/// 2 — модуль |S|, 3 — фаза S, радианы, 4 — фаза S, градусы.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#graph-types
#define TMC_GR_TYPE_WSVR 0
			//type graphics - L db
#define TMC_GR_TYPE_L    1
			//type graphics - S modul
#define TMC_GR_TYPE_SM   2
			//type graphics - S phase in radian
#define TMC_GR_TYPE_SFR  3
			//type graphics - S phase in gradus
#define TMC_GR_TYPE_SFD  4

			//type graphics - radio impuls
///@}
/// @name Величина по оси Y графика сигналов (TMCROS, TMC_DN)
///@{
/// Величина по оси Y графика сигнала: 0 — сигнал целиком (радиоимпульс), 1 — амплитуда,
/// 2 — фаза, радианы, 3 — фаза, градусы, 4 — КСВ, 5 — L, дБ, 6 — разность фаз.
#define TMC_GROTS_TYPE_FULL  0
			//type graphics - only amplituda
#define TMC_GROTS_TYPE_AMPL  1
			//type graphics - only phase 
#define TMC_GROTS_TYPE_PHAS_R  2
			//type graphics - only phase 
#define TMC_GROTS_TYPE_PHAS_G  3
			//type graphics - only KSV
#define TMC_GROTS_TYPE_WSVR  4
			//type graphics - only L dB
#define TMC_GROTS_TYPE_LDB   5
			//type graphics - only delta phase
#define TMC_GROTS_TYPE_PHASD 6

			//type graphics - X-axies
///@}
/// @name Единицы оси X графика S-параметров (частота)
/// Номер совпадает с номером переключателя в окне Viewport и записывается в файл
/// документа (`#nXType`). Частоты в файлах `.s` — Гц.
///@{
/// Единицы оси X: 0 — ГГц, 1 — МГц, 2 — кГц, 3 — Гц, 4 — единицы пользователя
/// (частота в Гц умножается на `#dXAxiesUnit`, подпись — `#dXAxiesName`).
#define TMC_GR_TYPE_FGHz 0
#define TMC_GR_TYPE_FMHz 1
#define TMC_GR_TYPE_FKHz 2
#define TMC_GR_TYPE_FHz  3
#define TMC_GR_TYPE_F_USER  4


///@}
/// @name Ключи настроек программы вьюверов
/// Раздел и ключи настроек (на Windows — реестр, в Qt6-версии — файл настроек
/// программы). Под ключом `#TMCGROTS` раздела `#TMCGROTS` хранится строка параметров
/// по умолчанию, поля которой разделены байтом 0x01.
///@{
/// Имя раздела и ключа настроек программы: строка параметров графика по умолчанию,
/// формат подписей осей X и Y, размер маркеров точек, угол (наследие, не используется).
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#settings
#define TMC_GROTS_DOCFILE_ID				"#TMCGROTS"
#define TMC_GROTS_DOCFILE_XAXIESFORMAT		"#TMCGROTS_X_AXIES_FORMAT"
#define TMC_GROTS_DOCFILE_YAXIESFORMAT		"#TMCGROTS_Y_AXIES_FORMAT"
#define TMC_GROTS_DOCFILE_POINTSIZE			"#TMCGROTS_POINTSIZE"
			//type graphics - X-axies
///@}
/// @name Единицы оси X графика сигналов (время, TMCROS, TMC_DN)
///@{
/// Единицы оси времени: 0 — номер шага по времени, 1 — пс, 2 — нс, 3 — мкс, 4 — мс, 5 — с.
#define TMC_GROTS_TYPE_nT   0
#define TMC_GROTS_TYPE_s    5
#define TMC_GROTS_TYPE_ms   4
#define TMC_GROTS_TYPE_mks  3
#define TMC_GROTS_TYPE_ns   2
#define TMC_GROTS_TYPE_ps   1

///@}
/// @name Метки файла сигналов `.t`
/// Пишет ядро (PlanarRT_H/X), читает TMCROS.
///@{
/// Метка строки заголовка файла сигналов на портах `.t`: признак файла, частота, номер
/// входа, признак многочастотного расчёта, порт, частота сигнала.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#t-file
#define TMC_GROTS_FILEOUT_ID		"#TMC_GraphicsOutputSignalFile FormVer2.0 2000 "
#define TMC_GROTS_FLOUTFreq_ID		"#TMC_GROTS_Freq "
#define TMC_GROTS_FLOUTInpNum_ID	"#TMC_GROTS_InpNum "
#define TMC_GROTS_FLOUTWideband_ID	"#TMC_GROTS_Wideband "
#define TMC_GROTS_FLOUTPort_ID		"#TMC_GROTS_Port "
#define TMC_GROTS_FLOUTSigFreq_ID	"#TMC_GROTS_SigFreq "

///@}
/// Метка первой строки файла топологии сетки, который пишет ядро и читает FieldView.
#define TMC_RTH_FILETOP_ID		"#TMC_TopologyOutputFile FormVer2.0 2000"

/// @name Размер области прокрутки видов
///@{
/// Размер области прокрутки вида по X и Y в логических единицах (0.01 мм).
#define TMC_VIEW_XSIZE 18000
#define TMC_VIEW_YSIZE 18000
///@}
/// Режим координат видов: `MM_HIMETRIC` — логическая единица 0.01 мм, ось Y направлена вверх.
#define TMC_VIEW_SIZEMODE MM_HIMETRIC

/// Размер буфера строки и пути в документах вьюверов, байт (включая завершающий ноль).
#define TMC_GROUT_MAXSTRING_BUF 10000

/// Число вершин ломаной, которой рисуется маркер точки графика (`TMC_GR_DOC1::piPoint`).
#define TMC_GROUT_POINT_TYPE   10
/// @name Размер маркера точки
///@{
/// Ширина (X) и высота (Y) маркера точки, логические единицы (0.01 мм); умножаются на
/// размер маркера из окна Viewport.
#define TMC_GROUT_POINT_TYPE_X 400
#define TMC_GROUT_POINT_TYPE_Y 400

#define TMCFIELDVIEW_3DDraw 1

///@}
/// @name Метки строк файла документа TMCGROUT (`.soc`)
/// Строка документа — метка, пробел и значение. Метки `IDP`, `IDPar`, `nPointForNorm`
/// в коде пакета не используются; `INIWND` и `INIWNDVIEWPORT` — ключи настроек
/// программы (положение окна и размер области графика).
///@{
/// Метка строки файла документа TMCGROUT или ключ настроек; `#TMCGROUT` — первая строка
/// файла, по ней файл документа отличается от файла `.s`.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
#define TMC_GROUT_DOCFILE_ID				"#TMCGROUT"
#define TMC_GROUT_DOCFILE_ID_PointDrawFlag	"#PointDrawFlag"
#define TMC_GROUT_DOCFILE_ID_nXType			"#nXType"
#define TMC_GROUT_DOCFILE_ID_nYType			"#nYType"
#define TMC_GROUT_DOCFILE_ID_Xmin			"#Xmin"
#define TMC_GROUT_DOCFILE_ID_Xmax			"#Xmax"
#define TMC_GROUT_DOCFILE_ID_Ymin			"#Ymin"
#define TMC_GROUT_DOCFILE_ID_Ymax			"#Ymax"
#define TMC_GROUT_DOCFILE_ID_nAFlagX		"#nAFlagX"
#define TMC_GROUT_DOCFILE_ID_nAFlagY		"#nAFlagY"
#define TMC_GROUT_DOCFILE_ID_INIWND			"#INIWND"
#define TMC_GROUT_DOCFILE_ID_INIWNDVWPRT	"#INIWNDVIEWPORT"
#define TMC_GROUT_DOCFILE_ID_dXAxiesUnit	"#dXAxiesUnit"
#define TMC_GROUT_DOCFILE_ID_dXAxiesName	"#dXAxiesName"

///@}
/// @name Метки заголовка файла поля
/// Пишут ядра (TMCIndan), читает FieldView.
///@{
/// Метка строки заголовка файла поля: признак файла, номер шага, время, шаг сетки,
/// начало области по X и Y, точность, единицы длины и времени, число узлов по X и Y.
#define TMC_GROFLD_DOCFILE_ID				"#TMC_GraphicsOutputFieldFile FormVer2.0 2000 "
#define TMC_GROFLD_DOCFILE_ID_nT			"#TMCGROFLD_nT "
#define TMC_GROFLD_DOCFILE_ID_T				"#TMCGROFLD_Time "
#define TMC_GROFLD_DOCFILE_ID_Delta			"#TMCGROFLD_Delta "
#define TMC_GROFLD_DOCFILE_ID_Xmin			"#TMCGROFLD_Xmin "
#define TMC_GROFLD_DOCFILE_ID_Ymin			"#TMCGROFLD_Ymin "
#define TMC_GROFLD_DOCFILE_ID_Accuracy		"#TMCGROFLD_Accuracy "
#define TMC_GROFLD_DOCFILE_ID_LongUnit		"#TMCGROFLD_LongUnit "
#define TMC_GROFLD_DOCFILE_ID_TimeUnit		"#TMCGROFLD_TimeUnit "
#define TMC_GROFLD_DOCFILE_ID_nX			"#TMCGROFLD_nX "
#define TMC_GROFLD_DOCFILE_ID_nY			"#TMCGROFLD_nY "

#define TMC_TOPVIEW_DOCFILE_ID_EDITORFLNM		"#EditorFileName"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawTextFont	"#sDrawTextFont"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawTextColor	"#sDrawTextColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawPointColor	"#sDrawPointColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawAxiesColor	"#sDrawAxiesColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawGridColor	"#sDrawGridColor"
#define TMC_TOPVIEW_DOCFILE_ID_sDrawBckgrdColor	"#sDrawBckgrdColor"

///@}
/// @name Ключевые слова блоков задания `.tpl`
/// Ищутся в начале строки описания блока (с завершающим пробелом). `_N`, `_B`, `_Y`, `_S` —
/// блоки плазмы X-ядра: концентрация, магнитное поле, проводимость шлейфа, sigma (См/м).
///@{
/// Ключевое слово блока задания: геометрия (прямоугольник, круг, многоугольник,
/// отрезок `L`), неподвижный или движущийся блок, вход по X/Y, блок из файла.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#blocks
#define CTMCRTH_INDANBLCK_RECTSTAT	  "RECT_STAT "
#define CTMCRTH_INDANBLCK_CIRCSTAT	  "CIRCLE_STAT "
#define CTMCRTH_INDANBLCK_POLYGSTT	  "POLYGON_STAT "
#define CTMCRTH_INDANBLCK_RECTSTATN	  "RECT_STAT_N "
#define CTMCRTH_INDANBLCK_CIRCSTATN	  "CIRCLE_STAT_N "
#define CTMCRTH_INDANBLCK_POLYGSTTN	  "POLYGON_STAT_N "
#define CTMCRTH_INDANBLCK_RECTSTATB	  "RECT_STAT_B "
#define CTMCRTH_INDANBLCK_CIRCSTATB	  "CIRCLE_STAT_B "
#define CTMCRTH_INDANBLCK_POLYGSTTB	  "POLYGON_STAT_B "
#define CTMCRTH_INDANBLCK_RECTSTATY	  "RECT_STAT_Y "
#define CTMCRTH_INDANBLCK_CIRCSTATY	  "CIRCLE_STAT_Y "
#define CTMCRTH_INDANBLCK_POLYGSTTY	  "POLYGON_STAT_Y "
#define CTMCRTH_INDANBLCK_RECTSTATS	  "RECT_STAT_S "		// B-20: conductivity sigma, S/m (X kernel)
#define CTMCRTH_INDANBLCK_CIRCSTATS	  "CIRCLE_STAT_S "
#define CTMCRTH_INDANBLCK_POLYGSTTS	  "POLYGON_STAT_S "
#define CTMCRTH_INDANBLCK_POLGLINE	  "L "
#define CTMCRTH_INDANBLCK_RECTMOVE	  "RECT_MOVE "
#define CTMCRTH_INDANBLCK_CIRCMOVE	  "CIRCLE_MOVE "
#define CTMCRTH_INDANBLCK_POLYGMOV	  "POLYGON_MOVE "
#define CTMCRTH_INDANBLCK_INPUT_X	  "INPUT_X "
#define CTMCRTH_INDANBLCK_INPUT_Y	  "INPUT_Y "
#define CTMCRTH_INDANBLCK_FILE		  "FILE "
#define CTMCRTH_INDANBLCK_FILEN		  "FILE_N "
#define CTMCRTH_INDANBLCK_FILEB		  "FILE_B "
#define CTMCRTH_INDANBLCK_FILEY		  "FILE_Y "
///@}
/// @name Материал блока
/// При `_MODEL_RTE__` (в сборке не задан) слова METAL и MAGNETIC меняются местами.
///@{
/// Ключевое слово материала блока: металл или магнитная стенка.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#node-types
	#define CTMCRTH_INDANBLCK_TYPEMET	  "METAL"
	#define CTMCRTH_INDANBLCK_TYPEMAG	  "MAGNETIC"
///@}
/// Ключевое слово материала блока: поглотитель.
#define CTMCRTH_INDANBLCK_TYPEABS	  "ABSORBER"

/// @name Коды блоков сетки
/// Записываются в `nType` блоков и узлов ядер H и X.
///@{
/// Код блока: 0 — диэлектрик, 100 — поглотитель, 101 — металл, 102/103 — вход слева/справа,
/// 104/105 — вход сверху/снизу, 107 — магнитная стенка.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#node-types
#define CTMCRTH_BLCKNTYPE_EPS		   0
#define CTMCRTH_BLCKNTYPE_MAGNETIC	 107
#define CTMCRTH_BLCKNTYPE_METAL		 101
#define CTMCRTH_BLCKNTYPE_INPXLEFT	 102
#define CTMCRTH_BLCKNTYPE_INPXRIGHT	 103
#define CTMCRTH_BLCKNTYPE_INPYTOP	 104
#define CTMCRTH_BLCKNTYPE_INPYBOT	 105
#define CTMCRTH_BLCKNTYPE_ABSORBER	 100
///@}
/// @name Коды соседства узлов поглотителя и магнитной стенки
///@{
/// Код узла поглотителя или магнитной стенки: бит 0001 — сосед слева, 0010 — сверху,
/// 0100 — справа, 1000 — снизу; 16 — без соседей, 21..24 — внутренние углы.
/// @see https://www.tamic.ru/docs/code/kernel-h.html#absorber
#define CTMCRTH_BLCKNTYPE_ABSR_0000	  16
#define CTMCRTH_BLCKNTYPE_ABSR_0001	   1
#define CTMCRTH_BLCKNTYPE_ABSR_0010	   2
#define CTMCRTH_BLCKNTYPE_ABSR_0011	   3
#define CTMCRTH_BLCKNTYPE_ABSR_0100	   4
#define CTMCRTH_BLCKNTYPE_ABSR_0101	   5
#define CTMCRTH_BLCKNTYPE_ABSR_0110	   6
#define CTMCRTH_BLCKNTYPE_ABSR_0111	   7
#define CTMCRTH_BLCKNTYPE_ABSR_1000	   8
#define CTMCRTH_BLCKNTYPE_ABSR_1001	   9
#define CTMCRTH_BLCKNTYPE_ABSR_1010	  10
#define CTMCRTH_BLCKNTYPE_ABSR_1011	  11
#define CTMCRTH_BLCKNTYPE_ABSR_1100	  12
#define CTMCRTH_BLCKNTYPE_ABSR_1101	  13
#define CTMCRTH_BLCKNTYPE_ABSR_1110	  14
#define CTMCRTH_BLCKNTYPE_ABSR_1111	  15
#define CTMCRTH_BLCKNTYPE_ABSR_1111_1 21
#define CTMCRTH_BLCKNTYPE_ABSR_1111_2 22
#define CTMCRTH_BLCKNTYPE_ABSR_1111_3 23
#define CTMCRTH_BLCKNTYPE_ABSR_1111_4 24









				   // surface type for kz or xx
				   // input type 


/// @brief Точка графика в единицах осей (вещественные координаты).
typedef struct
	{ 
		 /// Координата X в единицах оси X (ГГц, МГц, ... или единицы пользователя).
		 double x;				
		 /// Координата Y в единицах оси Y (КСВ, дБ, |S|, радианы или градусы).
		 double y;
	} TMC_POINT;

/// @brief Точка в логических координатах вида (0.01 мм) или смещение вершины маркера.
typedef struct
	{ 
		 /// Координата X, логические единицы (0.01 мм).
		 int nX;
		 /// Координата Y, логические единицы (0.01 мм); в режиме `MM_HIMETRIC` точки области
		 /// графика имеют отрицательный Y.
		 int nY;
	} TMC_INT_POINT;

/// @brief Один график документа вьювера: источник данных, данные и стиль рисования.
///
/// Массив таких записей — `TMC_GR_DOC::psGraph`; создаёт и освобождает документ
/// (`DocFileNewData`/`DocFileDelData`). Ещё 16 записей (`CTMCGROUTDoc::sGrDoc`) хранят
/// только стиль линий по умолчанию: графику с номером i достаётся стиль i % 16.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#document
typedef struct
	{
	   /// Число точек (частот); 0 или меньше — данные не прочитаны.
	   int		 nPoint;	// number of point
	   /// Частота для массива во времени, Гц (используют TMCROS/TMC_DN; в TMCGROUT не заполняется).
	   double	 dFreq;		// frequence for time array
	   /// Частоты точек, Гц, длина `nPoint`; выделяет sfile95 (для выражения — документ).
	   double	*pFreq;		// frequence array
	   /// Комплексный элемент S в каждой точке, длина `nPoint`; NULL у кривой-выражения.
	   _complex *pSmatr;	// point for complex elem Smatrix 
	   /// Кривая-выражение; NULL — обычный график из файла. Владеет документ.
	   CTMCGrExpression *pcExpr;// pointer of expression class 
	   /// Номер входа 1 элемента S (с 1); после чтения — исправленный по файлу.
	   int		 nInp1;		// numer for input 1
	   /// Номер моды на входе 1 (с 1).
	   int		 nMod1;		// moda for input 1
	   /// Номер входа 2 элемента S (с 1).
	   int		 nInp2;		// numer for input 2
	   /// Номер моды на входе 2 (с 1).
	   int		 nMod2;		// moda for input 2
	   /// Полный путь к прочитанному файлу `.s`, буфер `TMC_GROUT_MAXSTRING_BUF` байт.
	   char		*szFileName;// s matrix FileName with all path
	   /// Имя из файла документа: путь к `.s` (относительно папки документа) или выражение `{...}`.
	   char		*szGrapName;// graphics	S FileName
	   /// Подпись в легенде; при ошибке чтения — текст `Error_...` с причиной.
	   char		*szGrapPodp;// graphics	Podp
	   /// Цвет линии, `COLORREF` (0x00BBGGRR).
	   int		 LineColor;	// line color	 how draw line
	   /// Тип линии — стиль пера GDI: `PS_SOLID`, `PS_DASH`, `PS_DOT`, `PS_DASHDOT`, `PS_DASHDOTDOT`.
	   int		 LineType;	// line type
	   /// Толщина линии, логические единицы (0.01 мм), по умолчанию 50; у не сплошных линий
	   /// при рисовании берётся 1.
	   int		 LineWidth;	// line width
       /// Вершины ломаной маркера точки относительно самой точки, 0.01 мм (форма своя у каждого
       /// из 16 стилей).
       TMC_INT_POINT   piPoint[TMC_GROUT_POINT_TYPE];// how draw point
	   /// Цвет маркера, `COLORREF`; у всех графиков общий (цвет точек документа).
	   int		 PointColor;// point color
	   /// Стиль пера маркера (`PS_SOLID`).
	   int		 PointType;	// point type
	   /// Толщина пера маркера, 0.01 мм.
	   int		 PointWidth;// point width
	   /// TRUE — график рисуется и участвует в автодиапазоне; FALSE — скрыт.
	   int		 OutFlag;	// Flag for output or not graphics on view
	   /// Время последней записи файла `.s` при прошлой проверке (для автообновления).
	   FILETIME	 LastWriteTime;	// last time when S-matrix file was written
	} TMC_GR_DOC1;

/// @brief Документ графиков вьювера: список графиков и параметры окна графика.
///
/// Поле `CTMCGROUTDoc::grdoc`; записывается в файл документа (`.soc`) и читается из него.
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#doc-file
typedef struct
	{
		/// Число графиков в `psGraph`.
		int			 nGraph; // number of grahics in this document
		/// Массив графиков длиной `nGraph`; владеет документ.
		TMC_GR_DOC1 *psGraph;// pointer for grahics array		  
		/// TRUE — диапазон оси X подбирается по данным автоматически; FALSE — берётся `Xmin..Xmax`.
		BOOL		 nAFlagX;	//if nAFlagX = 0  - X; if = 1 - X1  
		/// TRUE — диапазон оси Y подбирается по данным автоматически; FALSE — берётся `Ymin..Ymax`.
		BOOL		 nAFlagY;	//if nAFlagY = 0  - Y; if = 1 - Y1  
		/// TRUE — рисовать маркеры точек.
		BOOL 		 PointDrawFlag;	//draw or not point in graphics
		/// Величина по оси Y, `TMC_GR_TYPE_WSVR`..`TMC_GR_TYPE_SFD`.
		int			 nYType;		//graphics type	Y-coord
		/// Единицы оси X, `TMC_GR_TYPE_FGHz`..`TMC_GR_TYPE_F_USER`.
		int			 nXType;		//graphics type	X-coord
		/// Левая граница оси X в единицах оси.
		double		 Xmin;			//
		/// Правая граница оси X в единицах оси.
		double		 Xmax;			//
		/// Нижняя граница оси Y в единицах оси Y.
		double		 Ymin;			//
		/// Верхняя граница оси Y в единицах оси Y.
		double		 Ymax;			//
		/// Время последней записи файла документа при прошлой проверке (для автообновления).
		FILETIME	 LastWriteTime;	// last time when document file was written
	} TMC_GR_DOC;

/// @brief Подготовленный к рисованию график: точки в единицах осей и в координатах вида.
typedef struct
	{
	   /// Число точек (как у графика документа).
	   int nPoint;			// number of point
	   /// Число точек, попавших в диапазон оси X (по нему прореживаются маркеры).
	   int		 nPointDraw;// number of drawing point 
           /// Точки в единицах осей, длина `nPoint`; владеет вид.
           TMC_POINT       *pPoint;	//   _real point in graphics
           /// Точки в логических координатах вида (0.01 мм), длина `nPoint`; владеет вид.
           TMC_INT_POINT   *piPoint;// logical point in graphics
	} TMC_GR_VIEW;

/// @brief Прямоугольник области графика в логических координатах вида (0.01 мм).
typedef struct
	{ 
		 /// Левый край области графика (ось Y), 0.01 мм.
		 int nXmin;
		 /// Правый край области графика, 0.01 мм.
		 int nXmax;
		 /// Верхний край области графика по модулю, 0.01 мм (логическая координата Y = -nYmin).
		 int nYmin;
		 /// Нижний край области графика по модулю, 0.01 мм (логическая координата Y = -nYmax, ось X).
		 int nYmax;
	} TMC_GR_VIEWPORT;

/// @brief Диапазоны осей графика в единицах осей и признаки автодиапазона.
typedef struct
	{ 
		/// Левая граница оси X, единицы оси.
		double Xmin;
		/// Правая граница оси X, единицы оси.
		double Xmax;
		/// Нижняя граница оси Y, единицы оси.
		double Ymin;
		/// Верхняя граница оси Y, единицы оси.
		double Ymax;
//		double X1min;
//		double X1max;
//		double Y1min;
//		double Y1max;
		/// TRUE — диапазон X по данным (автодиапазон).
		BOOL	nAFlagX;	//if nAFlagX = 0  - X; if = 1 - X1  
		/// TRUE — диапазон Y по данным (автодиапазон).
		BOOL	nAFlagY;	//if nAFlagY = 0  - Y; if = 1 - Y1  
	} TMC_GR_WINDOW_D;

/// @brief Окно графика вида: диапазоны осей, область рисования, типы осей и подготовленные графики.
///
/// Поле `CTMCGROUTView::tmcgrwin`; заполняется из документа в `OnInitialUpdate` и
/// при каждом изменении параметров.
typedef struct
	{ 
		/// Диапазоны осей в единицах осей.
		TMC_GR_WINDOW_D   sGrWin;//structure _real window
		/// Область рисования в логических координатах вида.
		TMC_GR_VIEWPORT sGrView;//structure int viewport
		/// Величина по оси Y, `TMC_GR_TYPE_*`.
		int			 nYType;		//graphics type	Y-coord
		/// Единицы оси X, `TMC_GR_TYPE_F*`.
		int			 nXType;		//graphics type	X-coord
		/// Число графиков в `psGraph`.
		int 		 nGraph;		//number of graphics
		/// TRUE — рисовать маркеры точек.
		BOOL 		 PointDrawFlag;	//draw or not point in graphics
		/// Подготовленные графики, длина `nGraph`; владеет вид.
		TMC_GR_VIEW		 *psGraph;	//pointer for graphics array structure
	} TMC_GR_WINDOW;


/// @brief Величина оси Y и её подпись (таблица вида для подписи оси и строки состояния).
typedef struct
	{ 
		/// Величина, `TMC_GR_TYPE_*`; -1 — конец таблицы.
		int				 nType;		//graphics type
		/// Подпись оси, до 99 символов.
		char 	 pszYname[100];		//Y name of graphics
	} TMC_GR_TYPE_Y;

/// @brief Единицы оси X и их подпись (таблица вида; пятая строка — единицы пользователя).
typedef struct
	{ 
		/// Единицы, `TMC_GR_TYPE_F*`; -1 — конец таблицы.
		int				 nType;		//graphics type
		/// Подпись оси, до 99 символов.
		char 	 pszXname[100];		//Y name of graphics
	} TMC_GR_TYPE_X;
/// @brief Переводит путь к файлу в путь относительно рабочей папки.
/// @param[in,out] szFileName путь к файлу; заменяется относительным
/// @param[in]     szWorkDir  рабочая папка
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void MakeRelatPathFile( char *szFileName, char *szWorkDir );
/// @copydoc MakeRelatPathFile(char*, char*)
/// @note Вариант с папкой в `CString`.
void MakeRelatPathFile( char *szFileName, CString szWorkDir );
/// @brief Возвращает длину первого элемента пути вместе с разделителем.
/// @param[in] ch путь
/// @return Число символов до разделителя включительно; 0 — разделителя нет.
/// @note Определения в исходниках пакета нет (объявление TAMIC); одноимённый метод есть у `CTMCGROUTDoc`.
int SelectDirName( char *ch );
/// @brief Возвращает число папок в пути.
/// @param[in] ch путь
/// @return Число разделителей пути.
/// @note Определения в исходниках пакета нет (объявление TAMIC); одноимённый метод есть у `CTMCGROUTDoc`.
int CalcNDirInPathName( char  *ch);
/// @brief Переводит путь относительно рабочей папки в полный.
/// @param[in,out] szFileName относительный путь; заменяется полным
/// @param[in]     szWorkDir  рабочая папка
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
void MakeFullPathFile( char *szFileName, char *szWorkDir );
/// @copydoc MakeFullPathFile(char*, char*)
/// @note Вариант с папкой в `CString`.
void MakeFullPathFile( char *szFileName, CString szWorkDir );

#endif
