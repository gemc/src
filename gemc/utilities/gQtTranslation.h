#pragma once

#include <QCoreApplication>

class GOptions;

namespace gemc {
/**
 * \brief Install GUI translations before constructing widgets, using the same language as GLogger.
 *
 * The application owns the translators. Regional catalogs override the base language; embedded catalogs
 * provide relocation-safe defaults. Missing entries retain English. Qt's standard-dialog catalogs are loaded
 * from the Qt installation when available. Calling again replaces the translators; existing widgets require
 * reconstruction to update their static labels. Numeric locale settings are left unchanged.
 */
void installGuiTranslations(QCoreApplication& application, const GOptions* options = nullptr);
}
