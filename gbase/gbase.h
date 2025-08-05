#pragma once

#include <iostream>
#include <string>
#include <typeinfo>

#include "glogger.h"

#define SDERIVED_NAME demangle(typeid(Derived).name())


#if defined(__GNUG__)
#include <cxxabi.h>
#include <memory>

std::string inline demangle(const char* name) {
	int status = 0;
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
	explicit GBase(std::shared_ptr<GOptions> gopt) {
		log = std::make_shared<GLogger>(gopt, getDerivedName(), SDERIVED_NAME);
		log->debug(CONSTRUCTOR, getDerivedName());
	}

	~GBase() {
		log->debug(DESTRUCTOR, SDERIVED_NAME);
	}

private:

	std::string getDerivedName() const {
		return demangle(typeid(Derived).name());
	}

protected:
	std::shared_ptr<GLogger> log;
};
