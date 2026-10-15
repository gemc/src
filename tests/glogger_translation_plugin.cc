#include <gemc/glogging/glogger.h>

extern "C" void translation_probe(GLogger* logger) {
	logger->info("Using %1 threads.", 6);
}
