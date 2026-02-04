#pragma once

// gstreamer
#include "gstreamer.h"
#include "gstreamerConventions.h"

// c++
#include <fstream>

/**
 * \file gstreamerJLABSROFactory.h
 * \brief JLAB SRO frame streamer plugin definitions.
 *
 * This plugin is specialized for producing binary frame records with a packed header (DataFrameHeader)
 * followed by payload words.
 */

#pragma pack(push, 1)
/**
 * \brief Packed frame header written at the beginning of each frame record.
 *
 * Packing  is required to ensure that the binary layout matches the expected
 * on-disk/on-wire format without compiler-inserted padding bytes.
 */
struct DataFrameHeader
{
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

/**
 * \class GstreamerJSROFactory
 * \brief JLAB SRO gstreamer plugin producing binary frame streams (\c ".ev" files).
 *
 * Output model:
 * - The plugin constructs a frame header and payload in a \c std::vector<unsigned int> buffer.
 * - The header and payload are written in two steps via the frame hook sequence:
 *   - publishFrameHeaderImpl() writes the packed header
 *   - publishPayloadImpl() writes the payload words
 *
 * Threading:
 * - Intended usage is one instance per worker thread (one output file per thread).
 * - The output file is owned as a raw \c std::ofstream* to match existing code; lifetime is managed
 *   in openConnection()/closeConnectionImpl().
 */
class GstreamerJSROFactory : public GStreamer
{
public:
	//	GstreamerJSROFactory() = default;

	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GStreamer::GStreamer;

private:
	// open and close the output media
	bool openConnection() override;
	bool closeConnectionImpl() override;

	// frame streams
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	// JLAB specific
	static inline std::uint64_t llswap(unsigned long long val) { return (val >> 32) | (val << 32); }

private:
	/// \brief Output stream pointer for the binary \c ".ev" file.
	std::ofstream* ofile = nullptr;

	/// \brief Buffer holding the current frame header and payload words.
	std::vector<unsigned int> frame_data{};

	/// \brief Return the output filename for this instance (\c ".ev").
	std::string filename() const override { return gstreamer_definitions.rootname + ".ev"; }
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
