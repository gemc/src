#include "gmessageCatalog.h"

#include <gemc/goptions/goptions.h>
#include <gemc/guts/gutilities.h>
#include <G4ios.hh>
#include <yaml-cpp/yaml.h>

#include <array>
#include <algorithm>
#include <cstdlib>
#include <mutex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>
#include "gloggingConfig.h"

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace {
namespace fs = std::filesystem;

std::string environment(const char* name) {
	const auto value = std::getenv(name);
	return value ? value : "";
}

std::string normalize_language(std::string value) {
	const auto suffix = value.find_first_of(".@");
	if (suffix != std::string::npos) value.resize(suffix);
	for (auto& character : value) {
		if (character == '_') character = '-';
		if (character >= 'A' && character <= 'Z') character += 'a' - 'A';
		if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
		      character == '-')) return "en";
	}
	if (value.empty() || value == "c" || value == "posix" || value.front() == '-' || value.back() == '-') {
		return "en";
	}
	return value;
}

std::string native_language() {
#if defined(__APPLE__)
	const auto languages = CFLocaleCopyPreferredLanguages();
	std::string result;
	if (languages) {
		if (CFArrayGetCount(languages) > 0) {
			const auto language = static_cast<CFStringRef>(CFArrayGetValueAtIndex(languages, 0));
			std::array<char, 256> buffer{};
			if (CFStringGetCString(language, buffer.data(), buffer.size(), kCFStringEncodingUTF8)) {
				result = buffer.data();
			}
		}
		CFRelease(languages);
	}
	return result;
#elif defined(_WIN32)
	std::array<wchar_t, LOCALE_NAME_MAX_LENGTH> buffer{};
	if (GetUserDefaultLocaleName(buffer.data(), buffer.size())) {
		std::string result;
		for (const auto character : buffer) {
			if (!character) break;
			result += static_cast<char>(character); // Windows locale tags are ASCII.
		}
		return result;
	}
#endif
	return "en";
}

std::string option(const GOptions* options, const char* name) {
	return options && options->doesOptionExist(name) ? options->getOptionalScalarString(name).value_or("") : "";
}

std::string requested_language(const GOptions* options) {
	const auto explicit_language = option(options, "language");
	if (!explicit_language.empty() && explicit_language != "auto") return normalize_language(explicit_language);
	const auto override = environment("GEMC_LANGUAGE");
	if (!override.empty() && override != "auto") return normalize_language(override);
	for (const auto name : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
		const auto value = environment(name);
		if (!value.empty()) return normalize_language(value);
	}
	static const auto system_language = normalize_language(native_language());
	return system_language;
}

std::vector<fs::path> path_list(const std::string& value) {
	std::vector<fs::path> result;
	std::istringstream input(value);
	std::string path;
	while (std::getline(input, path, ':')) {
		if (!path.empty()) result.emplace_back(path);
	}
	return result;
}

std::vector<fs::path> catalog_directories(const GOptions* options) {
	auto result = path_list(environment("GEMC_TRANSLATION_PATH"));
	try {
		const auto executable_dir = gutilities::executable_path().parent_path();
		const auto prefix = executable_dir.filename() == "bin" ? executable_dir.parent_path() : executable_dir;
		result.push_back(prefix / GEMC_CATALOG_DIRECTORY);
		// Source catalogs are used only for a recognized build tree, including non-default build directories.
		const auto build = fs::path(GEMC_CATALOG_BUILD_DIRECTORY).lexically_normal().generic_string();
		if (executable_dir.generic_string() == build || executable_dir.generic_string().starts_with(build + "/")) {
			result.emplace_back(GEMC_CATALOG_SOURCE_DIRECTORY);
		} else if (fs::exists(prefix / "build.ninja")) {
			result.push_back(prefix.parent_path() / "translations");
		}
	} catch (const std::exception&) {
		// An unavailable executable path must not prevent English diagnostics.
	}
	auto plugin_paths = path_list(option(options, "plugin_path") + ":" + environment("GEMC_PLUGIN_PATH"));
	for (const auto& directory : plugin_paths) {
		result.push_back(directory.parent_path() / GEMC_CATALOG_DIRECTORY);
		std::error_code error;
		if (fs::exists(directory / "build.ninja", error)) result.push_back(directory.parent_path() / "translations");
	}
	std::vector<fs::path> unique;
	for (const auto& directory : result) {
		std::error_code error;
		const auto absolute = fs::absolute(directory, error).lexically_normal();
		if (!error && std::find(unique.begin(), unique.end(), absolute) == unique.end()) unique.push_back(absolute);
	}
	return unique;
}

