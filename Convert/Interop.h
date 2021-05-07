#pragma once

namespace Avf {
	namespace Interop {
		std::string convert(String^ source);
		String^ convert(const std::string& source);
		std::string format(const char* fmt, ...);
		std::string vformat(const char* fmt, va_list args);
	}
}
