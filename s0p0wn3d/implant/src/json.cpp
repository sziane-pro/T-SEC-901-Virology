#include "json.h"
#include <cstdio>
#include <cstdlib>
#include <string>

std::string json_escape(const std::string& value) {
	std::string output;
	output.reserve(value.size() + 8);

	for (const char c : value) {
		switch (c) {
		case '\"': output += "\\\""; break;
		case '\\': output += "\\\\"; break;
		case '\b': output += "\\b";  break;
		case '\f': output += "\\f";  break;
		case '\n': output += "\\n";  break;
		case '\r': output += "\\r";  break;
		case '\t': output += "\\t";  break;
		default:
			if ('\x00' <= c && c <= '\x1f') {
				char buffer[7];
				snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned char>(c));
				output += buffer;
			}
			else {
				output += c;
			}
		}
	}
	return output;
}

namespace {

// Ajoute un point de code Unicode a `out`, encode en UTF-8.
void append_utf8(std::string& out, unsigned long code) {
	if (code <= 0x7F) {
		out += static_cast<char>(code);
	}
	else if (code <= 0x7FF) {
		out += static_cast<char>(0xC0 | (code >> 6));
		out += static_cast<char>(0x80 | (code & 0x3F));
	}
	else if (code <= 0xFFFF) {
		out += static_cast<char>(0xE0 | (code >> 12));
		out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (code & 0x3F));
	}
	else {
		out += static_cast<char>(0xF0 | (code >> 18));
		out += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
		out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
		out += static_cast<char>(0x80 | (code & 0x3F));
	}
}

bool parse_hex4(const std::string& json, size_t at, unsigned long& code) {
	if (at + 4 > json.size()) {
		return false;
	}
	const std::string hex = json.substr(at, 4);
	char* end = nullptr;
	code = strtoul(hex.c_str(), &end, 16);
	return end == hex.c_str() + 4;
}

}  // namespace

std::optional<std::string> json_get_string(const std::string& json,
                                           const std::string& key) {
	const std::string pattern = "\"" + key + "\"";

	size_t pos = json.find(pattern);
	if (pos == std::string::npos) {
		return std::nullopt;
	}

	pos = json.find(':', pos + pattern.size());
	if (pos == std::string::npos) {
		return std::nullopt;
	}

	++pos;
	while (pos < json.size() &&
		(json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n')) {
		++pos;
	}
	if (pos >= json.size() || json[pos] != '"') {
		return std::nullopt;
	}
	++pos;

	std::string value;
	for (; pos < json.size(); ++pos) {
		const char c = json[pos];

		if (c == '\\') {
			if (pos + 1 >= json.size()) {
				return std::nullopt;
			}
			const char next = json[++pos];
			switch (next) {
			case 'n': value += '\n'; break;
			case 'r': value += '\r'; break;
			case 't': value += '\t'; break;
			case 'b': value += '\b'; break;
			case 'f': value += '\f'; break;
			case '"': value += '"';  break;
			case '\\': value += '\\'; break;
			case '/': value += '/';  break;
			case 'u': {
				unsigned long code = 0;
				if (!parse_hex4(json, pos + 1, code)) {
					return std::nullopt;
				}
				pos += 4;

				// Paire de substitution : deux \uXXXX encodent un caractere
				// hors du plan de base (emoji, certains ideogrammes).
				if (code >= 0xD800 && code <= 0xDBFF &&
					pos + 6 < json.size() && json[pos + 1] == '\\' && json[pos + 2] == 'u') {
					unsigned long low = 0;
					if (!parse_hex4(json, pos + 3, low)) {
						return std::nullopt;
					}
					if (low >= 0xDC00 && low <= 0xDFFF) {
						code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
						pos += 6;
					}
				}

				append_utf8(value, code);
				break;
			}
			default: value += next; break;
			}
			continue;
		}

		if (c == '"') {
			return value;
		}
		value += c;
	}
	return std::nullopt;
}
