#include "gAnalysisView.h"

// qt charts
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLegend>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QValueAxis>

// qt
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPen>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

// c++
#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <stdexcept>

namespace {

QString sourceName(GAnalysisSource source) {
	return source == GAnalysisSource::true_information ? "True information" : "Digitized";
}

void configureNumberSpin(QDoubleSpinBox* spin) {
	spin->setRange(-1.0e100, 1.0e100);
	spin->setDecimals(6);
	spin->setKeyboardTracking(false);
}

void removeChartContents(QChart* chart) {
	chart->removeAllSeries();
	const auto axes = chart->axes();
	for (auto* axis : axes) {
		chart->removeAxis(axis);
		delete axis;
	}
}

QString plotObjectName(const QString& base, std::size_t index) {
	return index == 0 ? base : base + QString::number(index + 1);
}

class GEqualChartGridLayout final : public QGridLayout
{
public:
	void setEqualCells(bool enabled) {
		equal_cells = enabled;
		invalidate();
	}

	void setGeometry(const QRect& rectangle) override {
		QGridLayout::setGeometry(rectangle);
		if (!equal_cells) { return; }
		const int horizontal_gap = horizontalSpacing() >= 0 ? horizontalSpacing() : spacing();
		const int vertical_gap = verticalSpacing() >= 0 ? verticalSpacing() : spacing();
		const int cell_width = std::max(0, (rectangle.width() - horizontal_gap) / 2);
		const int cell_height = std::max(0, (rectangle.height() - vertical_gap) / 2);
		for (int row = 0; row < 2; ++row) {
			for (int column = 0; column < 2; ++column) {
				if (auto* item = itemAtPosition(row, column)) {
					item->setGeometry(QRect(rectangle.x() + column * (cell_width + horizontal_gap),
					                        rectangle.y() + row * (cell_height + vertical_gap),
					                        cell_width, cell_height));
				}
			}
		}
	}

private:
	bool equal_cells = false;
};

class GHistogram2DItem final : public QGraphicsObject
{
public:
	GHistogram2DItem(QChart* chart, GHistogram2DData histogram, GHistogram2DStyle drawing_style,
	                 bool logarithmic) :
		QGraphicsObject(chart), owner(chart), data(std::move(histogram)), style(drawing_style),
		logarithmic_z(logarithmic) {
		setZValue(2.0);
	}

	[[nodiscard]] QRectF boundingRect() const override { return owner->boundingRect(); }

	void updateGeometry() {
		prepareGeometryChange();
		update();
	}

	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
		if (data.bins == 0 || data.counts.empty()) { return; }
		const QRectF area = owner->plotArea();
		const double cell_width = area.width() / static_cast<double>(data.bins);
		const double cell_height = area.height() / static_cast<double>(data.bins);
		const double maximum = static_cast<double>(data.maximumCount());
		if (maximum <= 0.0) { return; }

		painter->save();
		painter->setClipRect(area);
		painter->setPen(Qt::NoPen);
		for (std::size_t y_bin = 0; y_bin < data.bins; ++y_bin) {
			for (std::size_t x_bin = 0; x_bin < data.bins; ++x_bin) {
				const double count = static_cast<double>(data.count(x_bin, y_bin));
				if (count == 0.0) { continue; }
				const double intensity = logarithmic_z
				                         ? std::log1p(count) / std::log1p(maximum)
				                         : count / maximum;
				QColor color = QColor::fromHsvF(0.68 * (1.0 - intensity), 0.85,
				                                 0.55 + 0.4 * intensity);
				color.setAlphaF(0.25 + 0.75 * intensity);
				painter->setBrush(color);

				QRectF cell(area.left() + static_cast<double>(x_bin) * cell_width,
				            area.bottom() - static_cast<double>(y_bin + 1) * cell_height,
				            cell_width, cell_height);
				if (style == GHistogram2DStyle::boxes) {
					const double fraction = std::sqrt(intensity);
					const QSizeF box_size(cell.width() * fraction, cell.height() * fraction);
					cell = QRectF(cell.center() - QPointF(box_size.width() / 2.0,
					                                      box_size.height() / 2.0), box_size);
				}
				painter->drawRect(cell);
			}
		}
		painter->restore();
	}

private:
	QChart* owner;
	GHistogram2DData data;
	GHistogram2DStyle style;
	bool logarithmic_z;
};

} // namespace

GHistogramChart::GHistogramChart(QWidget* parent) : QChartView(new QChart(), parent) {
	setRenderHint(QPainter::Antialiasing);
	setMinimumSize(0, 0);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	chart()->legend()->hide();
	chart()->setTheme(QChart::ChartThemeQt);
}

