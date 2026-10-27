/// @file sysmemory.cpp
/// @brief Запрос объёма памяти у Windows, macOS и Linux (`sysmem::query`).
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Единственный файл
/// `src/gui` с ветками по операционным системам.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-memory
#include "sysmemory.h"

#if defined(Q_OS_WIN) || defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#elif defined(__APPLE__)
#  include <mach/mach.h>
#  include <sys/sysctl.h>
#  include <sys/types.h>
#else
#  include <QFile>
#  include <QString>
#endif

namespace sysmem {

Info query()
{
    Info r;
#if defined(Q_OS_WIN) || defined(_WIN32)
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(ms);
    // Та же функция, по которой ядро решает, копить ли ДН по краям
    // (`SetBorderModeIfNeeded` ДН-сборки): цифры у нас и у него одни.
    if (GlobalMemoryStatusEx(&ms)) {
        r.ok = true;
        r.total = double(ms.ullTotalPhys);
        r.available = double(ms.ullAvailPhys);
    }
#elif defined(__APPLE__)
    uint64_t total = 0;
    size_t len = sizeof(total);
    if (sysctlbyname("hw.memsize", &total, &len, nullptr, 0) == 0 && total > 0) {
        vm_statistics64_data_t vm;
        mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
        vm_size_t page = 0;
        if (host_page_size(mach_host_self(), &page) == KERN_SUCCESS
            && host_statistics64(mach_host_self(), HOST_VM_INFO64,
                                 reinterpret_cast<host_info64_t>(&vm), &count) == KERN_SUCCESS) {
            r.ok = true;
            r.total = double(total);
            // Как «доступно» у Монитора системы: свободные, неактивные и
            // сбрасываемые страницы система отдаст расчёту без подкачки.
            r.available = double(vm.free_count + vm.inactive_count + vm.purgeable_count
                                 + vm.speculative_count) * double(page);
        }
    }
#else
    // Linux: MemAvailable — оценка ядра системы, сколько можно занять без подкачки.
    // MemFree для этого не годится: кэш страниц он считает занятым.
    QFile f(QStringLiteral("/proc/meminfo"));
    if (f.open(QIODevice::ReadOnly)) {
        double total = 0.0, avail = -1.0;
        const QList<QByteArray> lines = f.readAll().split('\n');
        for (const QByteArray &l : lines) {
            const QList<QByteArray> parts = l.simplified().split(' ');
            if (parts.size() < 2)
                continue;
            if (parts.at(0) == "MemTotal:")
                total = parts.at(1).toDouble() * 1024.0;
            else if (parts.at(0) == "MemAvailable:")
                avail = parts.at(1).toDouble() * 1024.0;
        }
        if (total > 0.0 && avail >= 0.0) {
            r.ok = true;
            r.total = total;
            r.available = avail;
        }
    }
#endif
    if (r.available > r.total)
        r.available = r.total;
    return r;
}

} // namespace sysmem
