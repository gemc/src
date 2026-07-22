#include "gAnalysisOptions.h"

// c++
#include <algorithm>
#include <cctype>
#include <vector>

namespace ganalysis {
namespace {

std::string normalized(std::string value) {
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
		return character == '-' || character == ' ' ? '_' : static_cast<char>(std::tolower(character));
	});
	return value;
}

std::optional<std::size_t> positionIndex(const std::string& position) {
	const std::string value = normalized(position);
	if (value == "top_left") { return 0; }
	if (value == "top_right") { return 1; }
	if (value == "bottom_left") { return 2; }
	if (value == "bottom_right") { return 3; }
	return std::nullopt;
}

template<typename T>
T valueOr(const std::shared_ptr<GOptions>& gopts, const YAML::Node& node,
	      const std::string& key, const T& fallback) {
	return gopts->get_variable_in_option<T>(node, key, fallback);
}

} // namespace

GOptions defineOptions() {
	GOptions options("ganalysis");
	options.defineOption(GVariable("ganalysis_plots", 1,
	                               "initial Analyzer layout: 1 or 4 plots"),
	                     "Use 1 for a single plot or 4 for an equal-size 2x2 chart grid.\n");
	options.defineOption(GVariable("ganalysis_accumulate", false,
	                               "initial state of the Analyzer Accumulate button"),
	                     "When true, subsequent GUI beamOn calls append to the Analyzer data.\n");

	std::vector<GVariable> plot_schema = {
		{"position", goptions::NODFLT,
		 "plot position: top_left, top_right, bottom_left, or bottom_right"},
		{"run", -1, "run number; -1 selects the first available run"},
		{"plugin", "''", "runtime sensitive-detector/plugin name"},
		{"source", "true", "variable source: true or digitized"},
		{"dimension", "1d", "histogram dimension: 1d or 2d"},
		{"x", "''", "X variable name to validate after beamOn"},
		{"y", "''", "Y variable name for a 2D histogram"},
		{"bins", 100, "number of bins per axis"},
		{"x_min_auto", true, "derive the X minimum from samples"},
		{"x_max_auto", true, "derive the X maximum from samples"},
		{"y_min_auto", true, "derive the Y minimum from samples"},
		{"y_max_auto", true, "derive the Y maximum from samples"},
		{"x_min", 0.0, "fixed X minimum when x_min_auto is false"},
		{"x_max", 1.0, "fixed X maximum when x_max_auto is false"},
		{"y_min", 0.0, "fixed Y minimum when y_min_auto is false"},
		{"y_max", 1.0, "fixed Y maximum when y_max_auto is false"},
		{"title", "''", "plot title; empty uses an automatic title"},
		{"style", "heatmap", "2D rendering style: heatmap or boxes"},
		{"scale", "linear", "1D Y scale or 2D Z scale: linear or logarithmic"}
	};
	std::string help = "Prepare Analyzer plots before the GUI starts. Variable names remain red until a\n";
	help += "beamOn run validates them against runtime data. Example:\n\n";
	help += "ganalysis_plots: 4\n";
	help += "ganalysis_accumulate: true\n";
	help += "ganalysis:\n";
	help += "  - position: top_left\n";
	help += "    run: 11\n";
	help += "    plugin: flux\n";
	help += "    source: true\n";
	help += "    dimension: 2d\n";
	help += "    x: avg_x\n";
	help += "    y: avg_y\n";
	help += "    bins: 100\n";
	help += "    style: heatmap\n";
	help += "    scale: logarithmic\n";
	options.defineOption("ganalysis", "prepare GUI Analyzer plots", plot_schema, help);
	return options;
}

Options getOptions(const std::shared_ptr<GOptions>& gopts) {
	Options result;
	if (gopts == nullptr || !gopts->doesOptionExist("ganalysis_plots")) { return result; }
	result.plot_count = gopts->getScalarInt("ganalysis_plots") == 4 ? 4 : 1;
	result.accumulate = gopts->getOptionNode("ganalysis_accumulate").as<bool>();
	if (!gopts->doesOptionExist("ganalysis")) { return result; }

	const YAML::Node plots = gopts->getOptionNode("ganalysis");
	if (!plots || !plots.IsSequence()) { return result; }
	for (const auto& node : plots) {
		const auto position = positionIndex(valueOr<std::string>(gopts, node, "position", ""));
		if (!position.has_value()) { continue; }
		PlotOptions plot;
		plot.position = *position;
		plot.run = valueOr<int>(gopts, node, "run", -1);
		plot.plugin = valueOr<std::string>(gopts, node, "plugin", "");
		plot.source = normalized(valueOr<std::string>(gopts, node, "source", "true"));
		plot.dimension = normalized(valueOr<std::string>(gopts, node, "dimension", "1d"));
		plot.x_variable = valueOr<std::string>(gopts, node, "x", "");
		plot.y_variable = valueOr<std::string>(gopts, node, "y", "");
		plot.bins = std::clamp(valueOr<int>(gopts, node, "bins", 100), 1, 10000);
		plot.automatic_x_min = valueOr<bool>(gopts, node, "x_min_auto", true);
		plot.automatic_x_max = valueOr<bool>(gopts, node, "x_max_auto", true);
		plot.automatic_y_min = valueOr<bool>(gopts, node, "y_min_auto", true);
		plot.automatic_y_max = valueOr<bool>(gopts, node, "y_max_auto", true);
		plot.x_min = valueOr<double>(gopts, node, "x_min", 0.0);
		plot.x_max = valueOr<double>(gopts, node, "x_max", 1.0);
		plot.y_min = valueOr<double>(gopts, node, "y_min", 0.0);
		plot.y_max = valueOr<double>(gopts, node, "y_max", 1.0);
		plot.title = valueOr<std::string>(gopts, node, "title", "");
		plot.style = normalized(valueOr<std::string>(gopts, node, "style", "heatmap"));
		plot.scale = normalized(valueOr<std::string>(gopts, node, "scale", "linear"));
		result.plots[*position] = std::move(plot);
	}
	return result;
}

} // namespace ganalysis