void GHistogramChart::setHistogram(const GHistogramData& histogram, const QString& title,
	                                std::optional<double> requested_y_min,
	                                std::optional<double> requested_y_max, bool logarithmic) {
	delete histogram_2d_item;
	histogram_2d_item = nullptr;
	current_histogram = histogram;
	current_histogram_2d = GHistogram2DData{};
	removeChartContents(chart());
	chart()->setTitle(title);
	chart()->legend()->hide();

	auto* x_axis = new QValueAxis;
	x_axis->setTitleText("Value");
	x_axis->setRange(histogram.x_min, histogram.x_max);
	x_axis->setLabelFormat("%.5g");
	chart()->addAxis(x_axis, Qt::AlignBottom);

	const double maximum_count = static_cast<double>(histogram.maximumCount());
	double y_min = requested_y_min.value_or(logarithmic ? 0.5 : 0.0);
	double y_max = requested_y_max.value_or(std::max(logarithmic ? 1.5 : 1.0,
	                                                 maximum_count * 1.15));
	if (logarithmic && y_min <= 0.0) {
		throw std::invalid_argument("Logarithmic y minimum must be positive");
	}
	if (!std::isfinite(y_min) || !std::isfinite(y_max) || y_min >= y_max) {
		throw std::invalid_argument("Histogram y minimum must be finite and below y maximum");
	}

	QAbstractAxis* y_axis = nullptr;
	if (logarithmic) {
		auto* log_axis = new QLogValueAxis;
		log_axis->setBase(10.0);
		log_axis->setLabelFormat("%.5g");
		log_axis->setRange(y_min, y_max);
		y_axis = log_axis;
	}
	else {
		auto* value_axis = new QValueAxis;
		value_axis->setLabelFormat("%.5g");
		value_axis->setRange(y_min, y_max);
		y_axis = value_axis;
	}
	y_axis->setTitleText("Entries");
	chart()->addAxis(y_axis, Qt::AlignLeft);

	const QColor line_color = palette().color(QPalette::Highlight);
	const double width = histogram.binWidth();
	if (logarithmic) {
		for (std::size_t index = 0; index < histogram.counts.size(); ++index) {
			if (histogram.counts[index] == 0) { continue; }
			auto* series = new QLineSeries;
			const double left = histogram.x_min + static_cast<double>(index) * width;
			const double right = left + width;
			series->append(left, static_cast<double>(histogram.counts[index]));
			series->append(right, static_cast<double>(histogram.counts[index]));
			series->setPen(QPen(line_color, 2.0));
			chart()->addSeries(series);
			series->attachAxis(x_axis);
			series->attachAxis(y_axis);
		}
	}
	else {
		auto* series = new QLineSeries;
		series->append(histogram.x_min, 0.0);
		for (std::size_t index = 0; index < histogram.counts.size(); ++index) {
			const double left = histogram.x_min + static_cast<double>(index) * width;
			const double right = left + width;
			const double count = static_cast<double>(histogram.counts[index]);
			series->append(left, count);
			series->append(right, count);
		}
		series->append(histogram.x_max, 0.0);
		series->setPen(QPen(line_color, 2.0));
		chart()->addSeries(series);
		series->attachAxis(x_axis);
		series->attachAxis(y_axis);
	}
}

void GHistogramChart::setHistogram2D(const GHistogram2DData& histogram, const QString& title,
	                                  const QString& x_title, const QString& y_title,
	                                  GHistogram2DStyle style, bool logarithmic_z) {
	delete histogram_2d_item;
	histogram_2d_item = nullptr;
	current_histogram = GHistogramData{};
	current_histogram_2d = histogram;
	removeChartContents(chart());
	chart()->setTitle(title);
	chart()->legend()->hide();

	auto* x_axis = new QValueAxis;
	x_axis->setTitleText(x_title);
	x_axis->setRange(histogram.x_min, histogram.x_max);
	x_axis->setLabelFormat("%.5g");
	chart()->addAxis(x_axis, Qt::AlignBottom);
	auto* y_axis = new QValueAxis;
	y_axis->setTitleText(y_title);
	y_axis->setRange(histogram.y_min, histogram.y_max);
	y_axis->setLabelFormat("%.5g");
	chart()->addAxis(y_axis, Qt::AlignLeft);

	auto* item = new GHistogram2DItem(chart(), histogram, style, logarithmic_z);
	histogram_2d_item = item;
	connect(chart(), &QChart::plotAreaChanged, item,
	        [item](const QRectF&) { item->updateGeometry(); });
}

bool GHistogramChart::exportPdf(const QString& filename) const {
	if (filename.isEmpty()) { return false; }

	QPdfWriter writer(filename);
	writer.setCreator("GEMC Analyzer");
	writer.setTitle(chart()->title());
	writer.setResolution(300);
	writer.setPageSize(QPageSize(QPageSize::A4));
	writer.setPageOrientation(QPageLayout::Landscape);

	QPainter painter(&writer);
	if (!painter.isActive()) { return false; }
	chart()->scene()->render(&painter, QRectF(0.0, 0.0, writer.width(), writer.height()),
	                         chart()->scene()->sceneRect(), Qt::KeepAspectRatio);
	painter.end();
	return QFileInfo(filename).isFile() && QFileInfo(filename).size() > 0;
}

void GHistogramChart::clearHistogram() {
	delete histogram_2d_item;
	histogram_2d_item = nullptr;
	current_histogram = GHistogramData{};
	current_histogram_2d = GHistogram2DData{};
	removeChartContents(chart());
	chart()->setTitle(QString());
	chart()->legend()->hide();
}

class GAnalysisPlotPanel : public QGroupBox
{
public:
	GAnalysisPlotPanel(std::size_t plot_index, QWidget* chart_parent, QWidget* parent = nullptr);

