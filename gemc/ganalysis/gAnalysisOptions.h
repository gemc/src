#pragma once

// gemc
#include <gemc/goptions/goptions.h>

// c++
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

namespace ganalysis {

/** \brief Startup settings for one Analyzer plot position. */
struct PlotOptions {
	std::size_t position = 0;
	int run = -1;
	std::string plugin;
	std::string source = "true";
	std::string dimension = "1d";
	std::string x_variable;
	std::string y_variable;
	int bins = 100;
	bool automatic_x_min = true;
	bool automatic_x_max = true;
	bool automatic_y_min = true;
	bool automatic_y_max = true;
	double x_min = 0.0;
	double x_max = 1.0;
	double y_min = 0.0;
	double y_max = 1.0;
	std::string title;
	std::string style = "heatmap";
	std::string scale = "linear";
};

/** \brief Complete Analyzer startup configuration projected from GOptions. */
struct Options {
	std::size_t plot_count = 1;
	bool accumulate = false;
	std::array<std::optional<PlotOptions>, 4> plots;
};

/** \brief Define Analyzer layout and per-plot options. */
[[nodiscard]] GOptions defineOptions();

/** \brief Project resolved GEMC options into strongly typed Analyzer settings. */
[[nodiscard]] Options getOptions(const std::shared_ptr<GOptions>& gopts);

} // namespace ganalysis
