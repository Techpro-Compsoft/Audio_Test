#include "stdafx.h"
#include "Interop.h"

using namespace Avf;

std::string Interop::convert(String^ source) {

	array<Byte>^ utf8 = Encoding::UTF8->GetBytes(source);

	if (utf8->Length > 0) {
		pin_ptr<Byte> p = &utf8[0];
		unsigned char* s = p;
	    return std::string(reinterpret_cast<const char*>(s), utf8->Length);
	}
	else {
		return std::string();
	}

}
String^ Interop::convert(const std::string& source) {
	return gcnew String(source.c_str());
}

std::string Interop::vformat(const char * format, va_list args) {

	std::string str;

	va_list vargs1;
	va_copy(vargs1, args);
	int n = vsnprintf(nullptr, 0, format, vargs1);
	va_end(vargs1);

	if (n > 0) {
		char* buf = new char[n+1];
		va_list vargs2;
		va_copy(vargs2, args);
		vsnprintf(buf, n+1, format, vargs2);
		va_end(vargs2);
		str = buf;
		delete[] buf;
	}

	return str;
}

std::string Interop::format(const char* fmt, ...) {
	va_list vargs;
	va_start(vargs, fmt);
	std::string str = vformat(fmt, vargs);
	va_end(vargs);	
	return str;	
}