	void applyOptions(const ganalysis::PlotOptions& options);
	void setSnapshot(const GAnalysisSeriesMap* new_snapshot, bool has_completed_run);
	[[nodiscard]] GHistogramChart* histogramChart() const { return histogram_chart; }
	[[nodiscard]] bool exportPdf(const QString& filename) const;

private:
	const GAnalysisSeriesMap* analysis_snapshot = nullptr;
	QComboBox* run_combo = nullptr;
	QComboBox* detector_combo = nullptr;
	QRadioButton* plot_1d = nullptr;
	QRadioButton* plot_2d = nullptr;
	QRadioButton* true_source = nullptr;
	QRadioButton* digitized_source = nullptr;
	QComboBox* x_variable_combo = nullptr;
	QComboBox* y_variable_combo = nullptr;
	QLabel* y_variable_label = nullptr;
	QComboBox* style_combo = nullptr;
	QLabel* style_label = nullptr;
	QSpinBox* bins_spin = nullptr;
	QCheckBox* automatic_x_min = nullptr;
	QCheckBox* automatic_x_max = nullptr;
	QDoubleSpinBox* x_min_spin = nullptr;
	QDoubleSpinBox* x_max_spin = nullptr;
	QCheckBox* automatic_y_min = nullptr;
	QCheckBox* automatic_y_max = nullptr;
	QDoubleSpinBox* y_min_spin = nullptr;
	QDoubleSpinBox* y_max_spin = nullptr;
	QLineEdit* title_edit = nullptr;
	QComboBox* scale_combo = nullptr;
	QLabel* scale_label = nullptr;
	QLabel* status_label = nullptr;
	GHistogramChart* histogram_chart = nullptr;
	std::optional<ganalysis::PlotOptions> configured_options;
	bool validation_started = false;

	[[nodiscard]] const GAnalysisSeriesMap& data() const;
	[[nodiscard]] GAnalysisSource selectedSource() const;
	[[nodiscard]] std::optional<GAnalysisSeriesKey> selectedSeriesKey(QComboBox* combo) const;
	void populateRuns();
	void populateDetectors();
	void populateVariables();
	void populateVariable(QComboBox* combo, const std::set<QString>& variables,
	                      const std::string& configured_name);
	void updatePlotType();
	void updateHistogram();
	void updateHistogram1D(const GAnalysisSeriesKey& x_key, const GAnalysisSeries& x_series);
	void updateHistogram2D(const GAnalysisSeriesKey& x_key, const GAnalysisSeries& x_series);
	void choosePdfFilename();
};

