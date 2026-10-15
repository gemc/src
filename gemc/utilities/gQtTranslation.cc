#include "gQtTranslation.h"

#include <gemc/glogging/gmessageCatalog.h>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QVariant>

#include <vector>

// Q_INIT_RESOURCE must resolve the generated resource symbol in the global namespace.
static void initializeGuiResources() { Q_INIT_RESOURCE(gemc_gui_translations); }

void gemc::installGuiTranslations(QCoreApplication& application, const GOptions* options) {
	initializeGuiResources();
	const auto language = GMessageCatalog::load(options)->language();
	if (auto* previous = application.findChild<QObject*>("gemcGuiTranslators", Qt::FindDirectChildrenOnly)) {
		delete previous; // QTranslator unregisters itself when destroyed.
	}
	auto* owner = new QObject(&application);
	owner->setObjectName("gemcGuiTranslators");
	application.setProperty("gemcGuiLanguage", QString::fromStdString(language));
	if (language == "en" || language.starts_with("en-")) return;

	auto* standard = new QTranslator(owner);
	const auto locale = QLocale(QString::fromStdString(language).replace('-', '_'));
	if (standard->load(locale, "qtbase", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
		application.installTranslator(standard);
	}

	const auto separator = language.find('-');
	const auto base = language.substr(0, separator);
	auto* embedded = new QTranslator(owner);
	if (embedded->load(QString::fromStdString(":/gemc/gui/" + base + ".qm"))) {
		application.installTranslator(embedded);
	}

	std::vector<std::string> variants{base};
	if (base != language) variants.push_back(language);
	const auto directories = GMessageCatalog::searchDirectories(options);
	for (const auto& variant : variants) {
		// Install low-priority files first: Qt checks the most recently installed translator first.
		for (auto directory = directories.rbegin(); directory != directories.rend(); ++directory) {
			for (const auto& path : {*directory / "gui" / (variant + ".qm"), *directory / (variant + ".qm")}) {
				std::error_code error;
				if (!std::filesystem::is_regular_file(path, error)) continue;
				auto* translator = new QTranslator(owner);
				if (translator->load(QString::fromStdString(path.string()))) {
					application.installTranslator(translator);
				} else {
					qWarning("Ignoring invalid GUI translation catalog: %s", path.string().c_str());
					delete translator;
				}
			}
		}
	}
}