std::vector<fs::path> catalog_files(const fs::path& directory, const std::string& language) {
	std::vector<fs::path> result{directory / (language + ".yaml")};
	std::error_code error;
	const fs::directory_iterator entries(directory, error);
	if (!error) {
		for (auto iterator = entries; iterator != fs::directory_iterator{} && !error; iterator.increment(error)) {
			if (iterator->is_directory(error) && !error) result.push_back(iterator->path() / (language + ".yaml"));
		}
	}
	std::sort(result.begin() + 1, result.end());
	return result;
}

std::set<unsigned> placeholders(std::string_view message) {
	std::set<unsigned> result;
	for (std::size_t index = 0; index < message.size(); ++index) {
		if (message[index] != '%' || index + 1 == message.size()) continue;
		if (message[index + 1] == '%') { ++index; continue; }
		if (message[index + 1] < '0' || message[index + 1] > '9') continue;
		unsigned position = 0;
		if (message[index + 1] == '0') throw std::invalid_argument("placeholder starts with zero");
		while (index + 1 < message.size() && message[index + 1] >= '0' && message[index + 1] <= '9') {
			position = position * 10 + static_cast<unsigned>(message[++index] - '0');
			if (position > 99) throw std::invalid_argument("placeholder exceeds %99");
		}
		result.insert(position);
	}
	return result;
}

std::map<std::string, std::string, std::less<>> read_catalog(const fs::path& path,
                                                         const std::string& language) {
	const auto catalog = YAML::LoadFile(path.string());
	if (!catalog.IsMap()) throw std::invalid_argument("catalog is not a mapping");
	std::set<std::string> fields;
	for (const auto& field : catalog) {
		if (!fields.insert(field.first.as<std::string>()).second) throw std::invalid_argument("duplicate field");
	}
	if (catalog["schema"].as<int>() != 1 ||
	    normalize_language(catalog["language"].as<std::string>()) != language) {
		throw std::invalid_argument("catalog schema or language does not match");
	}
	const auto messages = catalog["messages"];
	if (!messages.IsMap()) throw std::invalid_argument("messages is not a mapping");
	std::map<std::string, std::string, std::less<>> result;
	for (const auto& entry : messages) {
		const auto english = entry.first.as<std::string>();
		const auto translation = entry.second.as<std::string>();
		if (english.find_first_not_of(" \t\r\n") == std::string::npos ||
		    translation.find_first_not_of(" \t\r\n") == std::string::npos ||
		    placeholders(english) != placeholders(translation)) {
			throw std::invalid_argument("empty message or changed placeholders");
		}
		if (!result.emplace(english, translation).second) throw std::invalid_argument("duplicate English template");
	}
	return result;
}
} // namespace

GMessageCatalog::GMessageCatalog(std::string language, const std::vector<fs::path>& directories)
	: language_(std::move(language)) {
	if (language_ == "en" || language_.starts_with("en-")) return;
	std::vector<std::string> variants{language_};
	const auto separator = language_.find('-');
	if (separator != std::string::npos) variants.push_back(language_.substr(0, separator));
	for (const auto& variant : variants) {
		for (const auto& directory : directories) {
			for (const auto& path : catalog_files(directory, variant)) {
				std::error_code error;
				if (!fs::is_regular_file(path, error) || error) continue;
				try {
					// First match wins: regional text precedes base-language text, then path precedence.
					const auto messages = read_catalog(path, variant);
					messages_.insert(messages.begin(), messages.end());
				} catch (const std::exception& error) {
					G4cerr << "Ignoring invalid translation catalog <" << path.string() << ">: "
					       << error.what() << G4endl;
				}
			}
		}
	}
}

std::shared_ptr<const GMessageCatalog> GMessageCatalog::load(const GOptions* options) {
	const auto language = requested_language(options);
	using Key = std::array<std::string, 4>;
	static std::mutex mutex;
	static std::map<Key, std::shared_ptr<const GMessageCatalog>> cache;
	const std::lock_guard lock(mutex);
	const Key key{language, environment("GEMC_TRANSLATION_PATH"), option(options, "plugin_path"),
	              environment("GEMC_PLUGIN_PATH")};
	const auto found = cache.find(key);
	if (found != cache.end()) return found->second;
	// Resolve filesystem paths only on a cache miss; loggers are also constructed in event data hot paths.
	const auto directories = language == "en" || language.starts_with("en-")
		? std::vector<fs::path>{} : catalog_directories(options);
	auto catalog = std::shared_ptr<const GMessageCatalog>(new GMessageCatalog(language, directories));
	cache.emplace(key, catalog);
	return catalog;
}

std::string_view GMessageCatalog::translate(std::string_view english) const {
	const auto found = messages_.find(english);
	return found != messages_.end() ? std::string_view(found->second) : english;
}

std::vector<std::filesystem::path> GMessageCatalog::searchDirectories(const GOptions* options) {
	return catalog_directories(options);
}
