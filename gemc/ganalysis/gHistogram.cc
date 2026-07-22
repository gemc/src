#include "gHistogram.h"

// c++
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>

double GHistogramData::binWidth() const {
	return counts.empty() ? 0.0 : (x_max - x_min) / static_cast<double>(counts.size());
}

std::uint64_t GHistogramData::maximumCount() const {
	return counts.empty() ? 0 : *std::max_element(counts.begin(), counts.end());
}

GHistogramData makeHistogram(const std::vector<double>& values, std::size_t bins,
	                          std::optional<double> requested_min,
	                          std::optional<double> requested_max) {
	if (bins == 0) { throw std::invalid_argument("A histogram must have at least one bin"); }

	GHistogramData result;
	result.counts.assign(bins, 0);

	bool found_finite = false;
	double observed_min = 0.0;
	double observed_max = 1.0;
	for (double value : values) {
		if (!std::isfinite(value)) { continue; }
		if (!found_finite) {
			observed_min = value;
			observed_max = value;
			found_finite = true;
		}
		else {
			observed_min = std::min(observed_min, value);
			observed_max = std::max(observed_max, value);
		}
	}

	if (requested_min.has_value() && requested_max.has_value()) {
		result.x_min = *requested_min;
		result.x_max = *requested_max;
	}
	else if (requested_min.has_value()) {
		result.x_min = *requested_min;
		result.x_max = found_finite ? observed_max : result.x_min;
		if (result.x_max <= result.x_min) {
			result.x_max = result.x_min + std::max(std::abs(result.x_min) * 0.05, 0.5);
		}
	}
	else if (requested_max.has_value()) {
		result.x_max = *requested_max;
		result.x_min = found_finite ? observed_min : result.x_max;
		if (result.x_min >= result.x_max) {
			result.x_min = result.x_max - std::max(std::abs(result.x_max) * 0.05, 0.5);
		}
	}
	else {
		if (!found_finite) {
			result.x_min = 0.0;
			result.x_max = 1.0;
		}
		else {
			result.x_min = observed_min;
			result.x_max = observed_max;
		}
		if (result.x_min == result.x_max) {
			const double padding = std::max(std::abs(result.x_min) * 0.05, 0.5);
			result.x_min -= padding;
			result.x_max += padding;
		}
	}

	if (!std::isfinite(result.x_min) || !std::isfinite(result.x_max) ||
	    result.x_min >= result.x_max) {
		throw std::invalid_argument("Histogram x minimum must be finite and below x maximum");
	}

	const double width = result.binWidth();
	if (!std::isfinite(width) || width <= 0.0) {
		throw std::invalid_argument("Histogram x range is too narrow for the selected number of bins");
	}
	for (double value : values) {
		if (!std::isfinite(value)) { continue; }
		++result.finite_entries;
		if (value < result.x_min) {
			++result.underflow;
			continue;
		}
		if (value > result.x_max) {
			++result.overflow;
			continue;
		}

		const auto index = value == result.x_max
		                 ? bins - 1
		                 : static_cast<std::size_t>((value - result.x_min) / width);
		++result.counts[std::min(index, bins - 1)];
		++result.in_range_entries;
	}

	return result;
}

std::uint64_t GHistogram2DData::count(std::size_t x_bin, std::size_t y_bin) const {
	return counts.at(y_bin * bins + x_bin);
}

std::uint64_t GHistogram2DData::maximumCount() const {
	return counts.empty() ? 0 : *std::max_element(counts.begin(), counts.end());
}

namespace {

std::pair<double, double> histogramRange(const std::vector<double>& values,
	                                      std::optional<double> requested_min,
	                                      std::optional<double> requested_max) {
	const auto histogram = makeHistogram(values, 1, requested_min, requested_max);
	return {histogram.x_min, histogram.x_max};
}

} // namespace

GHistogram2DData makeHistogram2D(const std::vector<std::pair<double, double>>& values,
	                              std::size_t bins, std::optional<double> requested_x_min,
	                              std::optional<double> requested_x_max,
	                              std::optional<double> requested_y_min,
	                              std::optional<double> requested_y_max) {
	if (bins == 0) { throw std::invalid_argument("A histogram must have at least one bin"); }
	if (bins > 1000) {
		throw std::invalid_argument("A 2D histogram supports at most 1000 bins per axis");
	}

	std::vector<double> x_values;
	std::vector<double> y_values;
	x_values.reserve(values.size());
	y_values.reserve(values.size());
	for (const auto& [x, y] : values) {
		if (!std::isfinite(x) || !std::isfinite(y)) { continue; }
		x_values.push_back(x);
		y_values.push_back(y);
	}

	GHistogram2DData result;
	result.bins = bins;
	result.counts.assign(bins * bins, 0);
	std::tie(result.x_min, result.x_max) =
		histogramRange(x_values, requested_x_min, requested_x_max);
	std::tie(result.y_min, result.y_max) =
		histogramRange(y_values, requested_y_min, requested_y_max);

	const double x_width = (result.x_max - result.x_min) / static_cast<double>(bins);
	const double y_width = (result.y_max - result.y_min) / static_cast<double>(bins);
	for (const auto& [x, y] : values) {
		if (!std::isfinite(x) || !std::isfinite(y)) { continue; }
		++result.finite_entries;
		if (x < result.x_min || x > result.x_max || y < result.y_min || y > result.y_max) {
			++result.out_of_range;
			continue;
		}
		const std::size_t x_bin = x == result.x_max
		                          ? bins - 1
		                          : static_cast<std::size_t>((x - result.x_min) / x_width);
		const std::size_t y_bin = y == result.y_max
		                          ? bins - 1
		                          : static_cast<std::size_t>((y - result.y_min) / y_width);
		++result.counts[std::min(y_bin, bins - 1) * bins + std::min(x_bin, bins - 1)];
		++result.in_range_entries;
	}
	return result;
}
