#pragma once

// namespace needed for the inline functions below
#include "gutilities.h"
#include "gbase.h"

// c++
#include <string>
#include <utility>

/**
 * \class GVolume
 * \brief Geometry volume record loaded into a GSystem.
 *
 * A GVolume represents a single placed detector volume as loaded from a geometry source
 * (sqlite DB, ASCII text files, CAD import, GDML import, etc.).
 *
 * The constructor takes a serialized parameter vector whose positional layout matches the
 * geometry database row layout (GVOLUMENUMBEROFPARS). The class stores:
 * - solid definition (type + parameters);
 * - visualization attributes (visibility/style/color/opacity);
 * - placement attributes (mother/pos/rot);
 * - logical attributes (material, EM field);
 * - digitization/identity metadata;
 * - optional import filename for CAD/GDML sourced volumes;
 * - post-load modifiers (shift/tilt/existence) applied by GWorld;
 * - final Geant4 naming assigned during GWorld bookkeeping.
 *
 * \note External library types (e.g. Geant4 solids) should be referenced using \c, not \ref.
 */
class GVolume : public GBase<GVolume>
{
public:
	/**
	 * \brief Construct a volume from a serialized parameter vector.
	 *
	 * \param log Logger used for diagnostics and error reporting.
	 * \param system Name of the system that owns this volume.
	 * \param pars Serialized parameter list (must match GVOLUMENUMBEROFPARS).
	 * \param importPath Optional import path used by file-based factories (CAD/GDML).
	 *
	 * \details The parameter vector is positional; the implementation parses it in order.
	 * If the vector size is incorrect, the constructor logs an error.
	 */
	GVolume(const std::shared_ptr<GLogger>& log, const std::string& system, std::vector<std::string> pars,
	        const std::string&              importPath = UNINITIALIZEDSTRINGQUANTITY);

	/**
	 * \brief Construct the special ROOT/world volume.
	 *
	 * \param rootVolumeDefinition Definition string like: \c "G4Box 15*m 15*m 15*m G4_AIR".
	 * \param log Logger used for diagnostics.
	 *
	 * \details This constructor is used when the world volume is injected automatically.
	 * It creates a top-level volume whose mother is MOTHEROFUSALL.
	 */
	explicit GVolume(const std::string& rootVolumeDefinition, const std::shared_ptr<GLogger>& log);
	// special constructor for root volume

	/**
	 * \brief Polymorphic deep-copy.
	 *
	 * \return A heap-allocated clone of the current object.
	 */
	[[nodiscard]] virtual std::unique_ptr<GVolume> clone() const {
		return std::make_unique<GVolume>(*this); // Make a copy of the current object
	}

	/// \brief Virtual destructor (safe deletion through base pointers).
	virtual ~GVolume() = default;

private:
	std::string system;         ///< System of provenance (which subsystem this volume belongs to).
	std::string name;           ///< Volume name (unique within the system; used as lookup key).
	std::string motherName;     ///< Mother volume name (placement reference).
	std::string description;    ///< Human-readable description, used primarily for diagnostics/documentation.
	std::string importFilename; ///< For imported volumes: filename with the path, set with the import factory.

	// solid parameters
	std::string type;       ///< Solid type string (follows Geant4 naming conventions).
	std::string parameters; ///< Solid constructor parameters string (units may be embedded).

	// solid visualization style
	bool        visible; ///< Visibility flag: 0=invisible, 1=visible.
	int         style;   ///< Visual style: 0=wireframe, 1=solid.
	std::string color;   ///< Color in RRGGBB format (optional last digit is transparency).
	double      opacity; ///< Opacity parsed from configuration (convention depends on renderer).

	// logical attributes
	std::string material; ///< Material name (used to resolve to a GMaterial).
	std::string emfield;  ///< Associated magnetic/electric field label.

	// physical attributes
	std::string pos;   ///< Placement position relative to mother.
	std::string rot;   ///< Placement rotation relative to mother (x,y,z Euler angles).
	std::string shift; ///< Position modifier (applied post-load by GWorld).
	std::string tilt;  ///< Rotation modifier (applied post-load by GWorld).
	bool        exist; ///< Existence modifier (applied post-load by GWorld).

	std::string digitization; ///< Digitization label and collection identifier.
	std::string gidentity;    ///< Identifier string (e.g. "sector: 2, layer: 4, wire: 33").

	// special cases
	std::string copyOf;    ///< Name of gvolume to copy from (if supported by downstream logic).
	std::string solidsOpr; ///< Solid boolean operation descriptor (if used).

