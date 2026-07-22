#pragma once

// gemc
#include "gAnalysisAccumulator.h"
#include "gAnalysisOptions.h"
#include "gHistogram.h"

// qt
#include <QtCharts/QChartView>
#include <QString>
#include <QWidget>

// c++
#include <array>
#include <cstddef>
#include <memory>
#include <optional>

class GAnalysisPlotPanel;
class QGraphicsItem;
class QGridLayout;
class QPushButton;
class QTabWidget;

/** \brief Drawing styles available for a binned 2D histogram. */
enum class GHistogram2DStyle {
	heatmap,
	boxes
};

/** \brief Qt Charts renderer shared by the Analyzer page and PDF export. */
class GHistogramChart : public QChartView
{
public:
	explicit GHistogramChart(QWidget* parent = nullptr);

	void setHistogram(const GHistogramData& histogram, const QString& title,
	                  std::optional<double> y_min, std::optional<double> y_max,
	                  bool logarithmic);
	void setHistogram2D(const GHistogram2DData& histogram, const QString& title,
	                    const QString& x_title, const QString& y_title,
	                    GHistogram2DStyle style, bool logarithmic_z);

	/** \brief Render the current chart to a vector PDF. */
	[[nodiscard]] bool exportPdf(const QString& filename) const;

	/** \brief Remove the current series and axes so stale data cannot remain visible. */
	void clearHistogram();

	[[nodiscard]] const GHistogramData& histogram() const { return current_histogram; }
	[[nodiscard]] const GHistogram2DData& histogram2D() const { return current_histogram_2d; }

private:
	GHistogramData current_histogram;
	GHistogram2DData current_histogram_2d;
	QGraphicsItem* histogram_2d_item = nullptr;
};

/**
 * \brief Interactive Analyzer page backed by a GUI-only GAnalysisAccumulator.
 *
 * The page discovers runs, detector plugins, and variables from accumulated runtime records. No plugin schema
 * is required. It switches between one plot and a chart-only 2x2 grid. Four named configuration tabs retain
 * independent selections and display settings while sharing the same immutable data snapshot.
 */
class GAnalysisView : public QWidget
{
public:
	explicit GAnalysisView(std::shared_ptr<GAnalysisAccumulator> accumulator,
	                       QWidget* parent = nullptr);
	GAnalysisView(std::shared_ptr<GAnalysisAccumulator> accumulator,
	              const ganalysis::Options& startup_options, QWidget* parent = nullptr);

	/** \brief Refresh all selectors from a consistent accumulator snapshot. */
	void refresh();

	/** \brief Clear persisted samples and plots before a GUI geometry reload. */
	void clearForGeometryReload();

	/** \brief Export one displayed chart without opening a file dialog. */
	[[nodiscard]] bool exportPdf(const QString& filename, std::size_t plot_index = 0) const;

	/** \brief Return the chart widget for non-interactive validation. */
	[[nodiscard]] const GHistogramChart* histogramChart(std::size_t plot_index = 0) const;

	/** \brief Return whether all four independently configured plots are visible. */
	[[nodiscard]] bool fourPlotMode() const;

	/** \brief Return whether subsequent GUI beamOn calls should preserve accumulated samples. */
	[[nodiscard]] bool accumulateEnabled() const;

private:
	std::shared_ptr<GAnalysisAccumulator> analysis_accumulator;
	GAnalysisSeriesMap                    snapshot;
	std::array<GAnalysisPlotPanel*, 4> plots{};
	QGridLayout* plots_layout = nullptr;
	QTabWidget* configuration_tabs = nullptr;
	QPushButton* accumulate_button = nullptr;
	QPushButton* four_plots_button = nullptr;

	void setFourPlotMode(bool enabled);
};
