// tmc_ocl.h - OpenCL loaded at run time (LoadLibrary), no link dependency.
//
// A machine without OpenCL.dll still starts the program: TmcOcl::Load()
// simply returns false and the caller stays on the CPU.
// Header-only; include in ONE translation unit per module with
// TMC_OCL_IMPLEMENT defined before the include.

/// @file TmcOcl.h
/// @brief Загрузка OpenCL.dll во время работы и перечень видеокарт с двойной точностью.
///
/// Часть модуля «Расчёт на видеокарте». Программа не линкуется с OpenCL: таблица
/// указателей на функции `TmcOcl` заполняется из системной OpenCL.dll при первом
/// обращении, поэтому ядра и окна запускаются и на компьютере без драйвера OpenCL
/// (тогда расчёт идёт на процессоре). Используется GPU-веткой ядер H и X
/// (`tmcgpuh.h`, `tmcgpux.h`) и окном проверки карты (`gui_common/compute`).
/// Требуемый уровень API — OpenCL 1.2, одна ветка кода для NVIDIA и AMD.
/// Все функции заголовка встроенные (`inline`), макрос `TMC_OCL_IMPLEMENT` в коде
/// не используется: заголовок можно включать в любое число единиц трансляции.
/// @see https://www.tamic.ru/docs/code/gpu.html#opencl-loader
/// @see https://www.tamic.ru/docs/theory/gpu.html#opencl

#ifndef _TMC_OCL_H
#define _TMC_OCL_H

/// Версия API для заголовков `cl/cl.h`: 120 — OpenCL 1.2 (функции более новых версий
/// недоступны, программа работает с любым драйвером уровня 1.2 и выше).
#define CL_TARGET_OPENCL_VERSION 120
/// Разрешает функции, объявленные в новых заголовках устаревшими, но входящие в
/// OpenCL 1.2 (`clCreateCommandQueue` и др.), — без предупреждений компилятора.
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <cl/cl.h>
#include <windows.h>
#include <string>
#include <vector>

/// Список всех функций OpenCL, которые берутся из OpenCL.dll (X-макрос).
///
/// Для каждого имени `f` вызывает переданный макрос `X(f)`: так одним списком
/// объявляются поля `TmcOcl` (`TMC_OCL_DECL`) и заполняются через `GetProcAddress`
/// (`TMC_OCL_GET`). Все функции входят в OpenCL 1.2; если хоть одной нет в
/// библиотеке, `TmcOcl::Load` считает OpenCL недоступным.
#define TMC_OCL_FUNCS(X) \
	X(clGetPlatformIDs) X(clGetPlatformInfo) X(clGetDeviceIDs) X(clGetDeviceInfo) \
	X(clCreateContext) X(clReleaseContext) X(clCreateCommandQueue) X(clReleaseCommandQueue) \
	X(clCreateBuffer) X(clReleaseMemObject) X(clEnqueueReadBuffer) X(clEnqueueWriteBuffer) \
	X(clEnqueueReadBufferRect) X(clEnqueueWriteBufferRect) X(clEnqueueFillBuffer) \
	X(clCreateProgramWithSource) X(clBuildProgram) X(clGetProgramBuildInfo) X(clReleaseProgram) \
	X(clCreateKernel) X(clReleaseKernel) X(clSetKernelArg) X(clEnqueueNDRangeKernel) \
	X(clFinish) X(clFlush) X(clWaitForEvents) X(clReleaseEvent) X(clGetEventProfilingInfo)


/// @brief Таблица функций OpenCL, загруженных из системной OpenCL.dll.
///
/// Каждое поле — указатель на одноимённую функцию OpenCL (тип берётся из `cl/cl.h`),
/// вызов выглядит как `ocl.clFinish( q )`. До успешного `Load` все указатели нулевые.
/// Объект создаётся владельцем сеанса работы с картой: по одному в `CTmcGpuH`,
/// `CTmcGpuX` и один общий на процесс в окне проверки карты. Библиотека не
/// выгружается до конца процесса (деструктора нет).
/// @note Потоки: `Load` не защищён от одновременного вызова; после загрузки указатели
/// только читаются, и пользоваться ими можно из любого потока.
/// @see https://www.tamic.ru/docs/code/gpu.html#opencl-loader
struct TmcOcl
{
/// Объявляет поле-указатель `f` с типом функции OpenCL `f` (служебный, только внутри `TmcOcl`).
#define TMC_OCL_DECL(f) decltype(&::f) f;
	TMC_OCL_FUNCS(TMC_OCL_DECL)
#undef TMC_OCL_DECL
	/// Описатель загруженной OpenCL.dll; `NULL` — библиотека не загружена.
	HMODULE hLib;

