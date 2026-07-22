#include "gAnalysisView.h"
#include "event/gEventDataCollection.h"

// qt
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>

// c++
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace {

bool check(bool condition, const std::string& message) {
	if (!condition) { std::cerr << "Analyzer runtime integration failed: " << message << '\n'; }
	return condition;
}

void mergeTrueSample(const std::shared_ptr<GOptions>& gopts,
	                 const std::shared_ptr<GAnalysisAccumulator>& accumulator,
	                 int run, const std::string& detector,
	                 const std::string& variable, double value) {
	auto true_data = GTrueInfoData::create(gopts);
	true_data->includeVariable(variable, value);
	GAnalysisShard shard;
	shard.recordTrueInformation(run, detector, *true_data);
	accumulator->merge(std::move(shard));
}

void mergeDigitizedSample(const std::shared_ptr<GOptions>& gopts,
	                      const std::shared_ptr<GAnalysisAccumulator>& accumulator,
	                      int run, const std::string& detector,
	                      const std::string& variable, int value) {
	auto digitized_data = GDigitizedData::create(gopts);
	digitized_data->includeVariable(variable, value);
	GAnalysisShard shard;
	shard.recordDigitized(run, detector, *digitized_data);
	accumulator->merge(std::move(shard));
}

} // namespace

int main(int argc, char* argv[]) {
	qputenv("QT_QPA_PLATFORM", "offscreen");
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());
	QApplication application(argc, argv);
	bool success = true;

	success &= check(makeAnalysisAccumulator(false) == nullptr,
	                 "batch mode allocated an Analyzer service");
	auto accumulator = makeAnalysisAccumulator(true);
	success &= check(accumulator != nullptr, "GUI mode did not allocate an Analyzer service");

	accumulator->beginBeamOn(false);
	mergeTrueSample(gopts, accumulator, 17, "initial_plugin", "runtime_energy", 1.0);
	mergeDigitizedSample(gopts, accumulator, 17, "initial_plugin", "runtime_adc", 10);

	GAnalysisView view(accumulator);
	view.resize(900, 650);
	view.show();
	application.processEvents();

	auto* run_combo = view.findChild<QComboBox*>("analyzerRun");
	auto* detector_combo = view.findChild<QComboBox*>("analyzerDetector");
	auto* true_source = view.findChild<QRadioButton*>("analyzerTrueSource");
	auto* digitized_source = view.findChild<QRadioButton*>("analyzerDigitizedSource");
	auto* x_combo = view.findChild<QComboBox*>("analyzerXVariable");
	auto* accumulate_button = view.findChild<QPushButton*>("analyzerAccumulate");
	success &= check(run_combo != nullptr && run_combo->findData(17) >= 0,
	                 "initial run was not discovered");
	success &= check(detector_combo != nullptr && detector_combo->findData("initial_plugin") >= 0,
	                 "initial plugin was not discovered");
	success &= check(true_source != nullptr && true_source->isChecked() &&
	                 x_combo != nullptr && x_combo->findData("runtime_energy") >= 0,
	                 "initial runtime true variable was not discovered");
	if (x_combo != nullptr) { x_combo->setCurrentIndex(x_combo->findData("runtime_energy")); }

	if (accumulate_button != nullptr) { accumulate_button->click(); }
	accumulator->beginBeamOn(view.accumulateEnabled());
	mergeTrueSample(gopts, accumulator, 17, "initial_plugin", "runtime_energy", 2.0);
	view.refresh();
	application.processEvents();
	success &= check(view.histogramChart()->histogram().in_range_entries == 2,
	                 "repeated beamOn did not append to the selected histogram");

	accumulator->beginBeamOn(view.accumulateEnabled());
	mergeDigitizedSample(gopts, accumulator, 22, "clas12_dynamic_plugin", "runtime_signal", 25);
	view.refresh();
	application.processEvents();
	success &= check(run_combo != nullptr && run_combo->findData(17) >= 0 && run_combo->findData(22) >= 0,
	                 "accumulation did not preserve independent run-number keys");
	if (run_combo != nullptr) { run_combo->setCurrentIndex(run_combo->findData(22)); }
	application.processEvents();
	success &= check(detector_combo != nullptr &&
	                 detector_combo->findData("clas12_dynamic_plugin") >= 0,
	                 "schema-free dynamic plugin name was not discovered");
	if (detector_combo != nullptr) {
		detector_combo->setCurrentIndex(detector_combo->findData("clas12_dynamic_plugin"));
	}
	if (digitized_source != nullptr) { digitized_source->click(); }
	application.processEvents();
	success &= check(x_combo != nullptr && x_combo->findData("runtime_signal") >= 0,
	                 "dynamic plugin variable was not discovered after a later run");
	if (x_combo != nullptr) {
		x_combo->setCurrentIndex(x_combo->findData("runtime_signal"));
	}
	application.processEvents();
	success &= check(view.histogramChart()->histogram().in_range_entries == 1,
	                 "dynamic plugin histogram was not rendered");

	view.clearForGeometryReload();
	application.processEvents();
	success &= check(accumulator->snapshot().empty(), "geometry reload did not clear persisted samples");
	success &= check(run_combo != nullptr && run_combo->count() == 0,
	                 "geometry reload left stale run selectors visible");
	success &= check(view.histogramChart()->histogram().in_range_entries == 0,
	                 "geometry reload left a stale histogram visible");
	success &= check(view.accumulateEnabled(), "geometry reload changed the user's Accumulate selection");

	accumulator->beginBeamOn(view.accumulateEnabled());
	mergeDigitizedSample(gopts, accumulator, 23, "clas12_reloaded_plugin", "reloaded_signal", 30);
	view.refresh();
	application.processEvents();
	success &= check(run_combo != nullptr && run_combo->count() == 1 && run_combo->findData(23) == 0 &&
	                 run_combo->findData(22) < 0,
	                 "post-reload discovery retained stale run data");
	success &= check(detector_combo != nullptr &&
	                 detector_combo->findData("clas12_reloaded_plugin") >= 0,
	                 "post-reload dynamic plugin was not discovered");
	success &= check(x_combo != nullptr && x_combo->findData("reloaded_signal") >= 0,
	                 "post-reload runtime variable was not discovered");

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
