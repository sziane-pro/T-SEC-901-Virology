#define _CRT_SECURE_NO_WARNINGS

#include "error.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <windows.h>

namespace {

// CP1252 (ou toute page de code ANSI) -> UTF-8. Sans ca, un message systeme
// accentue devient invalide une fois transporte en JSON.
std::string to_utf8(const char* ansi) {
	const int wide_length = MultiByteToWideChar(CP_ACP, 0, ansi, -1, nullptr, 0);
	if (wide_length <= 1) {
		return ansi;
	}

	std::wstring wide(static_cast<size_t>(wide_length), L'\0');
	MultiByteToWideChar(CP_ACP, 0, ansi, -1, &wide[0], wide_length);

	const int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1,
		nullptr, 0, nullptr, nullptr);
	if (utf8_length <= 1) {
		return ansi;
	}

	std::string utf8(static_cast<size_t>(utf8_length), '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &utf8[0], utf8_length,
		nullptr, nullptr);
	utf8.pop_back();   // retire le \0 final
	return utf8;
}

std::string trim_message(std::string message) {
	while (!message.empty()) {
		const char last = message.back();
		if (last == '\r' || last == '\n' || last == ' ' || last == '.') {
			message.pop_back();
			continue;
		}
		break;
	}
	return message;
}

}  // namespace

std::string format_error(unsigned long code, const std::string& context) {
	char buffer[512] = {};
	const DWORD written = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, static_cast<DWORD>(code), 0,
		buffer, sizeof(buffer), nullptr);

	std::string message;
	if (written > 0) {
		message = trim_message(to_utf8(buffer));
	}

	char code_text[32];
	snprintf(code_text, sizeof(code_text), " (code %lu", code);

	return context + code_text + (message.empty() ? ")" : ": " + message + ")");
}

std::string format_last_error(const std::string& context) {
	return format_error(GetLastError(), context);
}

std::string format_errno(const std::string& context) {
	const int code = errno;
	const char* reason = strerror(code);
	return context + " (errno " + std::to_string(code) +
		(reason ? ": " + std::string(reason) : "") + ")";
}
