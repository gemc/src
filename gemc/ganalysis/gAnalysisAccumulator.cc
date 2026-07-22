#include "gAnalysisAccumulator.h"

// c++
#include <atomic>
#include <iterator>
#include <map>
#include <utility>

namespace {

GAnalysisNumericType merge_numeric_types(GAnalysisNumericType left, GAnalysisNumericType right) {
	if (left == right) { return left; }
	return GAnalysisNumericType::mixed;
}

std::atomic<std::uint64_t> next_sample_id{0};

} // namespace

void GAnalysisSeries::add(GAnalysisNumericType type, double value, std::uint64_t sample_id) {
	numeric_type = merge_numeric_types(numeric_type, type);
	samples.push_back(value);
	sample_ids.push_back(sample_id);
}

void GAnalysisSeries::merge(GAnalysisSeries&& other) {
	numeric_type = merge_numeric_types(numeric_type, other.numeric_type);
	samples.insert(samples.end(), std::make_move_iterator(other.samples.begin()),
	               std::make_move_iterator(other.samples.end()));
	sample_ids.insert(sample_ids.end(), std::make_move_iterator(other.sample_ids.begin()),
	                  std::make_move_iterator(other.sample_ids.end()));
}

void GAnalysisShard::record(int run_number, const std::string& detector, GAnalysisSource source,
	                         const std::string& variable, GAnalysisNumericType type, double value,
	                         std::uint64_t sample_id) {
	GAnalysisSeriesKey key{run_number, detector, source, variable};
	auto [it, inserted] = series.try_emplace(std::move(key), type);
	if (inserted) {
		it->second.samples.push_back(value);
		it->second.sample_ids.push_back(sample_id);
	}
	else {
		it->second.add(type, value, sample_id);
	}
}

void GAnalysisShard::recordTrueInformation(int run_number, const std::string& detector,
	                                        const GTrueInfoData& data) {
	const std::uint64_t sample_id = next_sample_id.fetch_add(1, std::memory_order_relaxed);
	for (const auto& [variable, value] : data.getDoubleVariablesMap()) {
		record(run_number, detector, GAnalysisSource::true_information, variable,
		       GAnalysisNumericType::floating_point, value, sample_id);
	}
}

void GAnalysisShard::recordDigitized(int run_number, const std::string& detector,
	                                  const GDigitizedData& data) {
	const std::uint64_t sample_id = next_sample_id.fetch_add(1, std::memory_order_relaxed);
	// A selector other than 0 or 1 returns every scalar observable, including conventional SRO fields.
	for (const auto& [variable, value] : data.getIntObservablesMap(-1)) {
		record(run_number, detector, GAnalysisSource::digitized, variable,
		       GAnalysisNumericType::integer, static_cast<double>(value), sample_id);
	}
	for (const auto& [variable, value] : data.getDblObservablesMap(-1)) {
		record(run_number, detector, GAnalysisSource::digitized, variable,
		       GAnalysisNumericType::floating_point, value, sample_id);
	}
}

std::vector<std::pair<double, double>> matchAnalysisSeries(
	const GAnalysisSeries& x_series, const GAnalysisSeries& y_series) {
	std::multimap<std::uint64_t, double> x_by_sample;
	for (std::size_t index = 0; index < x_series.size(); ++index) {
		x_by_sample.emplace(x_series.sampleIds()[index], x_series.values()[index]);
	}

	std::vector<std::pair<double, double>> matched;
	matched.reserve(std::min(x_series.size(), y_series.size()));
	for (std::size_t index = 0; index < y_series.size(); ++index) {
		const auto found = x_by_sample.find(y_series.sampleIds()[index]);
		if (found == x_by_sample.end()) { continue; }
		matched.emplace_back(found->second, y_series.values()[index]);
		x_by_sample.erase(found);
	}
	return matched;
}

void GAnalysisAccumulator::beginBeamOn(bool accumulate) {
	std::scoped_lock lock(mutex);
	if (!accumulate) { series.clear(); }
	current_run_number = -1;
	++beam_on_count;
}

void GAnalysisAccumulator::setCurrentRunNumber(int run_number) {
	std::scoped_lock lock(mutex);
	current_run_number = run_number;
}

int GAnalysisAccumulator::currentRunNumber() const {
	std::scoped_lock lock(mutex);
	return current_run_number;
}

void GAnalysisAccumulator::merge(GAnalysisShard shard) {
	std::scoped_lock lock(mutex);

	for (auto& [key, incoming] : shard.series) {
		auto [it, inserted] = series.try_emplace(key, incoming.type());
		if (inserted) {
			it->second = std::move(incoming);
		}
		else {
			it->second.merge(std::move(incoming));
		}
	}
}

void GAnalysisAccumulator::clear() {
	std::scoped_lock lock(mutex);
	series.clear();
	beam_on_count = 0;
	current_run_number = -1;
}

GAnalysisSeriesMap GAnalysisAccumulator::snapshot() const {
	std::scoped_lock lock(mutex);
	return series;
}

std::uint64_t GAnalysisAccumulator::beamOnCount() const {
	std::scoped_lock lock(mutex);
	return beam_on_count;
}

std::shared_ptr<GAnalysisAccumulator> makeAnalysisAccumulator(bool gui_mode) {
	return gui_mode ? std::make_shared<GAnalysisAccumulator>() : nullptr;
}
