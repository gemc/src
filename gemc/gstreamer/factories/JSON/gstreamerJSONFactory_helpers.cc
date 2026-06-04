// gstreamer
#include "gstreamerJSONFactory.h"

// Implementation summary:
// Minimal helper used by the JSON plugin to escape keys and string values
// without relying on an external JSON library.

std::string GstreamerJsonFactory::jsonEscape(const std::string& s) {
	std::string out;
	out.reserve(s.size() + 8);

	for (unsigned char c : s) {
		switch (c) {
		case '\\': out += "\\\\";
			break;
		case '"': out += "\\\"";
			break;
		case '\b': out += "\\b";
			break;
		case '\f': out += "\\f";
			break;
		case '\n': out += "\\n";
			break;
		case '\r': out += "\\r";
			break;
		case '\t': out += "\\t";
			break;
		default:
			// JSON requires control characters below 0x20 to be escaped.
			if (c < 0x20) {
				static const char* hex = "0123456789abcdef";
				out                    += "\\u00";
				out                    += hex[(c >> 4) & 0xF];
				out                    += hex[c & 0xF];
			}
			else {
				out.push_back(static_cast<char>(c));
			}
		}
	}
	return out;
}