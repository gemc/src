#include <gemc/glogging/glogger.h>

#include <array>
#include <fstream>
#include <iostream>
#include <thread>
#include <dlfcn.h>

int main(int argc, char* argv[]) {
	GOptions schema("translations");
	schema.defineOption(GVariable("test_mode", "messages", "translation test mode"), "");
	schema.defineOption(GVariable("plugin_path", "", "plugin directories for catalog discovery"), "");
	schema.defineOption(GVariable("rewrite", "", "scratch catalog directory to rewrite after loading"), "");
	schema.defineOption(GVariable("probe_plugin", "", "test plugin to load"), "");
	auto options = std::make_shared<GOptions>(argc, argv, schema);
	GLogger logger(options, "test", "translations");
	std::cout << "LANGUAGE=" << logger.get_language() << '\n';
	const auto mode = options->getRequiredScalarString("test_mode");
	if (mode == "error") logger.error(42, "Cannot read file %1.", "missing%2.txt");
	if (mode == "plugin") {
		const auto handle = dlopen(options->getRequiredScalarString("probe_plugin").c_str(), RTLD_NOW);
		if (!handle) { std::cerr << dlerror() << '\n'; return 1; }
		const auto probe = reinterpret_cast<void (*)(GLogger*)>(dlsym(handle, "translation_probe"));
		if (!probe) { dlclose(handle); return 1; }
		probe(&logger);
		dlclose(handle);
	}
	if (mode == "cache") {
		const auto first = GMessageCatalog::load(options.get());
		const auto rewrite = options->getRequiredScalarString("rewrite");
		if (!rewrite.empty()) {
			std::ofstream file(std::filesystem::path(rewrite) / "it.yaml");
			file << "schema: 1\nlanguage: it\nmessages:\n  'Using %1 threads.': 'Changed %1'\n";
		}
		std::array<bool, 8> correct{};
		std::array<std::thread, 8> workers;
		for (std::size_t index = 0; index < workers.size(); ++index) {
			workers[index] = std::thread([&, index] {
				const auto second = GMessageCatalog::load(options.get());
				correct[index] = first == second && second->translate("Using %1 threads.") == "Uso di %1 thread.";
			});
		}
		for (auto& worker : workers) worker.join();
		for (const auto result : correct) {
			if (!result) return 1;
		}
	}
	logger.info("Using %1 threads.", 4);
	logger.info("First %1, second %2.", "literal %2 and %%", 8);
	logger.warning("File %1 was not found; using %2.", "missing%2.txt", "fallback.txt");
	logger.critical("Starting detector %1.", "dc");
	logger.debug(NORMAL, "Loaded %1 entries from %2.", 10, "dc%1.txt");
	logger.info(2, "Verbose %1.", 9);
	logger.info("Untranslated %1.", 1.25);
	logger.info("%1", "External diagnostic: %2 and %%");
	return 0;
}
