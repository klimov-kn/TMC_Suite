// Чтение STEP через OpenCASCADE.
//
// Пишется под API, общий для OCCT 7.6 (пакеты Ubuntu 24.04) и 7.9 (своя сборка для
// MinGW на Windows): только классы, которые не менялись между ними.
/// @file stepio_occt.cpp
/// @brief Чтение STEP через OpenCASCADE (XCAF): обход сборки, имена, материалы, цвета, триангуляция.
///
/// Реализация stepio.h для сборки с OpenCASCADE (выбирается src/core/CMakeLists.txt).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#step-import
#include "stepio.h"
#include "lang.h"

#include <BRepBndLib.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>
#include <Interface_Static.hxx>
#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <Standard_Failure.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TDF_Label.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDocStd_Document.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>

#include <algorithm>
#include <cmath>

namespace tpl {

namespace {

/// @brief Читает имя метки XCAF (`TDataStd_Name`, UTF-16).
/// @param[in] l  метка.
/// @return Имя или пустая строка.
QString labelName(const TDF_Label &l)
{
    Handle(TDataStd_Name) n;
    if (!l.IsNull() && l.FindAttribute(TDataStd_Name::GetID(), n)) {
        const TCollection_ExtendedString &s = n->Get();
        // ExtendedString — UTF-16: русские имена деталей из Компаса так и приходят.
        return QString::fromUtf16(reinterpret_cast<const char16_t *>(s.ToExtString()),
                                  s.Length());
    }
    return QString();
}

/// Материал тела: в XCAF он висит деревом-ссылкой (MaterialRefGUID) на метку материала.
/// @param[in] tool  инструмент материалов документа.
/// @param[in] l     метка тела или ссылки.
/// @return Имя материала или пустая строка, если материал не задан.
QString labelMaterial(const Handle(XCAFDoc_MaterialTool) &tool, const TDF_Label &l)
{
    Handle(TDataStd_TreeNode) node;
    if (l.IsNull() || !l.FindAttribute(XCAFDoc::MaterialRefGUID(), node) || !node->HasFather())
        return QString();
    Handle(TCollection_HAsciiString) name, descr, densName, densType;
    Standard_Real density = 0.0;
    // Вызов через объект: в OCCT 7.6 метод обычный, в 7.9 — статический, и только
    // такая запись собирается с обеими.
    if (tool.IsNull()
        || !tool->GetMaterial(node->Father()->Label(), name, descr, density, densName, densType)
        || name.IsNull())
        return QString();
    return QString::fromUtf8(name->ToCString());
}

/// @brief Обход дерева сборки XCAF со сбором треугольников в `Mesh`.
///
/// Живёт на время одного вызова `readStepFile`.
struct Walker {
    /// Инструмент форм документа.
    Handle(XCAFDoc_ShapeTool) shapes;
    /// Инструмент цветов документа.
    Handle(XCAFDoc_ColorTool) colors;
    /// Инструмент материалов документа.
    Handle(XCAFDoc_MaterialTool) materials;
    /// Сетка-результат (принадлежит вызывающему).
    Mesh *mesh = nullptr;
    /// Допуск триангуляции, мм.
    double deflection = 0.1;
    /// @name Счётчики граней
    ///@{
    /// Число граней: всего (`faces`) и без триангуляции (`emptyFaces`).
    int faces = 0, emptyFaces = 0;

    ///@}
    /// @brief Ищет цвет метки (поверхности, затем общий).
    /// @param[in]  l  метка.
    /// @param[out] c  найденный цвет.
    /// @return true — цвет задан.
    bool colorOf(const TDF_Label &l, Quantity_Color *c) const
    {
        return colors->GetColor(l, XCAFDoc_ColorSurf, *c)
               || colors->GetColor(l, XCAFDoc_ColorGen, *c);
    }

