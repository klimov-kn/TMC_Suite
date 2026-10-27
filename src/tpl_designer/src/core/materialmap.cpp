/// @file materialmap.cpp
/// @brief Таблица распознавания материалов по имени и функция `guessMaterial`.
///
/// Часть импорта 3D в ядре TPL Designer; описание — в materialmap.h.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#mesh-import
#include "materialmap.h"
#include "lang.h"

#include <QHash>
#include <QRegularExpression>

namespace tpl {

namespace {

/// @brief Строка словаря материалов: шаблон имени и предлагаемый материал.
struct Entry {
    const char *pattern; ///< регулярное выражение по имени в нижнем регистре
    const char *what;   ///< по-русски
    const char *whatEn; ///< по-английски (tpl::tr — в месте использования)
    /// Материал задания, который предлагается для тела.
    Material material;
    /// Относительная проницаемость (для диэлектрика; у остальных 1).
    double eps;
    /// true — фон (воздух/вакуум): тело не вставляется.
    bool skip;
};

// Порядок важен: первое совпадение побеждает. Поэтому «поглотитель» раньше «abs»
// (ABS-пластик), «магнитная стенка» раньше «металл», а короткие имена — словами
// целиком (\b): «pla» не должно ловить «plasma», «cu» — «vacuum».
//
// Проницаемости — паспортные значения на СВЧ; человек правит их в таблице, если у его
// марки другое.
/// Словарь материалов в порядке проверки (первое совпадение побеждает).
const Entry kTable[] = {
    // Фон: вставлять нечего.
    {"\\b(air|vacuum|void)\\b|воздух|вакуум", "воздух / вакуум", "air / vacuum", Material::Dielectric, 1.0, true},

    // Поглотитель и магнитная стенка.
    {"absorb|поглот|eccosorb|\\bram\\b|радиопоглощ", "поглотитель", "absorber", Material::Absorber, 1.0, false},
    {"\\bpmc\\b|magnetic\\s*wall|магнитн\\w*\\s*стен", "магнитная стенка", "magnetic wall", Material::Magnetic, 1.0, false},

    // Диэлектрики — до металлов: «стеклотекстолит» не должен стать «сталью».
    {"\\bfr-?4\\b|стеклотекстолит|fib(er|re)\\s*glass|\\bg-?10\\b", "FR-4 (eps 4.4)", "FR-4 (eps 4.4)", Material::Dielectric, 4.4, false},
    {"ro-?4003|rogers\\s*4003", "Rogers RO4003C (eps 3.55)", "Rogers RO4003C (eps 3.55)", Material::Dielectric, 3.55, false},
    {"ro-?4350|rogers\\s*4350", "Rogers RO4350B (eps 3.66)", "Rogers RO4350B (eps 3.66)", Material::Dielectric, 3.66, false},
    {"5880|duroid", "RT/duroid 5880 (eps 2.2)", "RT/duroid 5880 (eps 2.2)", Material::Dielectric, 2.2, false},
    {"ptfe|teflon|тефлон|фторопласт|polytetrafluoroethylene", "фторопласт / PTFE (eps 2.1)", "PTFE / Teflon (eps 2.1)", Material::Dielectric, 2.1, false},
    {"alumina|al2o3|поликор|корунд|керамик|ceramic", "алюмооксидная керамика (eps 9.8)", "alumina ceramic (eps 9.8)", Material::Dielectric, 9.8, false},
    {"quartz|fused\\s*silica|кварц", "кварц (eps 3.8)", "quartz (eps 3.8)", Material::Dielectric, 3.8, false},
    {"pmma|plexi|acryl|оргстекл|perspex|lucite", "оргстекло (eps 2.6)", "acrylic glass (eps 2.6)", Material::Dielectric, 2.6, false},
    {"glass|стекл", "стекло (eps 6)", "glass (eps 6)", Material::Dielectric, 6.0, false},
    {"polyethylene|\\bhdpe\\b|\\bldpe\\b|\\bpe\\b|полиэтилен", "полиэтилен (eps 2.25)", "polyethylene (eps 2.25)", Material::Dielectric, 2.25, false},
    {"polystyrene|полистирол|rexolite|рексолит", "полистирол (eps 2.55)", "polystyrene (eps 2.55)", Material::Dielectric, 2.55, false},
    {"polypropylene|полипропилен", "полипропилен (eps 2.2)", "polypropylene (eps 2.2)", Material::Dielectric, 2.2, false},
    {"\\babs\\b", "ABS-пластик (eps 2.8)", "ABS plastic (eps 2.8)", Material::Dielectric, 2.8, false},
    {"\\bpla\\b", "PLA-пластик (eps 2.7)", "PLA plastic (eps 2.7)", Material::Dielectric, 2.7, false},
    {"nylon|polyamide|капрон|полиамид", "полиамид (eps 3.5)", "polyamide (eps 3.5)", Material::Dielectric, 3.5, false},
    {"silicon\\b|кремний", "кремний (eps 11.7)", "silicon (eps 11.7)", Material::Dielectric, 11.7, false},
    {"\\bwater\\b|\\bвода\\b", "вода (eps 80)", "water (eps 80)", Material::Dielectric, 80.0, false},
    {"rubber|резин", "резина (eps 3)", "rubber (eps 3)", Material::Dielectric, 3.0, false},
    {"dielectric|диэлектрик|substrate|подложк", "диэлектрик (eps уточните)", "dielectric (check eps)", Material::Dielectric, 2.0, false},

    // Проводники.
    {"\\bpec\\b|perfect\\s*(electric\\w*\\s*)?conductor|идеальн\\w*\\s*проводн", "идеальный проводник", "perfect conductor", Material::Metal, 1.0, false},
    {"copper|\\bcu\\b|медь|медн|\\bм[0-3]\\b", "медь", "copper", Material::Metal, 1.0, false},
    {"alumin|алюмин|\\bамг|\\bд16|\\b6061\\b|\\b7075\\b", "алюминий", "aluminium", Material::Metal, 1.0, false},
    {"steel|stainless|\\baisi\\b|\\b12х18н10т\\b|стал|нерж|\\bст\\s?[0-9]", "сталь", "steel", Material::Metal, 1.0, false},
    {"brass|латун|\\bл6[0-9]\\b", "латунь", "brass", Material::Metal, 1.0, false},
    {"bronze|бронз", "бронза", "bronze", Material::Metal, 1.0, false},
    {"silver|серебр|\\bag\\b", "серебро", "silver", Material::Metal, 1.0, false},
    {"gold|золот|\\bau\\b", "золото", "gold", Material::Metal, 1.0, false},
    {"titan|титан|\\bвт[0-9]", "титан", "titanium", Material::Metal, 1.0, false},
    {"tungsten|вольфрам", "вольфрам", "tungsten", Material::Metal, 1.0, false},
    {"nickel|никел|inconel|инконел", "никель", "nickel", Material::Metal, 1.0, false},
    {"iron|желез|чугун", "железо", "iron", Material::Metal, 1.0, false},
    {"zinc|цинк|\\btin\\b|олов|\\blead\\b|свин", "металл", "metal", Material::Metal, 1.0, false},
    {"metal|металл|conductor|проводник", "металл", "metal", Material::Metal, 1.0, false},
};

} // namespace

MaterialGuess guessMaterial(const QString &name)
{
    MaterialGuess g;
    const QString n = name.toLower();
    if (n.trimmed().isEmpty())
        return g;
    for (const Entry &e : kTable) {
        static QHash<const char *, QRegularExpression> cache;
        auto it = cache.find(e.pattern);
        if (it == cache.end())
            it = cache.insert(e.pattern,
                              QRegularExpression(QString::fromUtf8(e.pattern),
                                                 QRegularExpression::UseUnicodePropertiesOption));
        if (it->match(n).hasMatch()) {
            g.known = true;
            g.skip = e.skip;
            g.material = e.material;
            g.eps = e.eps;
            g.what = tpl::tr(e.what, e.whatEn);
            return g;
        }
    }
    return g;
}

} // namespace tpl