	/// @brief Обнуляет все указатели и описатель библиотеки (OpenCL ещё не загружен).
	TmcOcl() { memset( this, 0, sizeof(*this) ); }

	/// @brief Загружает системную OpenCL.dll и получает адреса всех функций списка `TMC_OCL_FUNCS`.
	///
	/// Библиотека ищется только в системном каталоге Windows (полный путь
	/// `<System32>\OpenCL.dll`), а не в текущей папке задания, — чтобы чужая DLL из
	/// папки с расчётом не была загружена. Повторный вызов после успеха ничего не делает.
	/// @return `true` — библиотека загружена и все функции найдены; `false` — нет файла
	///         или в нём нет хотя бы одной функции OpenCL 1.2 (библиотека при этом выгружается,
	///         `hLib` = `NULL`). Причину `false` метод не различает.
	bool Load()
	{
		if( hLib != NULL ) return true;
		{	// T-33: only the system OpenCL.dll (the full path), not a file from the current (task) folder
			char szDll[MAX_PATH + 32];
			UINT n = GetSystemDirectoryA( szDll, MAX_PATH );
			if( ( n == 0 )||( n >= MAX_PATH ) ) return false;
			strcat( szDll, "\\OpenCL.dll" );
			hLib = LoadLibraryA( szDll );
		}
		if( hLib == NULL ) return false;
// TMC_OCL_GET: адрес функции f из библиотеки; при неудаче - выгрузка и false (описание - перед TmcOcl)
#define TMC_OCL_GET(f) f = (decltype(&::f))GetProcAddress( hLib, #f ); if( f == NULL ) { FreeLibrary( hLib ); hLib = NULL; return false; }
		TMC_OCL_FUNCS(TMC_OCL_GET)
#undef TMC_OCL_GET
		return true;
	}
};

// one usable device (double precision supported)
/// @brief Описание одной видеокарты OpenCL, как его видит программа.
///
/// Заполняется `TmcOclListGpus` для каждой видеокарты всех платформ, в том числе для
/// карт без двойной точности (признак `bFP64`). Ядра и окно проверки отбирают из списка
/// карты с `bFP64` == `true` и нумеруют их с нуля — это номер карты в ключе `/GPU:n`
/// и в переменной `TMC_GPU` (= n + 1). Структура не владеет ресурсами OpenCL.
/// @see https://www.tamic.ru/docs/code/gpu.html#device-choice
struct TmcOclDevice
{
	/// Платформа OpenCL (драйвер производителя), которой принадлежит карта.
	cl_platform_id idPlatform;
	/// Устройство OpenCL; по нему создаются контекст и очередь команд.
	cl_device_id   idDevice;
	/// @name Текстовые сведения о карте
	///@{
	/// Текстовые сведения о карте без пробелов и нулей по краям: `sName` — имя карты,
	/// `sVendor` — производитель, `sDriver` — версия драйвера (CL_DRIVER_VERSION),
	/// `sVersion` — версия OpenCL устройства (CL_DEVICE_VERSION), `sPlatform` — имя
	/// платформы (например, «NVIDIA CUDA»).
	std::string    sName, sVendor, sDriver, sVersion, sPlatform;
	///@}
	/// @name Память карты
	///@{
	/// Память карты, байт: `nGlobalMem` — вся память; `nMaxAlloc` — наибольший размер
	/// одного буфера (обычно 1/4 памяти). 0 — драйвер не сообщил.
	cl_ulong       nGlobalMem, nMaxAlloc;
	///@}
	/// @name Вычислительные ресурсы
	///@{
	/// `nUnits` — число вычислительных блоков, `nClockMHz` — наибольшая частота, МГц.
	cl_uint        nUnits, nClockMHz;
	///@}
	/// `true` — карта умеет считать в `double` (расширение `cl_khr_fp64` или ненулевой
	/// CL_DEVICE_DOUBLE_FP_CONFIG). Только такие карты годятся для расчёта.
	bool           bFP64;
};

