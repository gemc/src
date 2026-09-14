#pragma once

#include <gemc/gstreamer/sro/gSROImplementation.h>
#include <gemc/gstreamer/sro/gSROService.h>

#include <atomic>

/**
 * Generic sro output factory, upcoming in the next release. Shared by master and worker actions.
 * The run owner calls begin_run before workers and finish_run after workers. Each worker dispatches
 * synchronously through its event callback; no hits, callbacks, or mutable digitizer state may escape it.
 * Ordinary event streamers do not load this factory or produce per-worker SRO files.
 */
class GSROFactory {
public:
	/// Return null when SRO is disabled. Load the selected implementation before workers are created.
	static std::shared_ptr<GSROFactory> from_options(const std::shared_ptr<GOptions>& options);

	GSROFactory(std::shared_ptr<GSROImplementation> implementation, std::string output_basename);
	void begin_run(int run_id, GSROEventId event_count);
	void create_crate_thread_if_needed(GSROCrateId crate);
	void dispatch_payload_to_crate(GSROPayload payload);
	void complete_event(GSROEventId event);
	void interrupt_run() { interrupted = true; }
	void cancel_run(std::exception_ptr error);
	void finish_run(bool all_events_processed);
	void rethrow_if_failed() const;
	GSROEventContext event_context(GSROEventId event) const { return {run_context.run_id, event}; }

private:
	// Declaration order keeps implementation code loaded until service/timing/data destruction finishes.
	std::shared_ptr<GSROImplementation> implementation;
	GSRORunContext run_context;
	std::atomic<bool> interrupted{false};
	GSROService service;
};
