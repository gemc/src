/**
 * \file gDigitizedData.cc
 * \brief Implements GDigitizedData.
 *
 * Non-Doxygen implementation summary:
 * - copies hit identity at construction so the object is independent of the source hit lifetime
 * - stores scalar observables with overwrite semantics for event usage
 * - accumulates scalar observables by summation for integrated usage
 * - builds filtered map snapshots that separate SRO from non-SRO content
 */

#include "gDigitizedData.h"
#include "gdataConventions.h"

// c++
#include <map>
#include <string>
#include <vector>

#include "gdynamicDigitization/gdynamicdigitization_options.h"

// Thread-safe counter used only by the example/test factory create().
std::atomic<int> GDigitizedData::globalDigitizedDataCounter{0};

GDigitizedData::GDigitizedData(const std::shared_ptr<GOptions>& gopts, const GHit* ghit)
	: GBase(gopts, GDIGITIZED_DATA_LOGGER) {
	// Copy the hit identity so this object remains self-contained after the source hit expires.
	gidentity = ghit->getGID();
}

std::map<std::string, int> GDigitizedData::getIntObservablesMap(int which) const {
	// Return a filtered snapshot of the stored integer observables.
	std::map<std::string, int> filteredIntObservablesMap;
	for (const auto& [varName, value] : intObservablesMap) {
		if (validVarName(varName, which)) { filteredIntObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from intObservablesMap.");
	return filteredIntObservablesMap;
}

std::map<std::string, double> GDigitizedData::getDblObservablesMap(int which) const {
	// Return a filtered snapshot of the stored floating-point observables.
	std::map<std::string, double> filteredDblObservablesMap;
	for (const auto& [varName, value] : doubleObservablesMap) {
		if (validVarName(varName, which)) { filteredDblObservablesMap[varName] = value; }
	}
	log->info(2, " getting ", which, " from doubleObservablesMap.");
	return filteredDblObservablesMap;
}

bool GDigitizedData::validVarName(const std::string& varName, int which) {
	// Classify variables as SRO or non-SRO using the conventional names from gdataConventions.h.
	bool isSROVar = (varName == CRATESTRINGID || varName == SLOTSTRINGID || varName == CHANNELSTRINGID ||
	                 varName == CHARGEATELECTRONICS || varName == TIMEATELECTRONICS);

	// which == 0 means the caller wants non-SRO variables only.
	if (which == 0) {
		if (isSROVar) { return false; }
	}
	// which == 1 means the caller wants SRO variables only.
	else if (which == 1) {
		if (!isSROVar) { return false; }
	}

	// Any other selector currently behaves like "no additional filtering".
	return true;
}

void GDigitizedData::includeVariable(const std::string& vname, int value) {
	// Event-level insertion with overwrite semantics.
	log->info(2, "Including int variable ", vname, " with value ", value);
	intObservablesMap[vname] = value;
}

void GDigitizedData::includeVariable(const std::string& vname, double value) {
	// Event-level insertion with overwrite semantics.
	log->info(2, "Including double variable ", vname, " with value ", value);
	doubleObservablesMap[vname] = value;
}

void GDigitizedData::accumulateVariable(const std::string& vname, int value) {
	// Run/integrated accumulation by summation.
	if (intObservablesMap.find(vname) == intObservablesMap.end()) {
		log->info(2, "Accumulating new int variable ", vname, " with value ", value);
		intObservablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating int variable ", vname, " with value ", value);
		intObservablesMap[vname] += value;
	}
}

void GDigitizedData::accumulateVariable(const std::string& vname, double value) {
	// Run/integrated accumulation by summation.
	if (doubleObservablesMap.find(vname) == doubleObservablesMap.end()) {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] = value;
	}
	else {
		log->info(2, "Accumulating double variable ", vname, " with value ", value);
		doubleObservablesMap[vname] += value;
	}
}

int GDigitizedData::getTimeAtElectronics() {
	// Return the conventional electronics-time value or the sentinel if absent.
	if (intObservablesMap.find(TIMEATELECTRONICS) == intObservablesMap.end()) { return TIMEATELECTRONICSNOTDEFINED; }
	log->info(2, "Getting TIMEATELECTRONICS from intObservablesMap.");
	return intObservablesMap[TIMEATELECTRONICS];
}

int GDigitizedData::getIntObservable(const std::string& varName) {
	// Retrieve a single integer observable by key.
	if (intObservablesMap.find(varName) == intObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::intObservablesMap");
	}
	return intObservablesMap[varName];
}

double GDigitizedData::getDblObservable(const std::string& varName) {
	// Retrieve a single floating-point observable by key.
	if (doubleObservablesMap.find(varName) == doubleObservablesMap.end()) {
		log->error(ERR_VARIABLENOTFOUND,
		           "variable name <" + varName + "> not found in GDigitizedData::doubleObservablesMap");
	}
	return doubleObservablesMap[varName];
}

std::string GDigitizedData::getIdentityString() const {
	// Build a compact label from the stored identifier vector.
	std::string identifierString;
	for (size_t i = 0; i < gidentity.size() - 1; i++) {
		identifierString += gidentity[i].getName() + "->" + std::to_string(gidentity[i].getValue()) + ", ";
	}
	identifierString += gidentity.back().getName() + "->" + std::to_string(gidentity.back().getValue());
	return identifierString;
}

std::ostream& operator<<(std::ostream& os, const GDigitizedData& data) {
	os << "GDigitizedData{identity=\"" << data.getIdentityString() << "\"";

	if (!data.intObservablesMap.empty()) {
		os << ", intObservables={";
		bool first = true;
		for (const auto& [name, value] : data.intObservablesMap) {
			if (!first) {
				os << ", ";
			}
			os << name << ": " << value;
			first = false;
		}
		os << "}";
	}

	if (!data.doubleObservablesMap.empty()) {
		os << ", doubleObservables={";
		bool first = true;
		for (const auto& [name, value] : data.doubleObservablesMap) {
			if (!first) {
				os << ", ";
			}
			os << name << ": " << value;
			first = false;
		}
		os << "}";
	}

	if (!data.arrayIntObservablesMap.empty()) {
		os << ", arrayIntObservables={";
		bool firstMap = true;
		for (const auto& [name, values] : data.arrayIntObservablesMap) {
			if (!firstMap) {
				os << ", ";
			}
			os << name << ": [";
			bool firstVal = true;
			for (const auto& value : values) {
				if (!firstVal) {
					os << ", ";
				}
				os << value;
				firstVal = false;
			}
			os << "]";
			firstMap = false;
		}
		os << "}";
	}

	if (!data.arrayDoubleObservablesMap.empty()) {
		os << ", arrayDoubleObservables={";
		bool firstMap = true;
		for (const auto& [name, values] : data.arrayDoubleObservablesMap) {
			if (!firstMap) {
				os << ", ";
			}
			os << name << ": [";
			bool firstVal = true;
			for (const auto& value : values) {
				if (!firstVal) {
					os << ", ";
				}
				os << value;
				firstVal = false;
			}
			os << "]";
			firstMap = false;
		}
		os << "}";
	}

	os << "}";
	return os;
}