	int pCopyNo{}; ///< Copy number bookkeeping (first volume of a given type should be 0).

	// mirrors
	std::string mirror; ///< Mirror configuration string (if used).

	// the map key names used in geant4 contain the system name
	// these are assigned by gworld after all volumes are loaded
	std::string g4name;       ///< Fully-qualified Geant4 volume name.
	std::string g4motherName; ///< Fully-qualified Geant4 mother volume name.

	// variation and run number for this gvolume
	std::string variation; ///< Variation tag used when loading this volume.
	int         runno{};   ///< Run number used when loading this volume.

	/// \brief Stream operator used for logging volume summaries.
	friend std::ostream& operator<<(std::ostream& stream, const GVolume&); // Logs infos on screen.

public:
	/// \name Identity and naming
	///@{
	[[nodiscard]] std::string getSystem() const { return system; }
	[[nodiscard]] std::string getName() const { return name; }
	[[nodiscard]] std::string getMotherName() const { return motherName; }
	[[nodiscard]] std::string getG4Name() const { return g4name; }
	[[nodiscard]] std::string getG4MotherName() const { return g4motherName; }
	///@}

	/**
	 * \brief Returns numeric detector dimensions parsed from the \c parameters string.
	 *
	 * \return Vector of numeric values. If parameters are unset, returns \c {0}.
	 *
	 * \note Parsing is delegated to gutilities helpers that interpret unit strings.
	 */
	[[nodiscard]] std::vector<double> getDetectorDimensions() const {
		if (parameters == UNINITIALIZEDSTRINGQUANTITY) { return {0}; }
		else { return gutilities::getG4NumbersFromString(parameters); }
	}

	/// \name Solid definition
	///@{
	[[nodiscard]] std::string getType() const { return type; }
	[[nodiscard]] std::string getParameters() const { return parameters; }
	///@}

	/// \name Logical attributes
	///@{
	[[nodiscard]] std::string getMaterial() const { return material; }
	[[nodiscard]] std::string getEMField() const { return emfield; }
	///@}

	/// \name Visualization attributes
	///@{
	[[nodiscard]] int              getPCopyNo() const { return pCopyNo; }
	[[nodiscard]] std::string_view getColor() const { return color; }
	[[nodiscard]] double           getOpacity() const { return opacity; }
	[[nodiscard]] bool             isVisible() const { return visible; }
	[[nodiscard]] int              getStyle() const { return style; }
	///@}

	/// \name Placement attributes and post-load modifiers
	///@{
	[[nodiscard]] bool        getExistence() const { return exist; }
	[[nodiscard]] std::string getPos() const { return pos; }
	[[nodiscard]] std::string getRot() const { return rot; }
	[[nodiscard]] std::string getShift() const { return shift; }
	[[nodiscard]] std::string getTilt() const { return tilt; }
	///@}

	/// \name Digitization and identity metadata
	///@{
	[[nodiscard]] std::string getDigitization() const { return digitization; }
	[[nodiscard]] std::string getGIdentity() const { return gidentity; }
	///@}

	/// \name Special cases / advanced features
	///@{
	[[nodiscard]] std::string getCopyOf() const { return copyOf; }
	[[nodiscard]] std::string getSolidsOpr() const { return solidsOpr; }
	[[nodiscard]] std::string getDescription() const { return description; }
	///@}

	/// \name Modifier application (performed by GWorld)
	///@{
	void applyShift(std::string s) { shift = std::move(s); }
	void applyTilt(std::string t) { tilt = std::move(t); }
	void modifyExistence(bool e) { exist = e; }
	void resetMotherName(std::string m) { motherName = std::move(m); }
	void setColor(std::string c) { color = std::move(c); }
	void setMaterial(std::string m) { material = std::move(m); }
	void setDigitization(std::string d) { digitization = std::move(d); }
	void setGIdentity(std::string g) { gidentity = std::move(g); }
	///@}

	/**
	 * \brief Return the import filename (path) for imported volumes.
	 *
	 * \return The original import filename stored by the factory.
	 */
	std::string getImportedFile() { return importFilename; }

	/**
	 * \brief Assign Geant4 names after all volumes are loaded.
	 *
	 * \param g4n Fully-qualified volume name (\c <system>/<name>).
	 * \param g4m Fully-qualified mother name (\c <motherSystem>/<motherName>).
	 *
	 * \note This is called by GWorld during the final bookkeeping step.
	 */
	inline void assignG4Names(const std::string& g4n, const std::string& g4m) {
		g4name       = g4n;
		g4motherName = g4m;
	}
};