GAnalysisPlotPanel::GAnalysisPlotPanel(std::size_t plot_index, QWidget* chart_parent, QWidget* parent) :
	QGroupBox(parent) {
	setObjectName(QString("analyzerPlot%1").arg(plot_index + 1));

	run_combo = new QComboBox(this);
	run_combo->setObjectName(plotObjectName("analyzerRun", plot_index));
	detector_combo = new QComboBox(this);
	detector_combo->setObjectName(plotObjectName("analyzerDetector", plot_index));
	plot_1d = new QRadioButton("1D", this);
	plot_2d = new QRadioButton("2D", this);
	plot_1d->setObjectName(plotObjectName("analyzerPlot1D", plot_index));
	plot_2d->setObjectName(plotObjectName("analyzerPlot2D", plot_index));
	plot_1d->setChecked(true);
	auto* dimension_group = new QButtonGroup(this);
	dimension_group->addButton(plot_1d);
	dimension_group->addButton(plot_2d);

	true_source = new QRadioButton("True information", this);
	digitized_source = new QRadioButton("Digitized", this);
	true_source->setObjectName(plotObjectName("analyzerTrueSource", plot_index));
	digitized_source->setObjectName(plotObjectName("analyzerDigitizedSource", plot_index));
	true_source->setChecked(true);
	auto* source_group = new QButtonGroup(this);
	source_group->addButton(true_source);
	source_group->addButton(digitized_source);

	x_variable_combo = new QComboBox(this);
	y_variable_combo = new QComboBox(this);
	x_variable_combo->setEditable(false);
	y_variable_combo->setEditable(false);
	x_variable_combo->setObjectName(plotObjectName("analyzerXVariable", plot_index));
	y_variable_combo->setObjectName(plotObjectName("analyzerYVariable", plot_index));
	y_variable_label = new QLabel("Y variable:", this);
	style_combo = new QComboBox(this);
	style_combo->setObjectName(plotObjectName("analyzerStyle", plot_index));
	style_combo->addItem("Heatmap", static_cast<int>(GHistogram2DStyle::heatmap));
	style_combo->addItem("Boxes", static_cast<int>(GHistogram2DStyle::boxes));
	style_label = new QLabel("2D style:", this);

	auto* selectors = new QGridLayout;
	selectors->addWidget(new QLabel("Run:"), 0, 0);
	selectors->addWidget(run_combo, 0, 1);
	selectors->addWidget(new QLabel("Plugin / detector:"), 0, 2);
	selectors->addWidget(detector_combo, 0, 3, 1, 3);
	selectors->addWidget(new QLabel("Plot:"), 1, 0);
	selectors->addWidget(plot_1d, 1, 1);
	selectors->addWidget(plot_2d, 1, 2);
	selectors->addWidget(new QLabel("Variables:"), 1, 3);
	selectors->addWidget(true_source, 1, 4);
	selectors->addWidget(digitized_source, 1, 5);
	selectors->addWidget(new QLabel("X variable:"), 2, 0);
	selectors->addWidget(x_variable_combo, 2, 1, 1, 2);
	selectors->addWidget(y_variable_label, 2, 3);
	selectors->addWidget(y_variable_combo, 2, 4, 1, 2);
	selectors->addWidget(style_label, 3, 0);
	selectors->addWidget(style_combo, 3, 1, 1, 2);
	selectors->setColumnStretch(1, 1);
	selectors->setColumnStretch(4, 1);

	bins_spin = new QSpinBox(this);
	bins_spin->setObjectName(plotObjectName("analyzerBins", plot_index));
	bins_spin->setRange(1, 10000);
	bins_spin->setValue(100);

	automatic_x_min = new QCheckBox("Auto", this);
	automatic_x_max = new QCheckBox("Auto", this);
	automatic_x_min->setObjectName(plotObjectName("analyzerAutomaticXMin", plot_index));
	automatic_x_max->setObjectName(plotObjectName("analyzerAutomaticXMax", plot_index));
	automatic_x_min->setChecked(true);
	automatic_x_max->setChecked(true);
	x_min_spin = new QDoubleSpinBox(this);
	x_max_spin = new QDoubleSpinBox(this);
	x_min_spin->setObjectName(plotObjectName("analyzerXMin", plot_index));
	x_max_spin->setObjectName(plotObjectName("analyzerXMax", plot_index));
	configureNumberSpin(x_min_spin);
	configureNumberSpin(x_max_spin);
	x_min_spin->setEnabled(false);
	x_max_spin->setEnabled(false);

	automatic_y_min = new QCheckBox("Auto", this);
	automatic_y_max = new QCheckBox("Auto", this);
	automatic_y_min->setObjectName(plotObjectName("analyzerAutomaticYMin", plot_index));
	automatic_y_max->setObjectName(plotObjectName("analyzerAutomaticYMax", plot_index));
	automatic_y_min->setChecked(true);
	automatic_y_max->setChecked(true);
	y_min_spin = new QDoubleSpinBox(this);
	y_max_spin = new QDoubleSpinBox(this);
	y_min_spin->setObjectName(plotObjectName("analyzerYMin", plot_index));
	y_max_spin->setObjectName(plotObjectName("analyzerYMax", plot_index));
	configureNumberSpin(y_min_spin);
	configureNumberSpin(y_max_spin);
	y_min_spin->setValue(0.0);
	y_max_spin->setValue(100.0);
	y_min_spin->setEnabled(false);
	y_max_spin->setEnabled(false);

	title_edit = new QLineEdit(this);
	title_edit->setObjectName(plotObjectName("analyzerTitle", plot_index));
	title_edit->setPlaceholderText("Automatic histogram title");
	scale_combo = new QComboBox(this);
	scale_combo->setObjectName(plotObjectName("analyzerScale", plot_index));
	scale_combo->addItem("Linear", false);
	scale_combo->addItem("Logarithmic", true);
	scale_label = new QLabel("Y scale:", this);

	auto* controls = new QGridLayout;
	controls->addWidget(new QLabel("Bins:"), 0, 0);
	controls->addWidget(bins_spin, 0, 1);
	controls->addWidget(new QLabel("Title:"), 0, 2);
	controls->addWidget(title_edit, 0, 3, 1, 3);
	controls->addWidget(scale_label, 0, 6);
	controls->addWidget(scale_combo, 0, 7);
	controls->addWidget(new QLabel("X min:"), 1, 0);
	controls->addWidget(x_min_spin, 1, 1);
	controls->addWidget(automatic_x_min, 1, 2);
	controls->addWidget(new QLabel("X max:"), 1, 3);
	controls->addWidget(x_max_spin, 1, 4);
	controls->addWidget(automatic_x_max, 1, 5);
	controls->addWidget(new QLabel("Y min:"), 2, 0);
	controls->addWidget(y_min_spin, 2, 1);
	controls->addWidget(automatic_y_min, 2, 2);
	controls->addWidget(new QLabel("Y max:"), 2, 3);
	controls->addWidget(y_max_spin, 2, 4);
	controls->addWidget(automatic_y_max, 2, 5);
	controls->setColumnStretch(3, 1);

	histogram_chart = new GHistogramChart(chart_parent);
	histogram_chart->setObjectName(plotObjectName("analyzerChart", plot_index));
	status_label = new QLabel("Run beamOn to discover Analyzer variables.", this);
	status_label->setWordWrap(true);
	auto* pdf_button = new QPushButton("Export PDF…", this);
	pdf_button->setObjectName(plotObjectName("analyzerExportPdf", plot_index));
	pdf_button->setToolTip("Save this histogram as a vector PDF");

	auto* footer = new QHBoxLayout;
	footer->addWidget(status_label, 1);
	footer->addWidget(pdf_button);
	auto* settings = new QHBoxLayout;
	settings->addLayout(selectors, 3);
	settings->addLayout(controls, 4);
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(6, 4, 6, 4);
	layout->setSpacing(2);
	layout->addLayout(settings);
	layout->addLayout(footer);
	setLayout(layout);

	connect(run_combo, &QComboBox::currentIndexChanged, this, [this]() { populateDetectors(); });
	connect(detector_combo, &QComboBox::currentIndexChanged, this, [this]() { populateVariables(); });
	connect(plot_1d, &QRadioButton::toggled, this,
	        [this](bool checked) { if (checked) { updatePlotType(); } });
	connect(plot_2d, &QRadioButton::toggled, this,
	        [this](bool checked) { if (checked) { updatePlotType(); } });
	connect(true_source, &QRadioButton::toggled, this,
	        [this](bool checked) { if (checked) { populateVariables(); } });
	connect(digitized_source, &QRadioButton::toggled, this,
	        [this](bool checked) { if (checked) { populateVariables(); } });
	connect(x_variable_combo, &QComboBox::currentIndexChanged, this,
	        [this]() { updateHistogram(); });
	connect(y_variable_combo, &QComboBox::currentIndexChanged, this,
	        [this]() { updateHistogram(); });
	connect(style_combo, &QComboBox::currentIndexChanged, this,
	        [this]() { updateHistogram(); });
	connect(bins_spin, &QSpinBox::valueChanged, this, [this]() { updateHistogram(); });
	const auto connect_automatic = [this](QCheckBox* automatic, QDoubleSpinBox* value) {
		connect(automatic, &QCheckBox::toggled, this, [this, value](bool checked) {
			value->setEnabled(!checked);
			updateHistogram();
		});
	};
	connect_automatic(automatic_x_min, x_min_spin);
	connect_automatic(automatic_x_max, x_max_spin);
	connect_automatic(automatic_y_min, y_min_spin);
	connect_automatic(automatic_y_max, y_max_spin);
	for (auto* spin : {x_min_spin, x_max_spin, y_min_spin, y_max_spin}) {
		connect(spin, &QDoubleSpinBox::valueChanged, this, [this]() { updateHistogram(); });
	}
	connect(title_edit, &QLineEdit::textChanged, this, [this]() { updateHistogram(); });
	connect(scale_combo, &QComboBox::currentIndexChanged, this, [this]() { updateHistogram(); });
	connect(pdf_button, &QPushButton::clicked, this, [this]() { choosePdfFilename(); });
	updatePlotType();
}

