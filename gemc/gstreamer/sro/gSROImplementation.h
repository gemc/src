#pragma once

#include "gSROCrate.h"

#include <gemc/gbase/gbase.h>
#include <gemc/gfactory/gdl.h>

/// Upcoming in the next release. Implementation-owned timing and output for one SRO invocation.
struct GSRORunContext {
	int run_id;
	GSROEventId event_count;
	std::string output_basename;
};

struct GSROConfiguration {
	std::shared_ptr<const GSROTiming> timing;
	GSROCrateLimits crate_limits;
	std::size_t max_pending_events = 65536;
};

/**
 * Implementation plugin loaded by the sro factory, for example from clas12-systems.
 * Export GSROImplementationFactory(const std::shared_ptr<GOptions>&) returning a new derived object.
 * configure_run runs on the run owner before workers start. create_crate runs concurrently on crate
 * threads and must return a sink and plugin owned by that thread. Use the run context to give each
 * crate a distinct output file and each run invocation a distinct name. The framework supplies no
 * frame size, binary layout, electronics model, or partial-frame policy.
 * The factory retains this object's library until timing, payloads, plugins, and sinks are destroyed.
 */
class GSROImplementation : public GBase<GSROImplementation> {
public:
	explicit GSROImplementation(const std::shared_ptr<GOptions>& opts)
		: GBase(opts, "gstreamer"), options(opts) {}
	~GSROImplementation() override = default;

	/// GManager calls this after loading an implementation, following the standard GEMC plugin contract.
	void set_loggers(const std::shared_ptr<GOptions>& opts) {
		options = opts;
		log = std::make_shared<GLogger>(opts, "GSROImplementation", "gstreamer");
	}

	virtual GSROConfiguration configure_run(const GSRORunContext& run) = 0;
	virtual GSROCrateResources create_crate(GSROCrateId crate, const GSRORunContext& run) const = 0;

	static GSROImplementation* instantiate(dlhandle handle, const std::shared_ptr<GOptions>& opts) {
		using Factory = GSROImplementation* (*)(const std::shared_ptr<GOptions>&);
		auto factory = reinterpret_cast<Factory>(dlsym(handle, "GSROImplementationFactory"));
		if (!factory) { throw std::runtime_error("SRO plugin is missing GSROImplementationFactory"); }
		return factory(opts);
	}

protected:
	std::shared_ptr<GOptions> options;
};
