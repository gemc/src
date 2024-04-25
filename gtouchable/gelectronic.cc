// gtouchable
#include "gelectronic.h"

// glibrary for UNINITIALIZEDNUMBERQUANTITY, etc
#include "gutsConventions.h"

// c++
using std::vector;

GElectronic::GElectronic(int c, int s, int ch, int m) : crate(c), slot(s), channel(ch), mode(m) {}

GElectronic::GElectronic() : crate(UNINITIALIZEDNUMBERQUANTITY), slot(UNINITIALIZEDNUMBERQUANTITY),
channel(UNINITIALIZEDNUMBERQUANTITY), mode(UNINITIALIZEDNUMBERQUANTITY) {}


void GElectronic::setHAddress(int c, int s, int ch) {
	crate   = c;
	slot    = s;
	channel = ch;
}

vector<int> GElectronic::getHAddress() {
	return {crate, slot, channel};
}

// comparison based on mode
bool GElectronic::operator == (const GElectronic& ge) const {
	if ( this->mode == 0) {
		return this->crate == ge.crate;
	} else if ( this->mode == 1) {
		return this->crate == ge.crate && this->slot == ge.slot;
	} else if ( this->mode == 2) {
		return this->crate == ge.crate && this->slot == ge.slot && this->channel == ge.channel;
	}
	return false;
}

std::ostream &operator<<(std::ostream &stream, GElectronic ge)
{
	stream << " Crate: "   << ge.crate;
	stream << " Slot: "    << ge.slot;
	stream << " Channel: " << ge.channel;

	return stream;
}
