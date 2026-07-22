#include "gAnalysisAccumulator.h"

// gemc
#include "event/gEventDataCollection.h"

// c++
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

namespace {

const GAnalysisSeries* find_series(const GAnalysisSeriesMap& data, int run_number,
	                                const std::string& detector, GAnalysisSource source,
	                                const std::string& variable) {
	const auto it = data.find(GAnalysisSeriesKey{run_number, detector, source, variable});
	return it == data.end() ? nullptr : &it->second;
}

bool check(bool condition, const std::string& message) {
	if (!condition) { std::cerr << "Analyzer accumulator validation failed: " << message << '\n'; }
	return condition;
}

} // namespace

int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());

	auto digitized = GDigitizedData::create(gopts);
	digitized->includeVariable("custom_integer", 7);
	digitized->includeVariable("custom_double", 2.5);

	auto true_information = GTrueInfoData::create(gopts);
	true_information->includeVariable("custom_true", 4.5);

	GAnalysisAccumulator accumulator;
	accumulator.beginBeamOn(false);
	accumulator.setCurrentRunNumber(11);
	bool success = true;
	success &= check(accumulator.currentRunNumber() == 11,
	                 "current simulation run number was not published consistently");

	constexpr int number_of_shards = 8;
	std::vector<GAnalysisShard> shards(static_cast<std::size_t>(number_of_shards));
	for (auto& shard : shards) {
		shard.recordDigitized(11, "ctof", *digitized);
		shard.recordTrueInformation(11, "ctof", *true_information);
	}

	std::vector<std::thread> workers;
	workers.reserve(shards.size());
	for (auto& shard : shards) {
		workers.emplace_back([&accumulator, shard = std::move(shard)]() mutable {
			accumulator.merge(std::move(shard));
		});
	}
	for (auto& worker : workers) { worker.join(); }

	auto data = accumulator.snapshot();
	const auto* integer_series = find_series(data, 11, "ctof", GAnalysisSource::digitized,
	                                         "custom_integer");
	const auto* double_series = find_series(data, 11, "ctof", GAnalysisSource::digitized,
	                                        "custom_double");
	const auto* sro_series = find_series(data, 11, "ctof", GAnalysisSource::digitized, "crate");
	const auto* true_series = find_series(data, 11, "ctof", GAnalysisSource::true_information,
	                                      "custom_true");

	success &= check(integer_series && integer_series->size() == number_of_shards,
	                 "integer variables were not merged from every shard");
	success &= check(integer_series && integer_series->type() == GAnalysisNumericType::integer,
	                 "integer variable type was not preserved");
	success &= check(double_series && double_series->size() == number_of_shards,
	                 "double variables were not merged from every shard");
	const auto matched_values = integer_series != nullptr && double_series != nullptr
	                            ? matchAnalysisSeries(*integer_series, *double_series)
	                            : std::vector<std::pair<double, double>>{};
	success &= check(matched_values.size() == number_of_shards,
	                 "same-record X/Y values were not preserved across concurrent shard merges");
	success &= check(std::all_of(matched_values.begin(), matched_values.end(), [](const auto& values) {
		return values.first == 7.0 && values.second == 2.5;
	}), "concurrent shard merging paired unrelated X/Y values");
	success &= check(sro_series && sro_series->size() == number_of_shards,
	                 "conventional SRO variables were not discovered");
	success &= check(true_series && true_series->size() == number_of_shards,
	                 "true-information variables were not merged from every shard");

	accumulator.beginBeamOn(true);
	GAnalysisShard accumulated_shard;
	accumulated_shard.recordDigitized(11, "ctof", *digitized);
	accumulator.merge(std::move(accumulated_shard));
	data = accumulator.snapshot();
	integer_series = find_series(data, 11, "ctof", GAnalysisSource::digitized, "custom_integer");
	success &= check(integer_series && integer_series->size() == number_of_shards + 1,
	                 "Accumulate did not preserve samples from the preceding beamOn");

	auto mixed = GDigitizedData::create(gopts);
	mixed->includeVariable("mixed_numeric", 3);
	mixed->includeVariable("mixed_numeric", 3.5);
	GAnalysisShard mixed_shard;
	mixed_shard.recordDigitized(11, "ctof", *mixed);
	accumulator.merge(std::move(mixed_shard));
	data = accumulator.snapshot();
	const auto* mixed_series = find_series(data, 11, "ctof", GAnalysisSource::digitized,
	                                       "mixed_numeric");
	success &= check(mixed_series && mixed_series->size() == 2,
	                 "same-name integer and double samples were not retained");
	success &= check(mixed_series && mixed_series->type() == GAnalysisNumericType::mixed,
	                 "same-name integer and double samples were not marked mixed");

	accumulator.beginBeamOn(false);
	success &= check(accumulator.currentRunNumber() == -1,
	                 "starting a beamOn interval did not reset the current run number");
	GAnalysisShard replacement_shard;
	replacement_shard.recordDigitized(22, "ec", *digitized);
	accumulator.merge(std::move(replacement_shard));
	data = accumulator.snapshot();
	success &= check(find_series(data, 11, "ctof", GAnalysisSource::digitized,
	                            "custom_integer") == nullptr,
	                 "non-accumulating beamOn did not clear earlier samples");
	success &= check(find_series(data, 22, "ec", GAnalysisSource::digitized,
	                            "custom_integer") != nullptr,
	                 "replacement beamOn samples were not stored");
	success &= check(accumulator.beamOnCount() == 3, "beamOn interval count is incorrect");

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
