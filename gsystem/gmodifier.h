#pragma once

#include <iostream>

class GModifier {

public:

	GModifier(const std::string& n, const std::string& s, const std::string& t, bool existence) :
			name(n),
			shift(s),
			tilts(t),
			isPresent(existence) {}

	GModifier(const GModifier &other) = default;

private:
	std::string name;  // volume name
	std::string shift;
	std::string tilts;
	bool isPresent; // true by default
	friend std::ostream &operator<<(std::ostream &stream, const GModifier& gm) {
		stream << "GModifier: " << gm.name << " shift: " << gm.shift << " tilts: " << gm.tilts << " isPresent: "
			   << gm.isPresent;
		return stream;
	}

public:
	std::string getName() { return name; }

	std::string getShift() { return shift; }

	std::string getTilts() { return tilts; }

	[[nodiscard]] bool getExistence() const { return isPresent; }

};
