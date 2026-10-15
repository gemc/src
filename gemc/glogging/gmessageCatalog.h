#pragma once

#include <array>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class GOptions;

/**
 * \brief Immutable, cached translations of complete English message templates.
 *
 * Catalogs use schema 1 and an English-template-to-translation mapping. Invalid files are ignored with
 * a diagnostic; missing messages retain their English text. Loading never changes the process locale.
 */
class GMessageCatalog {
public:
	/**
	 * \brief Resolve language and catalog paths from options/environment and reuse the cached catalog.
	 * \param options Optional configuration with language and plugin_path scalar options.
	 */
	[[nodiscard]] static std::shared_ptr<const GMessageCatalog> load(const GOptions* options = nullptr);

	/// Catalog search paths shared by logging and GUI translation startup.
	[[nodiscard]] static std::vector<std::filesystem::path> searchDirectories(const GOptions* options = nullptr);

	/// Returns a translated template, or the original view when no translation is available.
	[[nodiscard]] std::string_view translate(std::string_view english) const;

	/**
	 * \brief Translate and format a complete template without logging headers or verbosity filtering.
	 * Supports %1 through %99 and literal %%; inserted parameter text is never interpreted as a template.
	 * Invalid or unused parameter positions throw std::invalid_argument, matching GLogger formatting.
	 */
	template <typename... Args>
	[[nodiscard]] std::string format(std::string_view message, Args&&... args) const {
		static_assert(sizeof...(Args) <= 99, "Message templates support at most 99 parameters");
		message = translate(message);
		const std::array<std::string, sizeof...(Args)> parameters{
			parameter_string(std::forward<Args>(args))...
		};
		std::array<bool, sizeof...(Args)> used{};
		std::string result;
		result.reserve(message.size());

		for (std::size_t position = 0; position < message.size();) {
			const auto next = position + 1;
			if (message[position] != '%' || next == message.size()) {
				result += message[position++];
				continue;
			}
			if (message[next] == '%') {
				result += '%';
				position += 2;
				continue;
			}
			if (message[next] < '0' || message[next] > '9') {
				result += message[position++];
				continue;
			}
			if (message[next] == '0') {
				throw std::invalid_argument("Message placeholders must start with %1 through %99");
			}

			std::size_t index = 0;
			position = next;
			while (position < message.size() && message[position] >= '0' && message[position] <= '9') {
				index = index * 10 + static_cast<std::size_t>(message[position++] - '0');
				if (index > 99) throw std::invalid_argument("Message placeholder exceeds %99");
			}
			if (index > parameters.size()) {
				throw std::invalid_argument("Message placeholder has no corresponding parameter");
			}
			result += parameters[index - 1];
			used[index - 1] = true;
		}
		for (bool referenced : used) {
			if (!referenced) throw std::invalid_argument("Message template leaves a parameter unused");
		}
		return result;
	}

	/// Normalized requested language tag; a missing catalog still falls back to English per message.
	[[nodiscard]] const std::string& language() const { return language_; }

private:
	template <typename T>
	[[nodiscard]] static std::string parameter_string(T&& value) {
		std::ostringstream stream;
		stream << std::forward<T>(value);
		return stream.str();
	}

	explicit GMessageCatalog(std::string language, const std::vector<std::filesystem::path>& directories);
	std::string language_;
	std::map<std::string, std::string, std::less<>> messages_;
};
