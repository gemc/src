#pragma once

// gemc
#include <gemc/gdata/gDigitizedData.h>
#include <gemc/gdata/gTrueInfoData.h>

// c++
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/**
 * \file gAnalysisAccumulator.h
 * \brief Declares the non-Qt runtime-data accumulator used by the GEMC Analyzer.
 */

/** \brief Identifies which GEMC data record supplied an Analyzer variable. */
enum class GAnalysisSource {
	true_information,
	digitized
};

/** \brief Records the scalar representation observed for an Analyzer variable. */
enum class GAnalysisNumericType {
	integer,
	floating_point,
	mixed
};

/**
 * \brief Uniquely identifies one numeric series collected by the Analyzer.
 *
 * Repeated beamOn calls using the same run number append to the same key when accumulation is enabled.
 */
struct GAnalysisSeriesKey {
	int                 run_number = 0;
	std::string         detector;
	GAnalysisSource     source = GAnalysisSource::true_information;
	std::string         variable;

	[[nodiscard]] bool operator<(const GAnalysisSeriesKey& other) const {
		return std::tie(run_number, detector, source, variable) <
		       std::tie(other.run_number, other.detector, other.source, other.variable);
	}
};

/** \brief Numeric samples and discovered type information for one Analyzer series. */
class GAnalysisSeries
{
public:
	explicit GAnalysisSeries(GAnalysisNumericType type = GAnalysisNumericType::floating_point) :
		numeric_type(type) {
	}

	[[nodiscard]] GAnalysisNumericType type() const { return numeric_type; }
	[[nodiscard]] const std::vector<double>& values() const { return samples; }
	[[nodiscard]] const std::vector<std::uint64_t>& sampleIds() const { return sample_ids; }
	[[nodiscard]] std::size_t size() const { return samples.size(); }

private:
	GAnalysisNumericType       numeric_type;
	std::vector<double>        samples;
	std::vector<std::uint64_t> sample_ids;

	void add(GAnalysisNumericType type, double value, std::uint64_t sample_id);
	void merge(GAnalysisSeries&& other);

	friend class GAnalysisShard;
	friend class GAnalysisAccumulator;
};

using GAnalysisSeriesMap = std::map<GAnalysisSeriesKey, GAnalysisSeries>;

/** \brief Match two series by their originating detector-record identifier. */
[[nodiscard]] std::vector<std::pair<double, double>> matchAnalysisSeries(
	const GAnalysisSeries& x_series, const GAnalysisSeries& y_series);

/**
 * \brief Thread-confined staging area for Analyzer samples.
 *
 * Each Geant4 execution thread owns one shard and writes to it without synchronization. Completed shards are
 * moved into GAnalysisAccumulator, where the cross-thread merge is protected. A shard must not be written by
 * more than one thread at a time.
 */
class GAnalysisShard
{
public:
	/** \brief Discover and store all numeric true-information variables in one record. */
	void recordTrueInformation(int run_number, const std::string& detector, const GTrueInfoData& data);

	/** \brief Discover and store all scalar numeric digitized variables in one record. */
	void recordDigitized(int run_number, const std::string& detector, const GDigitizedData& data);

	[[nodiscard]] bool empty() const { return series.empty(); }
	[[nodiscard]] const GAnalysisSeriesMap& data() const { return series; }
	void clear() { series.clear(); }

private:
	GAnalysisSeriesMap series;

	void record(int run_number, const std::string& detector, GAnalysisSource source,
	            const std::string& variable, GAnalysisNumericType type, double value,
	            std::uint64_t sample_id);

	friend class GAnalysisAccumulator;
};

/**
 * \brief Thread-safe persistent store for completed Analyzer shards.
 *
 * beginBeamOn() applies the GUI's Accumulate selection before event processing starts. merge() may then be
 * called concurrently by worker threads. snapshot() returns an immutable copy suitable for GUI-side
 * histogramming after beamOn has returned.
 */
class GAnalysisAccumulator
{
public:
	/**
	 * \brief Start a beamOn collection interval.
	 * \param accumulate Preserve earlier samples when true; clear them before this interval when false.
	 */
	void beginBeamOn(bool accumulate);

	/** \brief Publish the simulation run number that the next Geant4 run will process. */
	void setCurrentRunNumber(int run_number);

	/** \brief Return the simulation run number published for the current Geant4 run. */
	[[nodiscard]] int currentRunNumber() const;

	/** \brief Merge one completed thread-local shard into the persistent dataset. */
	void merge(GAnalysisShard shard);

	/** \brief Remove all persisted Analyzer samples. */
	void clear();

	/** \brief Return a consistent copy of all currently persisted series. */
	[[nodiscard]] GAnalysisSeriesMap snapshot() const;

	/** \brief Return the number of beamOn intervals started since the accumulator was created or cleared. */
	[[nodiscard]] std::uint64_t beamOnCount() const;

private:
	mutable std::mutex mutex;
	GAnalysisSeriesMap series;
	std::uint64_t      beam_on_count = 0;
	int                current_run_number = -1;
};

/** \brief Create the Analyzer service only for GUI execution; return null for batch execution. */
[[nodiscard]] std::shared_ptr<GAnalysisAccumulator> makeAnalysisAccumulator(bool gui_mode);
