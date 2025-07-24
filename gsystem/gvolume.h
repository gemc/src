#pragma once

// namespace needed for the inline functions below
#include "gutilities.h"
#include "glogger.h"

// c++
#include <string>
#include <iostream>
#include <utility>


class GVolume {
public:
	GVolume(const std::shared_ptr<GLogger>& log, std::string system, std::vector<std::string> pars, std::string importPath = UNINITIALIZEDSTRINGQUANTITY);

	explicit GVolume(const std::string& rootVolumeDefinition); // special constructor for root volume

	// Define a virtual clone method to be used in the copy constructor
	[[nodiscard]] virtual std::unique_ptr<GVolume> clone() const {
		return std::make_unique<GVolume>(*this); // Make a copy of the current object
	}

	// Virtual destructor, needed for when unique_ptr<GVolume> is deleted
	virtual ~GVolume() = default;

private:
	std::string system;         // System of provenience
	std::string name;           // Name of the volume
	std::string motherName;     // Mother Volume name
	std::string description;    // Volume Description, for documentation
	std::string importFilename; // For imports, filename with the path, set with the import factory

	// solid parameters
	std::string type;       // solid type. This follows the GEANT4 definitions
	std::string parameters; // vector of parameters used in the geant4 solid constructor

	// solid visualization style
	bool        visible; // visibility of the detector: 0=invisible 1=visible
	int         style;   // Visual style: 0=wireframe 1=solid
	std::string color;   // 6(7) digits colors in RRGGBB format. The last optional digit is transparency

	// logical attributes
	std::string material; // Volume Material name.
	std::string emfield;  // Associated Magnetic Field name.

	// physical attributes
	std::string pos;   // Defines the position relative to the mother volume
	std::string rot;   // Define the rotation Matrix, defined by rotations along x,y,z axis relative to the mother volume
	std::string shift; // Position modifier
	std::string tilt;  // Rotation modifier
	bool        exist; // Existance modifier

	std::string digitization; // Assigns digitization type and collection ID
	std::string gidentity;    // String with identifiers in it. Example: 'sector: 2, layer: 4, wire; 33'

	// special cases
	std::string copyOf;    // name of gvolume to copy from
	std::string replicaOf; // name of gvolume to replica from
	std::string solidsOpr; // solid operation

	int pCopyNo{}; // should be set to 0 for the first volume of a given type

	// mirrors
	std::string mirror;

	// the map key names used in geant4 contain the system name
	// these are assigned by gworld after all voumes are loaded
	std::string g4name;       // Name of the g4volume
	std::string g4motherName; // Name of the g4 Mother volume

	// variation and run number for this gvolume
	std::string variation;
	int         runno{};

	friend std::ostream& operator<<(std::ostream& stream, const GVolume&); // Logs infos on screen.

public:
	[[nodiscard]] std::string getSystem() const { return system; }

	[[nodiscard]] std::string getName() const { return name; }

	[[nodiscard]] std::string getMotherName() const { return motherName; }

	[[nodiscard]] std::string getG4Name() const { return g4name; }

	[[nodiscard]] std::string getG4MotherName() const { return g4motherName; }

	[[nodiscard]] std::vector<double> getDetectorDimensions() const {
		if (parameters == UNINITIALIZEDSTRINGQUANTITY) { return {0}; }
		else { return gutilities::getG4NumbersFromString(parameters); }
	}

	[[nodiscard]] std::string getType() const { return type; }

	[[nodiscard]] std::string getParameters() const { return parameters; }

	[[nodiscard]] std::string getMaterial() const { return material; }

	[[nodiscard]] int getPCopyNo() const { return pCopyNo; }

	[[nodiscard]] std::string_view getColor() const { return color; }

	[[nodiscard]] std::string getEMField() const { return emfield; }

	[[nodiscard]] bool isVisible() const { return visible; }

	[[nodiscard]] int getStyle() const { return style; }

	[[nodiscard]] bool getExistence() const { return exist; }

	[[nodiscard]] std::string getPos() const { return pos; }

	[[nodiscard]] std::string getRot() const { return rot; }

	[[nodiscard]] std::string getShift() const { return shift; }

	[[nodiscard]] std::string getTilt() const { return tilt; }

	[[nodiscard]] std::string getDigitization() const { return digitization; }

	[[nodiscard]] std::string getGIdentity() const { return gidentity; }

	// special cases
	[[nodiscard]] std::string getCopyOf() const { return copyOf; }

	[[nodiscard]] std::string getReplicaOf() const { return replicaOf; }

	[[nodiscard]] std::string getSolidsOpr() const { return solidsOpr; }

	[[nodiscard]] std::string getDescription() const { return description; }

	// assign modifiers
	void applyShift(std::string s) { shift = std::move(s); }

	void applyTilt(std::string t) { tilt = std::move(t); }

	void modifyExistence(bool e) { exist = e; }

	void resetMotherName(std::string m) { motherName = std::move(m); }

	void setColor(std::string c) { color = std::move(c); }

	void setMaterial(std::string m) { material = std::move(m); }

	void setDigitization(std::string d) { digitization = std::move(d); }

	void setGIdentity(std::string g) { gidentity = std::move(g); }

	// imported volumes
	std::string getImportedFile() { return importFilename; }

	// assign g4names
	inline void assignG4Names(std::string g4n, std::string g4m) {
		g4name       = std::move(g4n);
		g4motherName = std::move(g4m);
	}

};