/// @brief Читает строковое свойство устройства OpenCL и обрезает пробелы и нули по краям.
///
/// @param[in] ocl   загруженная таблица функций OpenCL (`Load` выполнен успешно)
/// @param[in] d     устройство OpenCL
/// @param[in] what  код свойства (`CL_DEVICE_NAME`, `CL_DRIVER_VERSION` и т. п.)
/// @return Значение свойства; пустая строка — свойство не прочитано или пусто.
inline std::string TmcOclDevStr( TmcOcl &ocl, cl_device_id d, cl_device_info what )
{
	size_t n = 0;
	if( ocl.clGetDeviceInfo( d, what, 0, NULL, &n ) != CL_SUCCESS || n == 0 ) return "";
	std::string s( n, '\0' );
	ocl.clGetDeviceInfo( d, what, n, &s[0], NULL );
	while( !s.empty() && ( s.back() == '\0' || s.back() == ' ' ) ) s.pop_back();
	size_t k = s.find_first_not_of( ' ' );
	return ( k == std::string::npos ) ? "" : s.substr( k );
}

// all GPU devices of all platforms, in a stable order (platform, device)
/// @brief Перечисляет все видеокарты (CL_DEVICE_TYPE_GPU) всех платформ OpenCL.
///
/// Порядок устойчив: сначала по номеру платформы, затем по номеру устройства внутри
/// платформы, как их выдаёт драйвер. На этом порядке держится нумерация карт: ядро
/// (`CTmcGpuH::Begin`, `CTmcGpuX::Begin`) и окно проверки карты отбирают из списка
/// карты с двойной точностью и получают одинаковые номера. Процессоры и другие
/// устройства OpenCL в список не входят.
/// @param[in] ocl  загруженная таблица функций OpenCL (`Load` выполнен успешно)
/// @return Список карт, в том числе без двойной точности; пустой — платформ или карт нет.
/// @note Первый вызов в процессе может длиться до долей секунды (драйвер
/// инициализирует платформу).
/// @see https://www.tamic.ru/docs/code/gpu.html#device-choice
inline std::vector<TmcOclDevice> TmcOclListGpus( TmcOcl &ocl )
{
	std::vector<TmcOclDevice> v;
	cl_uint np = 0;
	if( ocl.clGetPlatformIDs( 0, NULL, &np ) != CL_SUCCESS || np == 0 ) return v;
	std::vector<cl_platform_id> pl( np );
	ocl.clGetPlatformIDs( np, &pl[0], NULL );
	for( cl_uint p = 0; p < np; p++ )
	{
		char szPl[256] = "";
		ocl.clGetPlatformInfo( pl[p], CL_PLATFORM_NAME, sizeof(szPl), szPl, NULL );
		cl_uint nd = 0;
		if( ocl.clGetDeviceIDs( pl[p], CL_DEVICE_TYPE_GPU, 0, NULL, &nd ) != CL_SUCCESS || nd == 0 ) continue;
		std::vector<cl_device_id> dv( nd );
		ocl.clGetDeviceIDs( pl[p], CL_DEVICE_TYPE_GPU, nd, &dv[0], NULL );
		for( cl_uint d = 0; d < nd; d++ )
		{
			TmcOclDevice x;
			x.idPlatform = pl[p];
			x.idDevice   = dv[d];
			x.sPlatform  = szPl;
			x.sName      = TmcOclDevStr( ocl, dv[d], CL_DEVICE_NAME );
			x.sVendor    = TmcOclDevStr( ocl, dv[d], CL_DEVICE_VENDOR );
			x.sDriver    = TmcOclDevStr( ocl, dv[d], CL_DRIVER_VERSION );
			x.sVersion   = TmcOclDevStr( ocl, dv[d], CL_DEVICE_VERSION );
			std::string sExt = TmcOclDevStr( ocl, dv[d], CL_DEVICE_EXTENSIONS );
			cl_device_fp_config fp = 0;
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(fp), &fp, NULL );
			x.bFP64 = ( sExt.find( "cl_khr_fp64" ) != std::string::npos ) || ( fp != 0 );
			x.nGlobalMem = 0; x.nMaxAlloc = 0; x.nUnits = 0; x.nClockMHz = 0;
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &x.nGlobalMem, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &x.nMaxAlloc, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &x.nUnits, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &x.nClockMHz, NULL );
			v.push_back( x );
		}
	}
	return v;
}

/// @brief Возвращает имя кода ошибки OpenCL для сообщений (например, «CL_OUT_OF_RESOURCES»).
///
/// @param[in] e  код, возвращённый функцией OpenCL
/// @return Статическая строка с именем кода; для кодов вне короткого списка — «CL_ERROR».
inline const char *TmcOclErr( cl_int e )
{
	switch( e )
	{
		case CL_SUCCESS: return "CL_SUCCESS";
		case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
		case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
		case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
		case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
		default: return "CL_ERROR";
	}
}

#endif
