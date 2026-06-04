#pragma once

// gstreamer
#include "gstreamer.h"
#include "gstreamerConventions.h"

// c++
#include <fstream>

/**
 * \file gstreamerJLABSROFactory.h
 * \brief JLAB SRO binary frame streamer declarations.
 * \ingroup gstreamer_plugin_jlabsro_api
 */

#pragma pack(push, 1)
/**
 * \struct DataFrameHeader
 * \ingroup gstreamer_plugin_jlabsro_api
 * \brief Packed binary frame header written ahead of each JLAB SRO payload.
 *
 * This structure mirrors the exact binary layout expected by the target format. Packing is used so
 * the compiler does not insert alignment padding between fields.
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
 * \ingroup gstreamer_plugin_jlabsro_api
 * \brief JLAB SRO plugin producing packed binary frame streams in \c ".ev" files.
 *
 * This plugin is specialized for frame-stream output. It builds the current frame record into an
 * in-memory word buffer containing:
 * - one packed \ref DataFrameHeader
 * - one payload section assembled from the incoming integral payload data
 *
 * The buffer is then written in stages through the frame publish sequence.
 *
 * Threading model:
 * - one instance per worker thread is the intended usage
 * - the output stream is managed manually through a raw pointer to match the current implementation
 */
class GstreamerJSROFactory : public GStreamer
{
public:
	/// \brief Inherit the constructor taking the parsed options container.
	using GStreamer::GStreamer;

private:
	/**
	 * \brief Open the binary \c ".ev" output file.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool openConnection() override;

	/**
	 * \brief Close the binary output file.
	 *
	 * \return \c true on success, \c false otherwise.
	 */
	bool closeConnectionImpl() override;

	/**
	 * \brief Begin assembly of one binary frame record.
	 *
	 * The implementation prepares the packed header and assembles the frame payload words derived
	 * from the incoming integral payload vectors.
	 *
	 * \param frameRunData Frame collection being serialized.
	 * \return \c true on success, \c false otherwise.
	 */
	bool startStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief End one binary frame record.
	 *
	 * \param frameRunData Frame collection being serialized.
	 * \return \c true on success, \c false otherwise.
	 */
	bool endStreamImpl(const GFrameDataCollection* frameRunData) override;

	/**
	 * \brief Write the packed frame header portion of the current frame record.
	 *
	 * \param gframeHeader Frame header associated with the current record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishFrameHeaderImpl(const GFrameHeader* gframeHeader) override;

	/**
	 * \brief Write the payload portion of the current frame record.
	 *
	 * \param payload Integral payload collection associated with the current record.
	 * \return \c true on success, \c false otherwise.
	 */
	bool publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) override;

	/**
	 * \brief Swap the two 32-bit halves of a 64-bit value.
	 *
	 * This helper is used while assembling packed header fields in the current implementation.
	 *
	 * \param val Input 64-bit value.
	 * \return Reordered 64-bit value.
	 */
	static inline std::uint64_t llswap(unsigned long long val) { return (val >> 32) | (val << 32); }

private:
	/// \brief Binary output stream pointer for the current \c ".ev" file.
	std::ofstream* ofile = nullptr;

	/// \brief Word buffer containing the packed header followed by the current frame payload.
	std::vector<unsigned int> frame_data{};

	/**
	 * \brief Return the final binary output filename for this plugin instance.
	 *
	 * \return Base output name plus the \c ".ev" extension.
	 */
	std::string filename() const override { return gstreamer_definitions.rootname + ".ev"; }
};