    /// @brief Триангулирует форму и добавляет её грани в сетку отдельным телом.
    /// @param[in] shape     форма детали (уже перенесённая в положение сборки).
    /// @param[in] name      имя тела.
    /// @param[in] material  имя материала; пусто — берётся имя тела.
    /// @param[in] hasColor  true — цвет задан.
    /// @param[in] color     цвет тела.
    void addShape(const TopoDS_Shape &shape, const QString &name, const QString &material,
                  bool hasColor, const Quantity_Color &color)
    {
        if (shape.IsNull())
            return;
        BRepMesh_IncrementalMesh(shape, deflection, Standard_False, 0.3, Standard_True);
        MeshBody body;
        body.name = name;
        body.material = material.isEmpty() ? name : material;
        body.hasColor = hasColor;
        if (hasColor) {
            const auto ch = [](double v) { return std::uint32_t(std::clamp(v, 0.0, 1.0) * 255 + 0.5); };
            body.rgb = (ch(color.Red()) << 16) | (ch(color.Green()) << 8) | ch(color.Blue());
        }
        body.firstTri = int(mesh->tris.size());
        for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
            const TopoDS_Face &face = TopoDS::Face(ex.Current());
            TopLoc_Location loc;
            const Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
            ++faces;
            if (tri.IsNull()) {
                ++emptyFaces;
                continue;
            }
            const gp_Trsf tr = loc.Transformation();
            const int base = int(mesh->vertices.size());
            for (int i = 1; i <= tri->NbNodes(); ++i) {
                const gp_Pnt p = tri->Node(i).Transformed(tr);
                mesh->vertices << Vec3{p.X(), p.Y(), p.Z()};
            }
            for (int i = 1; i <= tri->NbTriangles(); ++i) {
                int a = 0, b = 0, c = 0;
                tri->Triangle(i).Get(a, b, c);
                mesh->tris << std::array<int, 3>{base + a - 1, base + b - 1, base + c - 1};
            }
        }
        body.triCount = int(mesh->tris.size()) - body.firstTri;
        if (body.triCount > 0)
            mesh->bodies << body;
    }

    /// Обход сборки: узел — либо сборка (идём в компоненты), либо деталь (режем на тела).
    /// @param[in] label              метка узла сборки.
    /// @param[in] parentLoc          положение родителя.
    /// @param[in] inherited          имя, унаследованное от родителя.
    /// @param[in] inheritedMat       материал, унаследованный от родителя.
    /// @param[in] inheritedHasColor  задан ли цвет у родителя.
    /// @param[in] inheritedColor     цвет родителя.
    void walk(const TDF_Label &label, const TopLoc_Location &parentLoc, const QString &inherited,
              const QString &inheritedMat, bool inheritedHasColor, const Quantity_Color &inheritedColor)
    {
        TDF_Label ref = label;
        TopLoc_Location loc = parentLoc;
        if (shapes->IsReference(label)) {
            shapes->GetReferredShape(label, ref);
            loc = parentLoc * shapes->GetLocation(label);
        }
        QString name = labelName(label);
        if (name.isEmpty() || name.startsWith(QLatin1Char('=')))
            name = labelName(ref);
        if (name.isEmpty())
            name = inherited;
        QString mat = labelMaterial(materials, label);
        if (mat.isEmpty())
            mat = labelMaterial(materials, ref);
        if (mat.isEmpty())
            mat = inheritedMat;
        Quantity_Color color = inheritedColor;
        bool hasColor = inheritedHasColor;
        if (colorOf(label, &color) || colorOf(ref, &color))
            hasColor = true;

        if (shapes->IsAssembly(ref)) {
            TDF_LabelSequence comps;
            XCAFDoc_ShapeTool::GetComponents(ref, comps);
            for (int i = 1; i <= comps.Length(); ++i)
                walk(comps.Value(i), loc, name, mat, hasColor, color);
            return;
        }
        TopoDS_Shape shape = XCAFDoc_ShapeTool::GetShape(ref);
        if (shape.IsNull())
            return;
        shape.Move(loc);
        addShape(shape, name.isEmpty() ? tpl::tr("тело", "body") : name, mat, hasColor, color);
    }
};

} // namespace

