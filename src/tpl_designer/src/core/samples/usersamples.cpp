/// @file usersamples.cpp
/// @brief Реализация «Моих образцов»: папки с заданием и `sample.json` в Документах.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Описание открытых функций —
/// в `usersamples.h`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "usersamples.h"

#include "lang.h"
#include "tplreader.h"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>

#include <algorithm>

namespace tpl::samples {

namespace {

/// Имя файла с названием, описанием, ядром и временем сохранения образца.
const char *const kMeta = "sample.json";

/// Имя папки из названия: без символов, которых не любит файловая система.
/// @param[in] title  название образца
/// @return Название без `\ / : * ? " < > |` (заменены на `_`), не длиннее 60 символов;
///         пустое -> «образец».
QString folderName(const QString &title)
{
    QString s = title.trimmed();
    s.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|]")), QStringLiteral("_"));
    s = s.left(60).trimmed();
    return s.isEmpty() ? QStringLiteral("образец") : s;
}

} // namespace

QString userSamplesRoot()
{
    const QString env = qEnvironmentVariable("TPLD_USER_SAMPLES");
    if (!env.isEmpty())
        return env;
    const QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return QDir(docs).filePath(QStringLiteral("TPL Designer/Мои образцы"));
}

QVector<UserSample> userSamples()
{
    QVector<UserSample> out;
    const QDir root(userSamplesRoot());
    for (const QString &sub : root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QDir d(root.filePath(sub));
        const QStringList tpls = d.entryList({QStringLiteral("*.tpl"), QStringLiteral("*.TPL")},
                                             QDir::Files);
        if (tpls.isEmpty())
            continue;
        UserSample s;
        s.dir = d.absolutePath();
        s.tplPath = d.filePath(tpls.first());
        s.title = sub;
        QFile meta(d.filePath(QLatin1String(kMeta)));
        if (meta.open(QIODevice::ReadOnly)) {
            const QJsonObject o = QJsonDocument::fromJson(meta.readAll()).object();
            s.title = o.value(QStringLiteral("title")).toString(sub);
            s.summary = o.value(QStringLiteral("summary")).toString();
            s.kernel = o.value(QStringLiteral("kernel")).toString() == QLatin1String("X")
                           ? Kernel::X
                           : Kernel::H;
            s.created = QDateTime::fromString(o.value(QStringLiteral("created")).toString(),
                                              Qt::ISODate);
        }
        if (!s.created.isValid())
            s.created = QFileInfo(s.tplPath).lastModified();
        out << s;
    }
    std::sort(out.begin(), out.end(),
              [](const UserSample &a, const UserSample &b) { return a.created > b.created; });
    return out;
}

WriteResult saveUserSample(const QString &title, const QString &summary, const Project &p,
                           const QMap<QString, QByteArray> &companions, QString *dirOut)
{
    // Папка — по названию; если такая есть, добавляем номер, а не перезаписываем
    // чужой образец молча.
    const QDir root(userSamplesRoot());
    QString name = folderName(title);
    QString dir = root.filePath(name);
    for (int i = 2; QFileInfo::exists(dir); ++i)
        dir = root.filePath(QStringLiteral("%1 (%2)").arg(name).arg(i));

    WriteResult r = writeTask(dir, p, companions);
    if (!r.ok)
        return r;

    QJsonObject o;
    o.insert(QStringLiteral("title"), title.trimmed());
    o.insert(QStringLiteral("summary"), summary.trimmed());
    o.insert(QStringLiteral("kernel"), p.kernel == Kernel::X ? QStringLiteral("X") : QStringLiteral("H"));
    o.insert(QStringLiteral("created"), QDateTime::currentDateTime().toString(Qt::ISODate));
    QFile meta(QDir(dir).filePath(QLatin1String(kMeta)));
    if (!meta.open(QIODevice::WriteOnly)) {
        r.ok = false;
        r.error = tpl::tr("не удалось записать %1", "failed to write %1").arg(meta.fileName());
        return r;
    }
    meta.write(QJsonDocument(o).toJson());
    r.written << QLatin1String(kMeta);
    if (dirOut)
        *dirOut = dir;
    return r;
}

Build loadUserSample(const UserSample &s)
{
    Build b;
    const ReadResult r = readTplFile(s.tplPath);
    b.project = r.project;
    b.project.kernel = s.kernel == Kernel::X ? Kernel::X : r.project.kernel;
    for (const QString &w : r.warnings)
        b.notes << tpl::tr("При чтении: ", "While reading: ") + w;
    for (const QString &e : r.errors)
        b.notes << tpl::tr("Ошибка чтения: ", "Read error: ") + e;

    // Всё, кроме самого задания и его _step.h, — сопутствующие файлы (распределения .eps).
    const QDir d(s.dir);
    for (const QFileInfo &fi : d.entryInfoList(QDir::Files)) {
        const QString suf = fi.suffix().toLower();
        if (suf == QLatin1String("eps")) {
            QFile f(fi.absoluteFilePath());
            if (f.open(QIODevice::ReadOnly))
                b.companions.insert(fi.fileName(), f.readAll());
        }
    }
    b.theory << Row{tpl::tr("Папка", "Folder"), s.dir};
    b.theory << Row{tpl::tr("Шагов расчёта", "Calculation steps"), QString::number(b.project.stepCount())};
    b.theory << Row{tpl::tr("Фигур / входов", "Shapes / ports"), QStringLiteral("%1 / %2")
                                                           .arg(b.project.shapes.size())
                                                           .arg(b.project.ports.size())};
    if (s.created.isValid())
        b.theory << Row{tpl::tr("Сохранён", "Saved"), s.created.toString(QStringLiteral("dd.MM.yyyy hh:mm"))};
    return b;
}

bool removeUserSample(const UserSample &s)
{
    // Удаляем только то, что лежит внутри корня «Моих образцов»: ошибка в пути не
    // должна стоить пользователю чужой папки.
    const QString root = QDir(userSamplesRoot()).absolutePath();
    const QString dir = QDir(s.dir).absolutePath();
    if (!dir.startsWith(root + QLatin1Char('/')))
        return false;
    return QDir(dir).removeRecursively();
}

} // namespace tpl::samples