void GAnalysisPlotPanel::applyOptions(const ganalysis::PlotOptions& options) {
	configured_options = options;
	(options.source == "digitized" ? digitized_source : true_source)->setChecked(true);
	(options.dimension == "2d" ? plot_2d : plot_1d)->setChecked(true);
	bins_spin->setValue(options.bins);
	automatic_x_min->setChecked(options.automatic_x_min);
	automatic_x_max->setChecked(options.automatic_x_max);
	automatic_y_min->setChecked(options.automatic_y_min);
	automatic_y_max->setChecked(options.automatic_y_max);
	x_min_spin->setValue(options.x_min);
	x_max_spin->setValue(options.x_max);
	y_min_spin->setValue(options.y_min);
	y_max_spin->setValue(options.y_max);
	title_edit->setText(QString::fromStdString(options.title));
	style_combo->setCurrentIndex(options.style == "boxes" ? 1 : 0);
	scale_combo->setCurrentIndex(options.scale == "log" || options.scale == "logarithmic" ? 1 : 0);
	updatePlotType();
}

const GAnalysisSeriesMap& GAnalysisPlotPanel::data() const {
	static const GAnalysisSeriesMap empty;
	return analysis_snapshot != nullptr ? *analysis_snapshot : empty;
}

GAnalysisSource GAnalysisPlotPanel::selectedSource() const {
	return true_source->isChecked() ? GAnalysisSource::true_information : GAnalysisSource::digitized;
}

std::optional<GAnalysisSeriesKey> GAnalysisPlotPanel::selectedSeriesKey(QComboBox* combo) const {
	if (run_combo == nullptr || detector_combo == nullptr || combo->currentIndex() <= 0) {
		return std::nullopt;
	}
	return GAnalysisSeriesKey{run_combo->currentData().toInt(),
	                          detector_combo->currentData().toString().toStdString(), selectedSource(),
	                          combo->currentData().toString().toStdString()};
}

void GAnalysisPlotPanel::setSnapshot(const GAnalysisSeriesMap* new_snapshot, bool has_completed_run) {
	const auto previous_x = selectedSeriesKey(x_variable_combo);
	const auto previous_y = selectedSeriesKey(y_variable_combo);
	analysis_snapshot = new_snapshot;
	validation_started = has_completed_run;
	populateRuns();

	const auto restore = [](QComboBox* combo, const std::optional<GAnalysisSeriesKey>& key) {
		if (!key.has_value()) { return; }
		const int index = combo->findData(QString::fromStdString(key->variable));
		if (index > 0) { combo->setCurrentIndex(index); }
	};
	if (previous_x.has_value()) {
		const int run_index = run_combo->findData(previous_x->run_number);
		if (run_index >= 0) { run_combo->setCurrentIndex(run_index); }
		const int detector_index = detector_combo->findData(QString::fromStdString(previous_x->detector));
		if (detector_index >= 0) { detector_combo->setCurrentIndex(detector_index); }
		(previous_x->source == GAnalysisSource::true_information ? true_source : digitized_source)
			->setChecked(true);
	}
	restore(x_variable_combo, previous_x);
	restore(y_variable_combo, previous_y);
	if (data().empty()) { status_label->setText("Run beamOn to discover Analyzer variables."); }
}

