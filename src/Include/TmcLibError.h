// TmcLibError.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTmcLibError window

/// @file TmcLibError.h
/// @brief Объект ошибки `CTmcLibError`: флаг «была ошибка» и текст сообщения.
///
/// Библиотека TMCLibError. Объект возвращается или передаётся по ссылке функциями
/// ядер и библиотеки TMCIndan вместо кодов возврата: вызывающий проверяет `IsError()`
/// и показывает `GetErrorMessage()` в строке состояния или окне сообщения.
/// @see https://www.tamic.ru/docs/code/lib-tmcliberror.html
#ifndef CTMCLIBERROR_H__
#define CTMCLIBERROR_H__

#include <cstringt.h>

/// Текст «ошибки нет»: значение сообщения после создания объекта и после `Clear()`.
#define CTMCRTHERROR_0000 "Error:no"
/// @name Заготовки текстов ошибок
/// Все равны `"Error:"`; в коде пакета не используются.
/// @{
/** Заготовка текста ошибки, равна `"Error:"`; в коде пакета не используется. */
/// @}
/// Размер буфера сообщения, символов; используется только в отключённом
/// варианте `PutErrorMessage` с переменным числом аргументов.
#define CTMCRTHERROR_STRLENMAX 512

/// @brief Ошибка, накопленная при выполнении операции: флаг и текст.
///
/// Создаётся вызывающим (обычно на стеке или полем класса ядра) в состоянии «ошибки нет».
/// Вызываемая функция при сбое вызывает `PutErrorMessage`; повторный вызов заменяет
/// текст предыдущей ошибки. Объект не потокобезопасен: каждый поток использует свой.
/// @see https://www.tamic.ru/docs/code/lib-tmcliberror.html#usage
class CTmcLibError
{
public:
	/// @brief Фиксирует ошибку с текстом из C-строки.
	/// @param[in] pszErrorMessage  текст сообщения (копируется)
	void PutErrorMessage( const char *pszErrorMessage );
	/// @brief Фиксирует ошибку с текстом из `CString`.
	/// @param[in] csErrorMessage  текст сообщения (копируется)
	void PutErrorMessage( CString csErrorMessage );
	/// @brief Возвращает текст последней ошибки.
	/// @return Текст; `CTMCRTHERROR_0000` («Error:no»), если ошибки не было.
	CString GetErrorMessage( void );
	/// @brief Сообщает, была ли зафиксирована ошибка.
	/// @return TRUE — ошибка была (после последнего `Clear()`); FALSE — нет.
	BOOL IsError( void );
	/// @brief Сбрасывает объект в состояние «ошибки нет».
	void Clear( void );
	/// @brief Копирует флаг и текст ошибки из другого объекта.
	/// @param[in] cError1  источник
	/// @return Ссылка на этот объект.
	CTmcLibError& operator=(CTmcLibError& cError1);
	/// @brief Создаёт объект в состоянии «ошибки нет».
	CTmcLibError();

public:
	/// @brief Уничтожает объект (ресурсов, кроме строки, не держит).
	virtual ~CTmcLibError();

private:
	/// Текст последней ошибки.
	CString csError;
	/// TRUE — ошибка зафиксирована.
	BOOL bIsError;
};


#endif
/////////////////////////////////////////////////////////////////////////////
