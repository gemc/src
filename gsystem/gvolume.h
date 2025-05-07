#pragma once

// namespace needed for the inline functions below
#include "gutilities.h"
#include "glogger.h"

// c++
#include <string>

#include <iostream>
#include <utility>

using std::string;
using std::string_view;
using std::ostream;
using std::vector;

class GVolume {
public:
	GVolume(const std::shared_ptr<GLogger>& log, string system, vector<string> pars, string importPath = UNINITIALIZEDSTRINGQUANTITY);

	explicit GVolume(const string& rootVolumeDefinition); // special constructor for root volume

	// Define a virtual clone method to be used in the copy constructor
	[[nodiscard]] virtual std::unique_ptr<GVolume> clone() const {
		return std::make_unique<GVolume>(*this);  // Make a copy of the current object
	}

	// Virtual destructor, needed for when unique_ptr<GVolume> is deleted
	virtual ~GVolume() = default;


private:
	string system;         // System of provenience
	string name;           // Name of the volume
	string motherName;     // Mother Volume name
	string description;    // Volume Description, for documentation
	string importFilename; // For imports, filename with path, set with the import factory

	// solid parameters
	string type;       // solid type. This follows the GEANT4 definitions
	string parameters; // vector of parameters used in the geant4 solid constructor

	// solid visualization style
	bool   visible; // visibility of the detector: 0=invisible 1=visible
	int    style;   // Visual style: 0=wireframe 1=solid
	string color;   // 6(7) digits colors in RRGGBB format. The last optional digit is transparency

	// logical attributes
	string material; // Volume Material name.
	string emfield;  // Associated Magnetic Field name.

	// physical attributes
	string pos;   // Defines the position relative to the mother volume
	string rot;   // Define the rotation Matrix, defined by rotations along x,y,z axis relative to the mother volume
	string shift; // Position modifier
	string tilt;  // Rotation modifier
	bool   exist; // Existance modifier

	string digitization; // Assigns digitization type and collection ID
	string gidentity;    // String with identifiers in it. Example: 'sector: 2, layer: 4, wire; 33'

	// special cases
	string copyOf;    // name of gvolume to copy from
	string replicaOf; // name of gvolume to replica from
	string solidsOpr; // solids operation

	int pCopyNo{}; // should be set to 0 for the first volume of a given type

	// mirrors
	string mirror;

	// the map key names used in geant4 contains the system name
	// these are assigned by gworld after all voumes are loaded
	string g4name;       // Name of the g4volume
	string g4motherName; // Name of the g4 Mother volume

	// variation and run number for this gvolume
	string variation;
	int    runno{};

	friend ostream& operator<<(ostream& stream, const GVolume&); // Logs infos on screen.

public:
	[[nodiscard]] inline string getSystem() const { return system; }

	[[nodiscard]] inline string getName() const { return name; }

	[[nodiscard]] inline string getMotherName() const { return motherName; }

	[[nodiscard]] inline string getG4Name() const { return g4name; }

	[[nodiscard]] inline string getG4MotherName() const { return g4motherName; }

	[[nodiscard]] inline vector<double> getDetectorDimensions() const {
		if (parameters == UNINITIALIZEDSTRINGQUANTITY) { return {0}; }
		else { return gutilities::getG4NumbersFromString(parameters); }
	}

	[[nodiscard]] inline string getType() const { return type; }

	[[nodiscard]] inline string getParameters() const { return parameters; }

	[[nodiscard]] inline string getMaterial() const { return material; }

	[[nodiscard]] inline int getPCopyNo() const { return pCopyNo; }

	[[nodiscard]] inline string_view getColor() const { return color; }

	[[nodiscard]] inline string getEMField() const { return emfield; }

	[[nodiscard]] inline bool isVisible() const { return visible; }

	[[nodiscard]] inline int getStyle() const { return style; }

	[[nodiscard]] inline bool getExistence() const { return exist; }

	[[nodiscard]] inline string getPos() const { return pos; }

	[[nodiscard]] inline string getRot() const { return rot; }

	[[nodiscard]] inline string getShift() const { return shift; }

	[[nodiscard]] inline string getTilt() const { return tilt; }

	[[nodiscard]] inline string getDigitization() const { return digitization; }

	[[nodiscard]] inline string getGIdentity() const { return gidentity; }

	// special cases
	[[nodiscard]] inline string getCopyOf() const { return copyOf; }

	[[nodiscard]] inline string getReplicaOf() const { return replicaOf; }

	[[nodiscard]] inline string getSolidsOpr() const { return solidsOpr; }

	[[nodiscard]] inline string getDescription() const { return description; }

	// assign modifiers
	void applyShift(string s) { shift = std::move(s); }

	void applyTilt(string t) { tilt = std::move(t); }

	void modifyExistence(bool e) { exist = e; }

	void resetMotherName(string m) { motherName = std::move(m); }

	void setColor(string c) { color = std::move(c); }

	void setMaterial(string m) { material = std::move(m); }

	void setDigitization(string d) { digitization = std::move(d); }

	void setGIdentity(string g) { gidentity = std::move(g); }

	// imported volumes
	string getImportedFile() { return importFilename; }

	// assign g4names
	inline void assignG4Names(string g4n, string g4m) {
		g4name       = std::move(g4n);
		g4motherName = std::move(g4m);
	}

};