void GAnalysisPlotPanel::populateRuns() {
	const QSignalBlocker blocker(run_combo);
	const int previous_run = run_combo->currentData().toInt();
	std::set<int> runs;
	for (const auto& [key, series] : data()) {
		(void) series;
		runs.insert(key.run_number);
	}
	if (configured_options.has_value() && configured_options->run >= 0) {
		runs.insert(configured_options->run);
	}
	run_combo->clear();
	for (int run : runs) { run_combo->addItem(QString::number(run), run); }
	const int configured_run = configured_options.has_value() ? configured_options->run : -1;
	const int previous_index = run_combo->findData(configured_run >= 0 ? configured_run : previous_run);
	if (previous_index >= 0) { run_combo->setCurrentIndex(previous_index); }
	populateDetectors();
}

void GAnalysisPlotPanel::populateDetectors() {
	const QSignalBlocker blocker(detector_combo);
	const QString previous_detector = detector_combo->currentData().toString();
	const int run = run_combo->currentData().toInt();
	std::set<QString> detectors;
	for (const auto& [key, series] : data()) {
		(void) series;
		if (key.run_number == run) { detectors.insert(QString::fromStdString(key.detector)); }
	}
	if (configured_options.has_value() && !configured_options->plugin.empty()) {
		detectors.insert(QString::fromStdString(configured_options->plugin));
	}
	detector_combo->clear();
	for (const auto& detector : detectors) { detector_combo->addItem(detector, detector); }
	const QString configured_detector = configured_options.has_value()
	                                    ? QString::fromStdString(configured_options->plugin) : QString{};
	const int previous_index = detector_combo->findData(
		configured_detector.isEmpty() ? previous_detector : configured_detector);
	if (previous_index >= 0) { detector_combo->setCurrentIndex(previous_index); }
	populateVariables();
}

void GAnalysisPlotPanel::populateVariables() {
	const QSignalBlocker x_blocker(x_variable_combo);
	const QSignalBlocker y_blocker(y_variable_combo);
	const QString previous_x = x_variable_combo->currentData().toString();
	const QString previous_y = y_variable_combo->currentData().toString();
	const int run = run_combo->currentData().toInt();
	const std::string detector = detector_combo->currentData().toString().toStdString();
	const GAnalysisSource source = selectedSource();
	std::set<QString> variables;
	for (const auto& [key, series] : data()) {
		(void) series;
		if (key.run_number == run && key.detector == detector && key.source == source) {
			variables.insert(QString::fromStdString(key.variable));
		}
	}

	const std::string configured_x = configured_options.has_value()
	                                 ? configured_options->x_variable : std::string{};
	const std::string configured_y = configured_options.has_value()
	                                 ? configured_options->y_variable : std::string{};
	populateVariable(x_variable_combo, variables, configured_x);
	populateVariable(y_variable_combo, variables, configured_y);
	if (configured_x.empty()) {
		const int x_index = x_variable_combo->findData(previous_x);
		if (x_index > 0) { x_variable_combo->setCurrentIndex(x_index); }
	}
	if (configured_y.empty()) {
		const int y_index = y_variable_combo->findData(previous_y);
		if (y_index > 0) { y_variable_combo->setCurrentIndex(y_index); }
	}
	updateHistogram();
}

void GAnalysisPlotPanel::populateVariable(QComboBox* combo, const std::set<QString>& variables,
	                                       const std::string& configured_name) {
	combo->clear();
	combo->addItem("Select variable…");
	for (const auto& variable : variables) { combo->addItem(variable, variable); }
	if (configured_name.empty()) { return; }

	const QString name = QString::fromStdString(configured_name);
	int index = combo->findData(name);
	if (index < 0) {
		const QString label = validation_started ? name + " — Not available" : name;
		combo->addItem(label, name);
		index = combo->count() - 1;
		combo->setItemData(index, QColor(Qt::red), Qt::ForegroundRole);
		combo->setItemData(index, validation_started ? 2 : 1, Qt::UserRole + 1);
	}
	combo->setCurrentIndex(index);
}

void GAnalysisPlotPanel::updatePlotType() {
	const bool is_2d = plot_2d->isChecked();
	bins_spin->setMaximum(is_2d ? 500 : 10000);
	y_variable_label->setEnabled(is_2d);
	y_variable_combo->setEnabled(is_2d);
	style_label->setEnabled(is_2d);
	style_combo->setEnabled(is_2d);
	scale_label->setText(is_2d ? "Z scale:" : "Y scale:");
	updateHistogram();
}

