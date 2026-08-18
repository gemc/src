// gtouchable
#include "gelectronic.h"

// See header for API docs.

// Constructor initializing GElectronic with specified address and mode.
GElectronic::GElectronic(int c, int s, int ch, ComparisonMode comparison_mode) :
	crate(c), slot(s), channel(ch), mode(comparison_mode) {
	validateHAddress(c, s, ch);
}


void GElectronic::validateHAddress(int c, int s, int ch) {
	if (c < 0 || s < 0 || ch < 0) {
		throw std::invalid_argument("GElectronic hardware address components must be non-negative");
	}
}


// Sets the hardware address.
void GElectronic::setHAddress(int c, int s, int ch) {
	validateHAddress(c, s, ch);
	crate   = c;
	slot    = s;
	channel = ch;
}

// Returns the hardware address as a vector.
std::vector<int> GElectronic::getHAddress() const {
	return {crate, slot, channel};
}

// Equality operator comparing based on the mode.
bool GElectronic::operator==(const GElectronic& ge) const {
	if (mode == ComparisonMode::crate) {
		return this->crate == ge.crate;
	}
	if (mode == ComparisonMode::crate_slot) {
		return this->crate == ge.crate && this->slot == ge.slot;
	}
	return this->crate == ge.crate && this->slot == ge.slot && this->channel == ge.channel;
}

// Overloaded output stream operator for GElectronic.
std::ostream& operator<<(std::ostream& stream, const GElectronic& ge) {
	stream << " Crate: " << ge.crate;
	stream << " Slot: " << ge.slot;
	stream << " Channel: " << ge.channel;

	return stream;
}
