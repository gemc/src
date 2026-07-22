#pragma once

// c++
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

/** \brief Binned representation of one numeric Analyzer series. */
struct GHistogramData {
	double                     x_min = 0.0;
	double                     x_max = 1.0;
	std::vector<std::uint64_t> counts;
	std::size_t                finite_entries = 0;
	std::size_t                in_range_entries = 0;
	std::size_t                underflow = 0;
	std::size_t                overflow = 0;

	[[nodiscard]] double binWidth() const;
	[[nodiscard]] std::uint64_t maximumCount() const;
};

/**
 * \brief Bin numeric samples using either an automatic or explicit x range.
 *
 * Each range endpoint may be supplied independently. An omitted endpoint is derived from finite samples and
 * expanded when necessary to remain on the valid side of a fixed endpoint. Values outside the resulting
 * range are recorded as under/overflow; a sample exactly equal to x_max belongs to the final bin.
 */
[[nodiscard]] GHistogramData makeHistogram(
	const std::vector<double>& values,
	std::size_t bins,
	std::optional<double> x_min = std::nullopt,
	std::optional<double> x_max = std::nullopt);

/** \brief Binned representation of paired X and Y Analyzer samples. */
struct GHistogram2DData {
	double                     x_min = 0.0;
	double                     x_max = 1.0;
	double                     y_min = 0.0;
	double                     y_max = 1.0;
	std::size_t                bins = 0;
	std::vector<std::uint64_t> counts;
	std::size_t                finite_entries = 0;
	std::size_t                in_range_entries = 0;
	std::size_t                out_of_range = 0;

	[[nodiscard]] std::uint64_t count(std::size_t x_bin, std::size_t y_bin) const;
	[[nodiscard]] std::uint64_t maximumCount() const;
};

/** \brief Bin paired samples into a square 2D histogram. */
[[nodiscard]] GHistogram2DData makeHistogram2D(
	const std::vector<std::pair<double, double>>& values,
	std::size_t bins,
	std::optional<double> x_min = std::nullopt,
	std::optional<double> x_max = std::nullopt,
	std::optional<double> y_min = std::nullopt,
	std::optional<double> y_max = std::nullopt);
