#include <gemc/utilities/gemcUtilities.h>
#include <gemc/gui/gui.h>
#include <gemc/gboard/gboard.h>
#include <gemc/gqtbuttonswidget/gQtButtonsWidget.h>
#include "gemc_options.h"

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UImessenger.hh>
#include <G4UIparameter.hh>
#include <G4Electron.hh>
#include <G4ParticleTable.hh>
#include <G4VisExecutive.hh>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QTimer>
#ifdef GEMC_TEST_QTSVG
#include <QSvgRenderer>
#endif

#include <iostream>
#include <memory>

// Capture real UI command routing without starting physics, geometry, or external detector services.
class CommandProbe : public G4UImessenger {
public:
	CommandProbe() {
		vis = std::make_unique<G4UIdirectory>("/vis/");
		viewer = std::make_unique<G4UIdirectory>("/vis/viewer/");
		settings = std::make_unique<G4UIdirectory>("/vis/viewer/set/");
		run = std::make_unique<G4UIdirectory>("/run/");
		projection = std::make_unique<G4UIcommand>("/vis/viewer/set/projection", this);
		projection->SetParameter(new G4UIparameter("mode", 's', false));
		projection->SetParameter(new G4UIparameter("angle", 'd', false));
		beam = std::make_unique<G4UIcommand>("/run/beamOn", this);
		beam->SetParameter(new G4UIparameter("events", 'i', false));
	}
	void SetNewValue(G4UIcommand* command, G4String value) override {
		if (command == projection.get()) projectionValue = value;
		if (command == beam.get()) beamValue = value;
	}
	std::string projectionValue, beamValue;
private:
	std::unique_ptr<G4UIdirectory> vis, viewer, settings, run;
	std::unique_ptr<G4UIcommand> projection, beam;
};

int main(int argc, char* argv[]) {
	QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
	auto schema = gemc::defineOptions();
	schema.defineOption(GVariable("gui_test_source", "", "source tree for the GUI test"), "");
	schema.defineOption(GVariable("gui_snapshot", "", "optional screenshot path"), "");
	auto options = std::make_shared<GOptions>(argc, argv, schema);
	const auto numericLocale = QLocale().name();
	auto application = gemc::makeQtApplication(argc, argv, true, options.get());
	CommandProbe commands;
	G4VisExecutive visualization("quiet");
	G4Electron::ElectronDefinition();
	G4ParticleTable::GetParticleTable()->SetReadiness(true);
	auto routines = std::make_shared<const gdynamicdigitization::dRoutinesMap>();
	auto accumulator = makeAnalysisAccumulator(true);
	auto dispenser = std::make_shared<EventDispenser>(options, routines, accumulator);
	GemcGUI window(options, dispenser, nullptr, accumulator);
	window.show();
	application->processEvents();

	QJsonObject report;
	report["language"] = application->property("gemcGuiLanguage").toString();
	report["numeric_locale_unchanged"] = QLocale().name() == numericLocale;
	auto* runButton = window.findChild<QPushButton*>("gemcRun");
	auto* events = window.findChild<QLineEdit*>("gemcNumberOfEvents");
	auto* counter = window.findChild<QLabel*>("gemcEventNumber");
	if (!runButton || !events || !counter) {
		std::cerr << "Missing run controls: " << bool(runButton) << bool(events) << bool(counter) << '\n';
		return 1;
	}
	report["run"] = runButton->text();
	report["run_tooltip"] = runButton->toolTip();
	report["button_fits"] = runButton->width() >= runButton->sizeHint().width();
	events->setText("3");
	runButton->click();
	report["counter_first"] = counter->text();
	events->setText("2");
	runButton->click();
	report["counter_final"] = counter->text();
	report["beam_command"] = QString::fromStdString(commands.beamValue).trimmed();

	auto* projection = window.findChild<QComboBox*>("gemcProjection");
	auto* culling = window.findChild<QComboBox*>("gemcCulling");
	if (!projection || !culling) {
		std::cerr << "Missing display controls: " << bool(projection) << bool(culling) << '\n';
		return 1;
	}
	projection->setItemText(1, "opaque translated label");
	projection->setCurrentIndex(1);
	report["projection_command"] = QString::fromStdString(commands.projectionValue).trimmed();
	report["culling_density"] = culling->itemData(2).toString();

	auto* board = window.findChild<GBoard*>();
	QTimer::singleShot(0, [&]() {
		auto* dialog = qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
		report["dialog_found"] = dialog != nullptr;
		if (dialog) {
			report["dialog_title"] = dialog->windowTitle();
			report["dialog_filters"] = dialog->nameFilters().join(";;");
			dialog->reject();
		}
	});
	if (!board) { std::cerr << "Missing log board\n"; return 1; }
	if (!QMetaObject::invokeMethod(board, "saveLog", Qt::DirectConnection)) return 1;

	QFile svg(QString::fromStdString(options->getRequiredScalarString("gui_test_source")) +
	          "/gemc/gui/images/buttons/generator_1.svg");
	if (!svg.open(QIODevice::ReadOnly)) {
		std::cerr << "Cannot open test SVG: " << svg.fileName().toStdString() << '\n';
		return 1;
	}
	report["svg"] = gemc::translateSvgLabels(QString::fromUtf8(svg.readAll()));
	report["caption_fits"] = true;
#ifdef GEMC_TEST_QTSVG
	QString captionSvg = report["svg"].toString();
	captionSvg.replace("<text ", "<text id=\"guiCaption\" ");
	QSvgRenderer renderer(captionSvg.toUtf8());
	const auto bounds = renderer.boundsOnElement("guiCaption");
	report["caption_fits"] = renderer.isValid() && bounds.left() >= 0 && bounds.right() <= 48;
#endif
	const auto snapshot = options->getRequiredScalarString("gui_snapshot");
	if (!snapshot.empty()) window.grab().save(QString::fromStdString(snapshot));

	// The GUI session is about to die; subsequent Geant4 cleanup must not retain its cout destination.
	G4UImanager::GetUIpointer()->SetCoutDestination(nullptr);
	std::cout << "GUI_REPORT=" << QJsonDocument(report).toJson(QJsonDocument::Compact).constData() << '\n';
	return 0;
}
