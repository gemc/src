#include "gSROFactory.h"

#include <gemc/gfactory/gfactory.h>
#include <gemc/gstreamer/gstreamer_options.h>

std::shared_ptr<GSROFactory> GSROFactory::from_options(const std::shared_ptr<GOptions>& options) {
	std::optional<GStreamerDefinition> selected;
	for (const auto& definition : gstreamer::getGStreamerDefinition(options)) {
		if (definition.format != "sro") { continue; }
		if (selected) { throw std::invalid_argument("Configure only one sro output per simulation"); }
		selected = definition;
	}
	if (!selected) { return nullptr; }
	GManager manager(options);
	auto implementation = manager.LoadAndRegisterObjectFromLibrary<GSROImplementation>(
		selected->implementation, options);
	return std::make_shared<GSROFactory>(std::move(implementation), selected->rootname);
}

GSROFactory::GSROFactory(std::shared_ptr<GSROImplementation> plugin, std::string output_basename)
	: implementation(std::move(plugin)), run_context{0, 0, std::move(output_basename)} {
	if (!implementation) { throw std::invalid_argument("SRO requires an implementation plugin"); }
}

void GSROFactory::begin_run(int run_id, GSROEventId event_count) {
	run_context.run_id = run_id;
	run_context.event_count = event_count;
	interrupted = false;
	auto config = implementation->configure_run(run_context);
	service.begin_run([plugin = implementation, context = run_context](GSROCrateId crate) {
		return plugin->create_crate(crate, context);
	}, std::move(config.timing), config.crate_limits, config.max_pending_events);
}

void GSROFactory::create_crate_thread_if_needed(GSROCrateId crate) {
	service.create_crate_thread_if_needed(crate);
}
void GSROFactory::dispatch_payload_to_crate(GSROPayload payload) {
	service.dispatch_payload_to_crate(std::move(payload));
}
void GSROFactory::complete_event(GSROEventId event) { service.complete_event(event); }
void GSROFactory::cancel_run(std::exception_ptr error) {
	interrupted = true;
	service.cancel_run(error);
}
void GSROFactory::finish_run(bool all_events_processed) {
	const auto reason = all_events_processed && !interrupted
		? GSROEndReason::completed : GSROEndReason::interrupted;
	service.finish_run({reason, std::nullopt});
}
void GSROFactory::rethrow_if_failed() const { service.rethrow_if_failed(); }
