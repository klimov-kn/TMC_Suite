// TmcRTH_IndanTopology.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcRTH_IndanTopology window

/// @file TmcRTH_IndanTopology.h
/// @brief Топология шага: список блоков из `#TOPOLOGY` и их размещение из `#LINK_LIST`.
///
/// Часть библиотеки TMCIndan. Класс `CTmcRTH_IndanTopology` — тонкая обёртка
/// над списком блоков `CTmcRTH_BlockList`: передаёт ему тексты блоков,
/// разбирает строки размещения `T n; X0; Y0;` и уточняет сторону входов.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#topology

#ifndef CTMCRTH_INDANTOPOLOGY_H__
#define CTMCRTH_INDANTOPOLOGY_H__

#include <TmcRTH_BolckList.h>
#include <TmcRTH_IndanParam.h>

/// @brief Топология шага #STEP: список блоков и их размещение в области.
///
/// Принадлежит `CTmcRTH_Indan` (поле `cTopology`). Не владеет параметрами:
/// хранит указатель на `CTmcRTH_IndanParam` владельца (единицы и границы
/// области нужны для перевода координат). Блоки задаются в локальных
/// координатах; строка `T n; X0; Y0;` переносит начало локальных координат
/// блока n в точку (X0, Y0) области.
/// @note Потоки: не потокобезопасен; заполняется в потоке чтения задания.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#topology
/// @see https://www.tamic.ru/docs/theory/method.html#grid
class CTmcRTH_IndanTopology //: public CWnd
{
// Construction
public:
	/// @brief Прочитана ли топология.
	/// @return Всегда TRUE: число блоков `GetBlockNumber()` не бывает отрицательным.
	BOOL IsDataRead( void );
	/// @brief Возвращает число блоков в списке.
	/// @return Число блоков (без завершающего пустого элемента списка).
	int GetBlockNumber( void );
	/// @brief Копирует указатель на параметры и весь список блоков (глубокая копия блоков).
	/// @param[in] cTopol источник
	/// @return `*this`.
	CTmcRTH_IndanTopology& operator=( CTmcRTH_IndanTopology& cTopol );
	/// @brief Возвращает указатель на параметры шага (не принадлежит объекту).
	/// @return Указатель, заданный `SetParam`; `NULL` — не задан.
	CTmcRTH_IndanParam * GetpParam( void );
	/// @brief Возвращает голову списка блоков.
	/// @return Указатель на первый элемент; принадлежит объекту.
	CTmcRTH_BlockList * GetBlockList( void );
	/// @brief Задаёт параметры шага (единицы, границы области).
	/// @param[in] pcParam1 параметры; объект ими не владеет
	void SetParam( CTmcRTH_IndanParam * pcParam1 );
	/// @brief Проверяет, что номера блоков в `#TOPOLOGY` не повторяются.
	/// @param[in,out] cError ошибка; при повторе — «Dublicated BLOCK n…»
	void IsCorrectly( CTmcLibError &cError );
	/// @brief Проверяет, что каждый блок размещён строкой `T` в `#LINK_LIST`.
	/// @param[in,out] cError ошибка; для неразмещённого блока — «BLOCK n is absent…»
	void IsCorrectlyLink( CTmcLibError &cError );
	/// @brief Разбирает строку `T n; X0; Y0;` секции `#LINK_LIST`.
	///
	/// Номер блока должен быть целым, координаты — выражения в единицах `LONG_UNIT`.
	/// Блоку задаётся точка привязки (X0, Y0) в м. Для входа `INPUT_X` по X0
	/// выбирается сторона: в пределах шага от `Xmin` или в левой половине области —
	/// левый вход, в пределах шага от `Xmax` или в правой половине — правый
	/// (`CTMCRTH_BLCKNTYPE_INPXRIGHT`). Для `INPUT_Y` так же по Y0: нижний
	/// (`CTMCRTH_BLCKNTYPE_INPYBOT`) или верхний вход.
	/// @param[in]     csCh   строка секции
	/// @param[in,out] cError ошибка; X0/Y0 вне области для входа — ошибка с подсказкой
	/// @see https://www.tamic.ru/docs/theory/ports.html#transformer
	void ReadLink( CString &csCh, CTmcLibError &cError );
	/// @brief Разбирает текст одного блока `BLOCK n; ... ` и добавляет его в конец списка.
	/// @param[in]     csCh   текст блока (см. `CTmcRTH_BlockList::Add`)
	/// @param[in,out] cError ошибка разбора
	void Read( CString &csCh, CTmcLibError &cError );
	/// @brief Очищает список блоков и забывает указатель на параметры.
	void DeleteData( void );
	/// @brief Создаёт пустую топологию без параметров.
	CTmcRTH_IndanTopology();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTmcRTH_IndanTopology)
	//}}AFX_VIRTUAL

// Implementation
public:
	/// @brief Дописывает папку задания и расширение `.eps` к именам файлов блоков `FILE*`.
	/// @param[in] csCurrentPath папка задания
	void AddPathForFile( CString &csCurrentPath );
	/// @brief Уничтожает объект (список блоков освобождается своим деструктором).
	virtual ~CTmcRTH_IndanTopology();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTmcRTH_IndanTopology)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
private:
	/// Параметры шага (не принадлежат объекту; `NULL` — не заданы).
	CTmcRTH_IndanParam * pcParam;
	/// Голова списка блоков.
	CTmcRTH_BlockList cBlockList;
	/// @brief Удаляет из строки пробелы, табуляции и переводы строк (на месте).
	/// @param[in,out] ch строка, завершённая нулём
	/// @return Всегда 0.
	int expr_del_Blanks2( char *ch );
	/// @brief Выделяет очередной параметр до `;` верхнего уровня (вне скобок `{}()[]`).
	/// @param[out]    csBuf  текст параметра без `;`
	/// @param[in,out] szBuf  строка с текущей позиции; временно изменяется и восстанавливается
	/// @param[in,out] cError ошибка; «Missing separator ;», если `;` нет
	/// @return Сдвиг позиции за `;`; -1 при ошибке.
	int Search_1Param( CString &csBuf, char *szBuf, CTmcLibError &cError );
};


#endif
/////////////////////////////////////////////////////////////////////////////
