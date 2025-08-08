#pragma once

// C++
#include <iostream>
#include <string>
#include <typeinfo>

// gbase
#include "glogger.h"

#if defined(__GNUG__)
#include <cxxabi.h>
#include <memory>

std::string inline demangle(const char* name) {
	int status      = 0;
	using deleter_t = void(*)(void*);
	std::unique_ptr<char, deleter_t> demangled(abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
	return (status == 0 && demangled) ? demangled.get() : name;
}

#else
std::string demangle(const char* name) {
	return name; // No demangling on non-GNU compilers
}
#endif


template <typename Derived>
class GBase {
public:
	explicit GBase(std::shared_ptr<GOptions> gopt, std::string logger_name = "") {
		log = std::make_shared<GLogger>(gopt, getDerivedName(), logger_name);
		log->debug(CONSTRUCTOR, getDerivedName());
	}

	// sharing logger for heavy creations like gtouchable
	explicit GBase(const std::shared_ptr<GLogger>& logger) : log(logger) {}

	~GBase() { log->debug(DESTRUCTOR, getDerivedName()); }

	// Important: because we declared a destructor, default these explicitly.
	GBase(const GBase&)                = default; // shallow copy of shared_ptr
	GBase& operator=(const GBase&)     = default;
	GBase(GBase&&) noexcept            = default; // otherwise move would be suppressed
	GBase& operator=(GBase&&) noexcept = default;

private:
	std::string getDerivedName() const { return demangle(typeid(Derived).name()); }

protected:
	std::shared_ptr<GLogger> log;
};
