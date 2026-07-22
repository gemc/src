#include "gAnalysisView.h"
#include "gHistogram.h"
#include "event/gEventDataCollection.h"

// qt
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QtCharts/QLogValueAxis>

// c++
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool check(bool condition, const std::string& message) {
	if (!condition) { std::cerr << "Analyzer histogram validation failed: " << message << '\n'; }
	return condition;
}

} // namespace

int main(int argc, char* argv[]) {
	qputenv("QT_QPA_PLATFORM", "offscreen");
	auto option_definitions = gevent_data::defineOptions();
	option_definitions += ganalysis::defineOptions();
	auto gopts = std::make_shared<GOptions>(argc, argv, option_definitions);
	QApplication application(argc, argv);
	bool success = true;
	const auto default_analysis_options = ganalysis::getOptions(gopts);
	success &= check(default_analysis_options.plot_count == 1 &&
	                 !default_analysis_options.accumulate &&
	                 !default_analysis_options.plots[0].has_value(),
	                 "default ganalysis options unexpectedly prepared a plot");

	const std::vector<double> values{0.0, 0.25, 0.5, 0.75, 1.0,
	                                 std::numeric_limits<double>::quiet_NaN()};
	const auto automatic = makeHistogram(values, 4);
	success &= check(automatic.counts == std::vector<std::uint64_t>({1, 1, 1, 2}),
	                 "automatic range assigned samples to incorrect bins");
	success &= check(automatic.finite_entries == 5 && automatic.in_range_entries == 5,
	                 "non-finite values affected histogram entry counts");
	success &= check(automatic.x_min == 0.0 && automatic.x_max == 1.0,
	                 "automatic range did not preserve finite sample limits");

	const auto restricted = makeHistogram(values, 2, 0.25, 0.75);
	success &= check(restricted.counts == std::vector<std::uint64_t>({1, 2}),
	                 "explicit range assigned samples to incorrect bins");
	success &= check(restricted.underflow == 1 && restricted.overflow == 1,
	                 "explicit range did not count underflow and overflow");
	const auto automatic_max = makeHistogram(values, 2, 0.25, std::nullopt);
	success &= check(automatic_max.x_min == 0.25 && automatic_max.x_max == 1.0,
	                 "automatic x maximum was not derived independently");
	success &= check(automatic_max.underflow == 1 && automatic_max.overflow == 0,
	                 "fixed x minimum with automatic maximum counted excluded samples incorrectly");
	const auto automatic_min = makeHistogram(values, 2, std::nullopt, 0.75);
	success &= check(automatic_min.x_min == 0.0 && automatic_min.x_max == 0.75,
	                 "automatic x minimum was not derived independently");
	success &= check(automatic_min.underflow == 0 && automatic_min.overflow == 1,
	                 "automatic x minimum with fixed maximum counted excluded samples incorrectly");

	const auto constant = makeHistogram(std::vector<double>{7.0, 7.0}, 10);
	success &= check(constant.x_min < 7.0 && constant.x_max > 7.0,
	                 "constant-valued input did not receive a usable automatic range");

	bool rejected_invalid_range = false;
	try { (void) makeHistogram(values, 10, 1.0, 1.0); }
	catch (const std::invalid_argument&) { rejected_invalid_range = true; }
	success &= check(rejected_invalid_range, "invalid explicit x range was accepted");
	const auto histogram_2d = makeHistogram2D({{0.0, 1.0}, {0.5, 1.5}, {1.0, 2.0}}, 2);
	success &= check(histogram_2d.in_range_entries == 3 && histogram_2d.maximumCount() == 2,
	                 "2D histogram did not bin paired samples correctly");

	auto accumulator = std::make_shared<GAnalysisAccumulator>();
	accumulator->beginBeamOn(false);
	GAnalysisShard shard;
	auto true_data = GTrueInfoData::create(gopts);
	true_data->includeVariable("energy", 2.0);
	true_data->includeVariable("momentum", 20.0);
	shard.recordTrueInformation(17, "dynamic_detector", *true_data);
	true_data->includeVariable("energy", 3.0);
	true_data->includeVariable("momentum", 30.0);
	shard.recordTrueInformation(17, "dynamic_detector", *true_data);
	auto digitized_data = GDigitizedData::create(gopts);
	digitized_data->includeVariable("custom_adc", 42);
	shard.recordDigitized(17, "dynamic_detector", *digitized_data);
	accumulator->merge(std::move(shard));

	GAnalysisView view(accumulator);
	view.resize(900, 650);
	view.show();
	application.processEvents();
	auto* run_combo = view.findChild<QComboBox*>("analyzerRun");
	auto* detector_combo = view.findChild<QComboBox*>("analyzerDetector");
	auto* true_source = view.findChild<QRadioButton*>("analyzerTrueSource");
	auto* digitized_source = view.findChild<QRadioButton*>("analyzerDigitizedSource");
	auto* x_combo = view.findChild<QComboBox*>("analyzerXVariable");
	auto* y_combo = view.findChild<QComboBox*>("analyzerYVariable");
	auto* automatic_x_min = view.findChild<QCheckBox*>("analyzerAutomaticXMin");
	auto* automatic_x_max = view.findChild<QCheckBox*>("analyzerAutomaticXMax");
	auto* automatic_y_min = view.findChild<QCheckBox*>("analyzerAutomaticYMin");
	auto* automatic_y_max = view.findChild<QCheckBox*>("analyzerAutomaticYMax");
	auto* x_min_spin = view.findChild<QDoubleSpinBox*>("analyzerXMin");
	auto* x_max_spin = view.findChild<QDoubleSpinBox*>("analyzerXMax");
	success &= check(run_combo != nullptr && run_combo->findData(17) >= 0,
	                 "runtime run selector was not populated");
	success &= check(detector_combo != nullptr && detector_combo->findData("dynamic_detector") >= 0,
	                 "runtime plugin selector was not populated");
	success &= check(true_source != nullptr && true_source->isChecked() &&
	                 x_combo != nullptr && x_combo->findData("energy") >= 0,
	                 "runtime true-variable selector was not populated");
	if (digitized_source != nullptr) { digitized_source->click(); }
	success &= check(x_combo != nullptr && x_combo->findData("custom_adc") >= 0 &&
	                 y_combo != nullptr && y_combo->findData("custom_adc") >= 0,
	                 "shared source choice did not repopulate both variable selectors");
	if (true_source != nullptr) { true_source->click(); }
	success &= check(automatic_x_min != nullptr && automatic_x_min->isChecked() &&
	                 automatic_x_max != nullptr && automatic_x_max->isChecked() &&
	                 automatic_y_min != nullptr && automatic_y_min->isChecked() &&
	                 automatic_y_max != nullptr && automatic_y_max->isChecked(),
	                 "axis endpoints did not receive independent automatic controls");
	if (automatic_x_min != nullptr) { automatic_x_min->setChecked(false); }
	success &= check(x_min_spin != nullptr && x_min_spin->isEnabled() &&
	                 x_max_spin != nullptr && !x_max_spin->isEnabled(),
	                 "changing automatic x minimum also changed the x maximum control");
	if (x_combo != nullptr) { x_combo->setCurrentIndex(x_combo->findData("energy")); }
	application.processEvents();
	success &= check(view.histogramChart()->histogram().in_range_entries == 2,
	                 "selected runtime series was not plotted");

	auto* four_plots_button = view.findChild<QPushButton*>("analyzerFourPlots");
	auto* configuration_tabs = view.findChild<QTabWidget*>("analyzerConfigurationTabs");
	success &= check(four_plots_button != nullptr && !view.fourPlotMode(),
	                 "Analyzer did not start in single-plot mode");
	success &= check(configuration_tabs != nullptr && configuration_tabs->count() == 4 &&
	                 configuration_tabs->tabText(0) == "Top Left" &&
	                 configuration_tabs->tabText(1) == "Top Right" &&
	                 configuration_tabs->tabText(2) == "Bottom Left" &&
	                 configuration_tabs->tabText(3) == "Bottom Right",
	                 "2x2 configuration tabs were not named by grid position");
	if (four_plots_button != nullptr) { four_plots_button->click(); }
	application.processEvents();
	success &= check(view.fourPlotMode() && view.histogramChart(1)->isVisible() &&
	                 configuration_tabs != nullptr && configuration_tabs->isTabVisible(3),
	                 "4 plots button did not display the 2x2 plot grid");
	auto* second_configuration = view.findChild<QWidget*>("analyzerPlot2");
	success &= check(second_configuration != nullptr &&
	                 second_configuration->findChild<QWidget*>("analyzerChart2") == nullptr,
	                 "2x2 configuration tab still contains a chart widget");

	auto* second_digitized_source = view.findChild<QRadioButton*>("analyzerDigitizedSource2");
	auto* second_x_combo = view.findChild<QComboBox*>("analyzerXVariable2");
	auto* first_bins = view.findChild<QSpinBox*>("analyzerBins");
	auto* second_bins = view.findChild<QSpinBox*>("analyzerBins2");
	if (second_digitized_source != nullptr) { second_digitized_source->click(); }
	success &= check(second_x_combo != nullptr && second_x_combo->findData("custom_adc") >= 0,
	                 "second plot did not receive independent runtime selectors");
	if (second_x_combo != nullptr) {
		second_x_combo->setCurrentIndex(second_x_combo->findData("custom_adc"));
	}
	if (second_bins != nullptr) { second_bins->setValue(7); }
	application.processEvents();
	success &= check(view.histogramChart(1) != nullptr &&
	                 view.histogramChart(1)->histogram().in_range_entries == 1,
	                 "second plot did not render its independently selected variable");
	success &= check(first_bins != nullptr && second_bins != nullptr &&
	                 first_bins->value() == 100 && second_bins->value() == 7,
	                 "changing second-plot binning modified the first plot");
	success &= check(view.histogramChart() != view.histogramChart(1),
	                 "2x2 mode reused one chart instead of independent plot widgets");
	if (four_plots_button != nullptr) {
		four_plots_button->click();
		application.processEvents();
		four_plots_button->click();
		application.processEvents();
	}
	success &= check(view.fourPlotMode() && view.histogramChart(1)->isVisible() &&
	                 second_bins != nullptr && second_bins->value() == 7 &&
	                 second_x_combo != nullptr &&
	                 second_x_combo->currentData().toString() == "custom_adc",
	                 "plot settings were lost while switching between one and four plots");

	auto* plot_2d = view.findChild<QRadioButton*>("analyzerPlot2D");
	auto* style_combo = view.findChild<QComboBox*>("analyzerStyle");
	auto* scale_combo = view.findChild<QComboBox*>("analyzerScale");
	if (plot_2d != nullptr) { plot_2d->click(); }
	if (x_combo != nullptr) { x_combo->setCurrentIndex(x_combo->findData("energy")); }
	if (y_combo != nullptr) { y_combo->setCurrentIndex(y_combo->findData("momentum")); }
	if (style_combo != nullptr) { style_combo->setCurrentIndex(style_combo->findText("Boxes")); }
	if (scale_combo != nullptr) { scale_combo->setCurrentIndex(scale_combo->findText("Logarithmic")); }
	application.processEvents();
	success &= check(plot_2d != nullptr && plot_2d->isChecked() && style_combo != nullptr &&
	                 style_combo->isEnabled() && view.histogramChart()->histogram2D().in_range_entries == 2,
	                 "2D selection did not render paired Y-vs-X data with a selectable style");
	const auto two_dimensional_y_axes = view.histogramChart()->chart()->axes(Qt::Vertical);
	success &= check(two_dimensional_y_axes.size() == 1 &&
	                 dynamic_cast<QLogValueAxis*>(two_dimensional_y_axes.front()) == nullptr,
	                 "2D logarithmic selection incorrectly changed the Y axis instead of the Z scale");
	QTemporaryDir two_dimensional_directory;
	const QString two_dimensional_pdf = two_dimensional_directory.filePath("analyzer-2d.pdf");
	success &= check(view.exportPdf(two_dimensional_pdf), "2D Qt chart PDF export failed");
	auto* plot_1d = view.findChild<QRadioButton*>("analyzerPlot1D");
	if (plot_1d != nullptr) { plot_1d->click(); }
	application.processEvents();

	auto* accumulate_button = view.findChild<QPushButton*>("analyzerAccumulate");
	success &= check(accumulate_button != nullptr && !view.accumulateEnabled(),
	                 "Accumulate was not disabled by default");
	if (accumulate_button != nullptr) { accumulate_button->click(); }
	success &= check(view.accumulateEnabled(), "Accumulate button did not enable persistence");
	accumulator->beginBeamOn(view.accumulateEnabled());
	GAnalysisShard accumulated_shard;
	true_data->includeVariable("energy", 4.0);
	accumulated_shard.recordTrueInformation(17, "dynamic_detector", *true_data);
	accumulator->merge(std::move(accumulated_shard));
	view.refresh();
	application.processEvents();
	success &= check(view.histogramChart()->histogram().in_range_entries == 3,
	                 "enabled Accumulate did not append samples across beamOn intervals");

	if (accumulate_button != nullptr) { accumulate_button->click(); }
	accumulator->beginBeamOn(view.accumulateEnabled());
	GAnalysisShard replacement_shard;
	true_data->includeVariable("energy", 5.0);
	replacement_shard.recordTrueInformation(17, "dynamic_detector", *true_data);
	accumulator->merge(std::move(replacement_shard));
	view.refresh();
	application.processEvents();
	success &= check(!view.accumulateEnabled() &&
	                 view.histogramChart()->histogram().in_range_entries == 1,
	                 "disabled Accumulate did not replace samples at the next beamOn interval");

	GHistogramChart logarithmic_chart;
	logarithmic_chart.setHistogram(automatic, "Logarithmic validation", std::nullopt, std::nullopt, true);
	const auto vertical_axes = logarithmic_chart.chart()->axes(Qt::Vertical);
	success &= check(vertical_axes.size() == 1 && dynamic_cast<QLogValueAxis*>(vertical_axes.front()) != nullptr,
	                 "logarithmic selection did not create a Qt logarithmic axis");

	QTemporaryDir temporary_directory;
	const QString pdf_filename = temporary_directory.filePath("analyzer.pdf");
	success &= check(view.exportPdf(pdf_filename), "Qt chart PDF export failed");
	QFile pdf(pdf_filename);
	success &= check(pdf.open(QIODevice::ReadOnly) && pdf.read(4) == "%PDF",
	                 "exported file is not a PDF document");

	gopts->setOptionValueFromString("ganalysis_plots", "4");
	gopts->setOptionValueFromString("ganalysis_accumulate", "true");
	gopts->setOptionValueFromString(
		"ganalysis",
		"[{position: top_left, run: 44, plugin: prepared_plugin, source: true, dimension: 2d, "
		"x: expected_x, y: expected_y, bins: 12, x_min_auto: false, x_min: -5, "
		"x_max_auto: true, y_min_auto: false, y_min: -10, y_max_auto: false, y_max: 10, "
		"title: 'Prepared plot', style: boxes, scale: logarithmic}, "
		"{position: top_right, run: 44, plugin: prepared_plugin, x: another_x}]");
	const auto prepared_options = ganalysis::getOptions(gopts);
	success &= check(prepared_options.plot_count == 4 && prepared_options.accumulate &&
	                 prepared_options.plots[0].has_value() &&
	                 prepared_options.plots[0]->x_variable == "expected_x" &&
	                 prepared_options.plots[0]->y_variable == "expected_y",
	                 "ganalysis options were not projected into four-plot startup settings");

	auto prepared_accumulator = std::make_shared<GAnalysisAccumulator>();
	GAnalysisView prepared_view(prepared_accumulator, prepared_options);
	prepared_view.resize(1000, 800);
	prepared_view.show();
	application.processEvents();
	auto* prepared_x = prepared_view.findChild<QComboBox*>("analyzerXVariable");
	auto* prepared_y = prepared_view.findChild<QComboBox*>("analyzerYVariable");
	auto* prepared_bins = prepared_view.findChild<QSpinBox*>("analyzerBins");
	auto* prepared_x_auto = prepared_view.findChild<QCheckBox*>("analyzerAutomaticXMin");
	auto* prepared_x_max_auto = prepared_view.findChild<QCheckBox*>("analyzerAutomaticXMax");
	auto* prepared_y_auto = prepared_view.findChild<QCheckBox*>("analyzerAutomaticYMin");
	auto* prepared_y_max_auto = prepared_view.findChild<QCheckBox*>("analyzerAutomaticYMax");
	auto* prepared_x_min = prepared_view.findChild<QDoubleSpinBox*>("analyzerXMin");
	auto* prepared_y_min = prepared_view.findChild<QDoubleSpinBox*>("analyzerYMin");
	auto* prepared_y_max = prepared_view.findChild<QDoubleSpinBox*>("analyzerYMax");
	auto* prepared_title = prepared_view.findChild<QLineEdit*>("analyzerTitle");
	auto* prepared_2d = prepared_view.findChild<QRadioButton*>("analyzerPlot2D");
	auto* prepared_true = prepared_view.findChild<QRadioButton*>("analyzerTrueSource");
	auto* prepared_style = prepared_view.findChild<QComboBox*>("analyzerStyle");
	auto* prepared_scale = prepared_view.findChild<QComboBox*>("analyzerScale");
	success &= check(prepared_view.fourPlotMode() && prepared_view.accumulateEnabled() &&
	                 prepared_bins != nullptr && prepared_bins->value() == 12 &&
	                 prepared_x_auto != nullptr && !prepared_x_auto->isChecked() &&
	                 prepared_x_max_auto != nullptr && prepared_x_max_auto->isChecked() &&
	                 prepared_y_auto != nullptr && !prepared_y_auto->isChecked() &&
	                 prepared_y_max_auto != nullptr && !prepared_y_max_auto->isChecked() &&
	                 prepared_x_min != nullptr && prepared_x_min->value() == -5.0 &&
	                 prepared_y_min != nullptr && prepared_y_min->value() == -10.0 &&
	                 prepared_y_max != nullptr && prepared_y_max->value() == 10.0 &&
	                 prepared_title != nullptr && prepared_title->text() == "Prepared plot",
	                 "configured layout, accumulation, binning, limits, or title were not applied");
	success &= check(prepared_2d != nullptr && prepared_2d->isChecked() &&
	                 prepared_true != nullptr && prepared_true->isChecked() &&
	                 prepared_style != nullptr && prepared_style->currentText() == "Boxes" &&
	                 prepared_scale != nullptr && prepared_scale->currentText() == "Logarithmic",
	                 "configured dimension, source, style, or scale was not applied");
	success &= check(prepared_x != nullptr && !prepared_x->isEditable() &&
	                 prepared_x->currentData().toString() == "expected_x" &&
	                 prepared_x->itemData(prepared_x->currentIndex(), Qt::ForegroundRole).value<QColor>() ==
	                     QColor(Qt::red) &&
	                 prepared_y != nullptr && !prepared_y->isEditable() &&
	                 prepared_y->currentData().toString() == "expected_y" &&
	                 prepared_y->itemData(prepared_y->currentIndex(), Qt::ForegroundRole).value<QColor>() ==
	                     QColor(Qt::red),
	                 "configured variable names were not read-only and red before validation");
	const QSize cell_size = prepared_view.histogramChart(0)->size();
	success &= check(cell_size == prepared_view.histogramChart(1)->size() &&
	                 cell_size == prepared_view.histogramChart(2)->size() &&
	                 cell_size == prepared_view.histogramChart(3)->size(),
	                 "2x2 chart cells did not receive exactly equal dimensions");

	prepared_accumulator->beginBeamOn(false);
	GAnalysisShard unavailable_shard;
	auto unavailable_data = GTrueInfoData::create(gopts);
	unavailable_data->includeVariable("different_variable", 1.0);
	unavailable_shard.recordTrueInformation(44, "prepared_plugin", *unavailable_data);
	prepared_accumulator->merge(std::move(unavailable_shard));
	prepared_view.refresh();
	application.processEvents();
	success &= check(prepared_x != nullptr && prepared_x->currentText().contains("Not available") &&
	                 prepared_view.histogramChart()->histogram2D().in_range_entries == 0,
	                 "missing configured variable was not marked unavailable after the first run");

	prepared_accumulator->beginBeamOn(true);
	GAnalysisShard available_shard;
	auto available_data = GTrueInfoData::create(gopts);
	available_data->includeVariable("expected_x", 1.0);
	available_data->includeVariable("expected_y", 2.0);
	available_shard.recordTrueInformation(44, "prepared_plugin", *available_data);
	prepared_accumulator->merge(std::move(available_shard));
	prepared_view.refresh();
	application.processEvents();
	success &= check(prepared_x != nullptr && prepared_x->currentText() == "expected_x" &&
	                 prepared_x->itemData(prepared_x->currentIndex(), Qt::ForegroundRole).value<QColor>() !=
	                     QColor(Qt::red) &&
	                 prepared_view.histogramChart()->histogram2D().in_range_entries == 1,
	                 "configured variables were not validated and plotted when runtime data appeared");
	success &= check(prepared_view.histogramChart(0)->size() == prepared_view.histogramChart(1)->size() &&
	                 prepared_view.histogramChart(0)->size() == prepared_view.histogramChart(3)->size(),
	                 "rendering a configured plot changed the equal 2x2 grid cell sizes");

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
