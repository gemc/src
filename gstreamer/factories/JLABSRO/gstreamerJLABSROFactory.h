#pragma once

// gstreamer
#include "gstreamer.h"

// c++
#include <fstream>

#pragma pack(push, 1)
struct DataFrameHeader {
	uint32_t source_id;
	uint32_t total_length;
	uint32_t payload_length;
	uint32_t compressed_length;
	uint32_t magic;
	uint32_t format_version;
	uint32_t flags;
	uint64_t record_counter;
	uint64_t ts_sec;
	uint64_t ts_nsec;
};
#pragma pack(pop)

class GstreamerJSROFactory : public GStreamer {
public:
	GstreamerJSROFactory() = default;

private:
	// open and close the output media
	bool openConnection(const std::shared_ptr<GLogger>& log) override;
	bool closeConnection(const std::shared_ptr<GLogger>& log) override;

	// frame streams
	bool startStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool endStream(const GFrameDataCollection* frameRunData, const std::shared_ptr<GLogger>& log) override;
	bool publishFrameHeader(const GFrameDataCollectionHeader* gframeHeader, const std::shared_ptr<GLogger>& log) override;
	bool publishPayload(const std::vector<GIntegralPayload*>* payload, const std::shared_ptr<GLogger>& log) override;

	// JLAB specific
	static inline std::uint64_t llswap(unsigned long long val) { return (val >> 32) | (val << 32); }

private:
	std::ofstream*       ofile = nullptr;
	std::vector<unsigned int> frame_data{};
};


/**
 
 Note on Pragma Pack
 
 
 #pragma pack instructs the compiler to pack structure members with particular alignment.
 Most compilers, when you declare a struct, will insert padding between members to ensure that
 they are aligned to appropriate addresses in memory (usually a multiple of the type's size).
 This avoids the performance penalty (or outright error) on some architectures associated
 with accessing variables that are not aligned properly.
 
 For example, given 4-byte integers and the following struct:
 
 struct Test
 {
 char AA;
 int BB;
 char CC;
 };
 The compiler could choose to lay the struct out in memory like this:
 
 |   1        |   2       |   3      |   4     |
 
 | AA(1)   |  pad......................... |
 | BB(1)   | BB(2) | BB(3) | BB(4) |
 | CC(1)   | pad.......................... |
 
 and sizeof(Test) would be 4 × 3 = 12, even though it only contains 6 bytes of data.
 The most common use case for the #pragma (to my knowledge) is when working with hardware devices
 where you need to ensure that the compiler does not insert padding into the data and each member
 follows the previous one.
 
 With #pragma pack(1), the struct above would be laid out like this:
 
 |     1    |
 
 | AA(1) |
 | BB(1) |
 | BB(2) |
 | BB(3) |
 | BB(4) |
 | CC(1) |
 
 */
