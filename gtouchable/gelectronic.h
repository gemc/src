#ifndef GELECTRONIC_H
#define GELECTRONIC_H  1

#include <ostream>
#include <vector>

struct GElectronic {

public:

	GElectronic(int c, int s, int ch, int m);

	// empty constructor needed because of the map declaration in the translation table
	GElectronic();

	void setHAddress(int c, int s, int ch);
	std::vector<int> getHAddress();

private:
	int crate;
	int slot;
	int channel;

	// GElectronic comparison:
	// mode == 0: crate
	// mode == 1: crate, slot
	// mode == 2: crate, slot, channel
	// for example, if mode = 1:
	// 3,3,1 is the same as 3,3,4
	// in mode 2 the above would not be the same
	int mode;

	//! overloading the << operator
	friend std::ostream &operator<<(std::ostream &stream, GElectronic ge);

	//  Overloaded "!=": comparing idValue
	bool operator == (const GElectronic& ge) const; 

};


#endif