void GAnalysisPlotPanel::updateHistogram() {
	const auto x_key = selectedSeriesKey(x_variable_combo);
	if (!x_key.has_value()) {
		histogram_chart->clearHistogram();
		if (!data().empty()) { status_label->setText("Select an X variable to plot."); }
		return;
	}
	const auto x_series = data().find(*x_key);
	if (x_series == data().end()) {
		histogram_chart->clearHistogram();
		status_label->setText(validation_started
		                      ? QString("%1 — Not available").arg(x_variable_combo->currentData().toString())
		                      : "Run beamOn to validate configured variables.");
		return;
	}
	try {
		if (plot_2d->isChecked()) { updateHistogram2D(*x_key, x_series->second); }
		else { updateHistogram1D(*x_key, x_series->second); }
	}
	catch (const std::exception& error) {
		histogram_chart->clearHistogram();
		status_label->setText(QString("Cannot draw histogram: %1").arg(error.what()));
	}
}

void GAnalysisPlotPanel::updateHistogram1D(const GAnalysisSeriesKey& x_key,
	                                        const GAnalysisSeries& x_series) {
	const std::optional<double> x_min = automatic_x_min->isChecked()
	                                    ? std::nullopt : std::optional<double>(x_min_spin->value());
	const std::optional<double> x_max = automatic_x_max->isChecked()
	                                    ? std::nullopt : std::optional<double>(x_max_spin->value());
	const auto histogram = makeHistogram(x_series.values(), static_cast<std::size_t>(bins_spin->value()),
	                                     x_min, x_max);
	const std::optional<double> y_min = automatic_y_min->isChecked()
	                                    ? std::nullopt : std::optional<double>(y_min_spin->value());
	const std::optional<double> y_max = automatic_y_max->isChecked()
	                                    ? std::nullopt : std::optional<double>(y_max_spin->value());
	const QString automatic_title = QString("%1 / %2 / %3 — run %4")
	                                    .arg(QString::fromStdString(x_key.detector),
	                                         sourceName(x_key.source),
	                                         QString::fromStdString(x_key.variable))
	                                    .arg(x_key.run_number);
	const QString title = title_edit->text().trimmed().isEmpty()
	                      ? automatic_title : title_edit->text().trimmed();
	histogram_chart->setHistogram(histogram, title, y_min, y_max, scale_combo->currentData().toBool());
	status_label->setText(QString("%1 entries in range; %2 underflow; %3 overflow")
	                      .arg(histogram.in_range_entries).arg(histogram.underflow).arg(histogram.overflow));
}

void GAnalysisPlotPanel::updateHistogram2D(const GAnalysisSeriesKey& x_key,
	                                        const GAnalysisSeries& x_series) {
	const auto y_key = selectedSeriesKey(y_variable_combo);
	if (!y_key.has_value()) {
		histogram_chart->clearHistogram();
		status_label->setText("Select a Y variable to plot.");
		return;
	}
	const auto y_series = data().find(*y_key);
	if (y_series == data().end()) {
		histogram_chart->clearHistogram();
		status_label->setText(validation_started
		                      ? QString("%1 — Not available").arg(y_variable_combo->currentData().toString())
		                      : "Run beamOn to validate configured variables.");
		return;
	}
	const auto paired_values = matchAnalysisSeries(x_series, y_series->second);
	const std::optional<double> x_min = automatic_x_min->isChecked()
	                                    ? std::nullopt : std::optional<double>(x_min_spin->value());
	const std::optional<double> x_max = automatic_x_max->isChecked()
	                                    ? std::nullopt : std::optional<double>(x_max_spin->value());
	const std::optional<double> y_min = automatic_y_min->isChecked()
	                                    ? std::nullopt : std::optional<double>(y_min_spin->value());
	const std::optional<double> y_max = automatic_y_max->isChecked()
	                                    ? std::nullopt : std::optional<double>(y_max_spin->value());
	const auto histogram = makeHistogram2D(paired_values, static_cast<std::size_t>(bins_spin->value()),
	                                       x_min, x_max, y_min, y_max);
	const QString x_name = QString::fromStdString(x_key.variable);
	const QString y_name = QString::fromStdString(y_key->variable);
	const QString automatic_title = QString("%1 / %2 / %3 vs %4 — run %5")
	                                    .arg(QString::fromStdString(x_key.detector),
	                                         sourceName(x_key.source), y_name, x_name)
	                                    .arg(x_key.run_number);
	const QString title = title_edit->text().trimmed().isEmpty()
	                      ? automatic_title : title_edit->text().trimmed();
	const auto style = static_cast<GHistogram2DStyle>(style_combo->currentData().toInt());
	histogram_chart->setHistogram2D(histogram, title, x_name, y_name, style,
	                                scale_combo->currentData().toBool());
	status_label->setText(QString("%1 paired entries in range; %2 outside the selected axes")
	                      .arg(histogram.in_range_entries).arg(histogram.out_of_range));
}

bool GAnalysisPlotPanel::exportPdf(const QString& filename) const {
	return histogram_chart != nullptr && histogram_chart->exportPdf(filename);
}

void GAnalysisPlotPanel::choosePdfFilename() {
	QString filename = QFileDialog::getSaveFileName(this, "Export Analyzer histogram", QString(),
	                                                "PDF documents (*.pdf)");
	if (filename.isEmpty()) { return; }
	if (!filename.endsWith(".pdf", Qt::CaseInsensitive)) { filename += ".pdf"; }
	if (!exportPdf(filename)) {
		QMessageBox::warning(this, "Analyzer PDF export", "The histogram could not be written to PDF.");
	}
}

