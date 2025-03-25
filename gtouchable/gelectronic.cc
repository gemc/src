// gtouchable
#include "gelectronic.h"

// for UNINITIALIZEDNUMBERQUANTITY
#include "gutsConventions.h"


// Constructor initializing GElectronic with specified address and mode.
GElectronic::GElectronic(int c, int s, int ch, int m) : crate(c), slot(s), channel(ch), mode(m) {}

// Default constructor initializing GElectronic with uninitialized values.
GElectronic::GElectronic() : crate(UNINITIALIZEDNUMBERQUANTITY), slot(UNINITIALIZEDNUMBERQUANTITY),
							 channel(UNINITIALIZEDNUMBERQUANTITY), mode(UNINITIALIZEDNUMBERQUANTITY) {}


// Sets the hardware address.
void GElectronic::setHAddress(int c, int s, int ch) {
	crate = c;
	slot = s;
	channel = ch;
}

// Returns the hardware address as a vector.
std::vector<int> GElectronic::getHAddress() {
	return {crate, slot, channel};
}

// Equality operator comparing based on the mode.
bool GElectronic::operator==(const GElectronic &ge) const {
	if (this->mode == 0) {
		return this->crate == ge.crate;
	} else if (this->mode == 1) {
		return this->crate == ge.crate && this->slot == ge.slot;
	} else if (this->mode == 2) {
		return this->crate == ge.crate && this->slot == ge.slot && this->channel == ge.channel;
	}
	return false;
}

// Overloaded output stream operator for GElectronic.
std::ostream &operator<<(std::ostream &stream, const GElectronic &ge) {
	stream << " Crate: " << ge.crate;
	stream << " Slot: " << ge.slot;
	stream << " Channel: " << ge.channel;

	return stream;
}