bool stepSupported() { return true; }
bool stepWriterAvailable() { return true; }

MeshReadResult readStepFile(const QString &path, double deflection)
{
    MeshReadResult r;
    r.fileUnitMm = 1.0;
    // OpenCASCADE печатает статистику перевода в stdout — окну она ни к чему, а
    // консольные утилиты засоряет. Убираем печать в поток один раз.
    static const bool quiet = [] {
        Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_PrinterOStream));
        return true;
    }();
    Q_UNUSED(quiet);
    try {
        // Единица результата — миллиметры, во что бы ни писал файл (метры, дюймы).
        Interface_Static::SetCVal("xstep.cascade.unit", "MM");

        Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
        Handle(TDocStd_Document) doc;
        app->NewDocument("MDTV-XCAF", doc);

        STEPCAFControl_Reader reader;
        reader.SetNameMode(Standard_True);
        reader.SetColorMode(Standard_True);
        reader.SetMatMode(Standard_True);
        const QByteArray local = path.toLocal8Bit();
        // ⚠️ OCCT открывает файл по пути в кодировке системы; русские папки на Windows
        // проходят через toLocal8Bit (CP1251), на Linux/macOS это UTF-8.
        if (reader.ReadFile(local.constData()) != IFSelect_RetDone) {
            r.error = tpl::tr("файл STEP не прочитан: он повреждён или это не STEP",
                              "STEP file not read: it is damaged or is not STEP");
            return r;
        }
        if (!reader.Transfer(doc)) {
            r.error = tpl::tr("в файле STEP не нашлось геометрии, которую можно перевести",
                              "no geometry that can be converted was found in the STEP file");
            return r;
        }

        Walker w;
        w.shapes = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
        w.colors = XCAFDoc_DocumentTool::ColorTool(doc->Main());
        w.materials = XCAFDoc_DocumentTool::MaterialTool(doc->Main());
        w.mesh = &r.mesh;

        TDF_LabelSequence roots;
        w.shapes->GetFreeShapes(roots);
        // Отклонение треугольников: тысячная доля габарита. Мельче — дольше, крупнее —
        // окружность в срезе станет заметно многоугольной.
        if (deflection <= 0.0) {
            Bnd_Box box;
            for (int i = 1; i <= roots.Length(); ++i)
                BRepBndLib::Add(XCAFDoc_ShapeTool::GetShape(roots.Value(i)), box);
            double x0, y0, z0, x1, y1, z1;
            if (!box.IsVoid()) {
                box.Get(x0, y0, z0, x1, y1, z1);
                const double diag = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)
                                              + (z1 - z0) * (z1 - z0));
                deflection = std::max(1e-4, diag * 1e-3);
            } else {
                deflection = 0.1;
            }
        }
        w.deflection = deflection;
        const Quantity_Color none;
        for (int i = 1; i <= roots.Length(); ++i)
            w.walk(roots.Value(i), TopLoc_Location(), QString(), QString(), false, none);

        if (w.emptyFaces > 0) {
            r.warnings << tpl::tr("%1 из %2 граней не удалось разбить на треугольники — "
                                  "в срезе на их месте будет разрыв",
                                  "%1 of %2 faces could not be triangulated: the slice will have "
                                  "a gap in their place")
                              .arg(w.emptyFaces).arg(w.faces);
        }
        if (r.mesh.isEmpty())
            r.error = tpl::tr("в файле STEP нет поверхностей: только кривые или точки",
                              "the STEP file has no surfaces: only curves or points");
    } catch (const Standard_Failure &f) {
        r.error = tpl::tr("OpenCASCADE не смог прочитать файл: %1",
                          "OpenCASCADE could not read the file: %1")
                      .arg(QString::fromUtf8(f.GetMessageString()));
    }
    return r;
}

} // namespace tpl