GAnalysisView::GAnalysisView(std::shared_ptr<GAnalysisAccumulator> accumulator, QWidget* parent) :
	GAnalysisView(std::move(accumulator), ganalysis::Options{}, parent) {
}

GAnalysisView::GAnalysisView(std::shared_ptr<GAnalysisAccumulator> accumulator,
	                         const ganalysis::Options& startup_options, QWidget* parent) :
	QWidget(parent), analysis_accumulator(std::move(accumulator)) {
	setObjectName("gemcAnalyzerView");

	accumulate_button = new QPushButton("Accumulate", this);
	accumulate_button->setObjectName("analyzerAccumulate");
	accumulate_button->setCheckable(true);
	accumulate_button->setChecked(startup_options.accumulate);
	accumulate_button->setToolTip("Preserve Analyzer samples and append subsequent beamOn results");
	four_plots_button = new QPushButton("4 plots", this);
	four_plots_button->setObjectName("analyzerFourPlots");
	four_plots_button->setCheckable(true);
	four_plots_button->setToolTip("Switch between one plot and four independently configured plots");

	auto* toolbar = new QHBoxLayout;
	toolbar->addWidget(accumulate_button);
	toolbar->addStretch(1);
	toolbar->addWidget(four_plots_button);

	configuration_tabs = new QTabWidget(this);
	configuration_tabs->setObjectName("analyzerConfigurationTabs");
	configuration_tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	plots_layout = new GEqualChartGridLayout;
	plots_layout->setContentsMargins(0, 0, 0, 0);
	plots_layout->setSpacing(4);
	plots_layout->setRowStretch(0, 1);
	plots_layout->setRowStretch(1, 1);
	plots_layout->setColumnStretch(0, 1);
	plots_layout->setColumnStretch(1, 1);
	const std::array<QString, 4> tab_names{"Top Left", "Top Right", "Bottom Left", "Bottom Right"};
	for (std::size_t index = 0; index < plots.size(); ++index) {
		plots[index] = new GAnalysisPlotPanel(index, this, configuration_tabs);
		if (startup_options.plots[index].has_value()) {
			plots[index]->applyOptions(*startup_options.plots[index]);
		}
		configuration_tabs->addTab(plots[index], tab_names[index]);
	}
	configuration_tabs->setMaximumHeight(configuration_tabs->sizeHint().height());

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(4);
	layout->addLayout(toolbar);
	layout->addWidget(configuration_tabs);
	layout->addLayout(plots_layout, 1);
	setLayout(layout);

	connect(four_plots_button, &QPushButton::toggled,
	        this, [this](bool enabled) { setFourPlotMode(enabled); });
	four_plots_button->setChecked(startup_options.plot_count == 4);
	if (startup_options.plot_count != 4) { setFourPlotMode(false); }
	refresh();
}

void GAnalysisView::setFourPlotMode(bool enabled) {
	static_cast<GEqualChartGridLayout*>(plots_layout)->setEqualCells(enabled);
	for (auto* plot : plots) { plots_layout->removeWidget(plot->histogramChart()); }
	if (enabled) {
		for (std::size_t index = 0; index < plots.size(); ++index) {
			plots_layout->addWidget(plots[index]->histogramChart(), static_cast<int>(index / 2),
			                        static_cast<int>(index % 2));
			plots[index]->histogramChart()->show();
			configuration_tabs->setTabVisible(static_cast<int>(index), true);
		}
		four_plots_button->setText("1 plot");
	}
	else {
		plots_layout->addWidget(plots.front()->histogramChart(), 0, 0, 2, 2);
		plots.front()->histogramChart()->show();
		for (std::size_t index = 1; index < plots.size(); ++index) {
			plots[index]->histogramChart()->hide();
			configuration_tabs->setTabVisible(static_cast<int>(index), false);
		}
		configuration_tabs->setCurrentIndex(0);
		four_plots_button->setText("4 plots");
	}
}

void GAnalysisView::refresh() {
	snapshot = analysis_accumulator != nullptr ? analysis_accumulator->snapshot() : GAnalysisSeriesMap{};
	const bool has_completed_run = analysis_accumulator != nullptr && analysis_accumulator->beamOnCount() > 0;
	for (auto* plot : plots) { plot->setSnapshot(&snapshot, has_completed_run); }
}

void GAnalysisView::clearForGeometryReload() {
	if (analysis_accumulator != nullptr) { analysis_accumulator->clear(); }
	refresh();
}

bool GAnalysisView::exportPdf(const QString& filename, std::size_t plot_index) const {
	return plot_index < plots.size() && plots[plot_index] != nullptr &&
	       plots[plot_index]->exportPdf(filename);
}

const GHistogramChart* GAnalysisView::histogramChart(std::size_t plot_index) const {
	return plot_index < plots.size() && plots[plot_index] != nullptr
	       ? plots[plot_index]->histogramChart() : nullptr;
}

bool GAnalysisView::fourPlotMode() const {
	return four_plots_button != nullptr && four_plots_button->isChecked();
}

bool GAnalysisView::accumulateEnabled() const {
	return accumulate_button != nullptr && accumulate_button->isChecked();
}
