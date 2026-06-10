#pragma once

// gemc
#include <gemc/gqtbuttonswidget/gQtButtonsWidget.h> // Custom toggle button widget
#include <gemc/g4system/g4volume.h>
#include <gemc/gsystem/gvolume.h>
#include <gemc/gbase/gbase.h> // Provides application options/configuration
#include <gemc/goptions/goptions.h>

// qt
#include <QWidget>
#include <QTreeWidgetItem>
#include <QColor>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QSlider>

class QTimer;

// cpp
#include <map>
#include <memory>

/**
 * @class G4Ttree_item
 * @ingroup gtree_module
 * \brief Lightweight per-volume record used by GTree to populate the UI.
 *
 * @details
 * This class extracts and caches a subset of geometry and visualization
 * information from a `G4Volume` wrapper and the corresponding `GVolume` descriptor:
 * - hierarchy (mother logical volume name)
 * - material name
 * - computed mass/volume/density (scaled to common units)
 * - visualization attributes (visibility, color, opacity)
 * - pygemc descriptor fields (solid type, parameters, position, rotation, description)
 *
 * It is intentionally small and UI-oriented: it acts as the "model" part of
 * the widget, decoupled from Qt item instances.
 */
class G4Ttree_item
{
public:
    /**
     * \brief Construct a cached record for a single geometry volume.
     *
     * \param g4volume  Pointer to the volume wrapper (Geant4 solid/logical/physical).
     * \param gvolume   Optional pointer to the GEMC volume descriptor (pygemc fields).
     *                  May be \c nullptr for volumes without a descriptor (e.g., world volume).
     *
     * @details
     * The constructor reads:
     * - The mother logical volume name (or a sentinel value for the world/root).
     * - The material name.
     * - Mass, volume, and density derived from the underlying geometry.
     * - Visualization color and alpha (opacity), and the visibility flag.
     * - Solid type, parameters, position, rotation, and description from the descriptor.
     *
     * The stored "mother" name is used by GTree to reconstruct parent-child
     * relationships when building the Qt tree.
     */
    explicit G4Ttree_item(G4Volume* g4volume, const GVolume* gvolume = nullptr);

private:
    /**
     * \brief Mother logical volume full name used to build the hierarchy.
     *
     * @details
     * This is expected to match the naming convention used by the geometry map,
     * so GTree can locate the parent `QTreeWidgetItem` for each volume.
     */
    std::string mother;

    /**
     * \brief Cached RGB color used by the UI and for opacity updates.
     *
     * @details
     * Opacity updates reuse the cached RGB components and replace only the alpha
     * channel when issuing Geant4 visualization commands.
     */
    QColor color;

    /**
     * \brief Cached visibility state derived from the volume visualization attributes.
     */
    bool is_visible = true;

    /**
     * \brief Cached representation mode.
     *
     * @details
     * The value convention is:
     * - 0: wireframe
     * - 1: solid
     * - 2: cloud
     *
     * The widget uses button indices to select a representation command.
     */
    int representation = 1; // 0 = wireframe ; 1 = solid; 2 = cloud

    /**
     * \brief Cached opacity (alpha) in the range [0,1].
     */
    double opacity = 1.0;

    /**
     * \brief Cached material name for display in the properties panel.
     */
    std::string material;

    /**
     * \brief Cached mass (typically displayed in g or kg depending on magnitude).
     */
    double mass = 0.0;

    /**
     * \brief Cached volume (typically displayed in cm3 or m3 depending on magnitude).
     */
    double volume = 0.0;

    /**
     * \brief Cached density computed from mass/volume (average density).
     */
    double density = 0.0;

    /**
     * \brief Flag intended for recursive operations (e.g. propagate styling).
     *
     * @details
     * The UI currently propagates only to direct children for visibility. This
     * flag is kept to support future recursive behavior toggles.
     */
    bool recursive = false;

    // pygemc descriptor fields (empty when no GVolume descriptor is available)
    std::string solidType;       ///< Geant4 solid type string, e.g. "G4Box".
    std::string parameters;      ///< Solid constructor parameters with units, e.g. "30*mm, 40*mm, 60*mm".
    std::string position;        ///< Placement position with units, e.g. "0*mm, 0*mm, 50*mm".
    std::string rotation;        ///< Placement rotation with units, e.g. "0*deg, 0*deg, 0*deg".
    std::string motherVolume;    ///< Mother volume name as defined in pygemc.
    std::string volDescription;  ///< Human-readable volume description.

public:
    /** \brief Return the cached mother name. */
    [[nodiscard]] std::string get_mother() const { return mother; }

    /** \brief Return the cached RGB color. */
    [[nodiscard]] QColor get_color() const { return color; }

    /** \brief Return the cached visibility state. */
    [[nodiscard]] bool get_visibility() const { return is_visible; }

    /** \brief Return the cached representation mode. */
    [[nodiscard]] int get_representation() const { return representation; }

    /** \brief Return the cached opacity (alpha) in [0,1]. */
    [[nodiscard]] double get_opacity() const { return opacity; }

    /** \brief Return the cached mass. */
    [[nodiscard]] double get_mass() const { return mass; }

    /** \brief Return the cached volume. */
    [[nodiscard]] double get_volume() const { return volume; }

    /** \brief Return the cached density. */
    [[nodiscard]] double get_density() const { return density; }

    /** \brief Return the cached material name. */
    [[nodiscard]] std::string get_material() const { return material; }

    /** \brief Return the cached recursive flag. */
    [[nodiscard]] bool get_recursive() const { return recursive; }

    /** \brief Return the solid type string (e.g. "G4Box"). */
    [[nodiscard]] std::string get_solidType() const { return solidType; }
    /** \brief Return the solid parameters string with units. */
    [[nodiscard]] std::string get_parameters() const { return parameters; }
    /** \brief Return the placement position string with units. */
    [[nodiscard]] std::string get_position() const { return position; }
    /** \brief Return the placement rotation string with units. */
    [[nodiscard]] std::string get_rotation() const { return rotation; }
    /** \brief Return the mother volume name. */
    [[nodiscard]] std::string get_motherVolume() const { return motherVolume; }
    /** \brief Return the volume description. */
    [[nodiscard]] std::string get_volDescription() const { return volDescription; }

    /**
     * \brief Set the recursive flag.
     * \param recursive New recursive state.
     */
    void set_recursive(bool recursive) { this->recursive = recursive; }

    /**
     * \brief Update the cached color.
     * \param c New RGB color.
     *
     * @details
     * This is used after a color or opacity change so that subsequent opacity
     * changes can preserve RGB and update only alpha.
     */
    void set_color(const QColor& c)  { color = c; }

    /**
     * \brief Update the cached opacity.
     * \param opacity New opacity (alpha) in [0,1].
     */
    void set_opacity(double opacity) { this->opacity = opacity; }

    /**
     * \brief Extract the "leaf" volume name from a full volume name.
     *
     * \param v4name Full volume name including a system prefix separated by `/`.
     * \return The substring after the last `/`.
     *
     * @details
     * This helper is used for display in the Qt tree so the user sees a compact
     * name, while the full name remains stored as item metadata.
     */
    static std::string vname_from_v4name(std::string v4name);

    /**
     * \brief Extract the system name from a full volume name.
     *
     * \param v4name Full volume name including a system prefix separated by `/`.
     * \return The substring before the last `/`.
     *
     * @details
     * The returned system name is used as the top-level grouping node in the
     * tree widget.
     */
    static std::string system_from_v4name(std::string v4name);
};

// map for a single system
using g4tree_map = std::map<std::string, std::unique_ptr<G4Ttree_item>>;

/**
 * @class GTree
 * @ingroup gtree_module
 * \brief Qt widget that displays the geometry hierarchy and allows interactive style edits.
 *
 * @details
 * The widget builds an internal model of systems and volumes from a provided
 * geometry volume map, then constructs a `QTreeWidget` view with:
 * - a visibility checkbox column
 * - a per-item color button column
 * - a name column
 *
 * User interactions trigger slots that:
 * - update Qt state (checkboxes, button styles, slider labels)
 * - send visualization commands to the Geant4 UI manager
 *
 * The right-side panel shows properties of the selected volume and provides
 * controls for representation and opacity.
 */
class GTree : public QWidget, public GBase<GTree>
{
    Q_OBJECT

public:
    /**
     * \brief Construct the geometry tree widget.
     *
     * \param gopt Shared pointer to the option set used to configure logging and behavior.
     * \param g4volumes_map Map from full volume names to `G4Volume*` wrappers.
     * \param parent Optional Qt parent used for ownership and lifetime management.
     *
     * @details
     * Construction steps:
     * 1. Build the internal model (`g4_systems_tree`) from the volume map.
     * 2. Create and configure the `QTreeWidget` view.
     * 3. Create the right-side property panel.
     * 4. Populate the tree and connect UI signals to slots.
     */
    explicit GTree(const std::shared_ptr<GOptions>& gopt,
                   std::unordered_map<std::string, G4Volume*> g4volumes_map,
                   std::unordered_map<std::string, const GVolume*> gvolumes_map = {},
                   QWidget* parent = nullptr);

    /**
     * \brief Non-copyable: the widget owns unique model resources.
     */
    GTree(const GTree&) = delete;

    /**
     * \brief Non-assignable: the widget owns unique model resources.
     */
    GTree& operator=(const GTree&) = delete;

private:
    /**
     * \brief Internal model grouped by system.
     *
     * @details
     * - outer map key: system name
     * - inner map key: full volume name
     * - inner map value: per-volume cached attributes
     */
    std::map<std::string, g4tree_map> g4_systems_tree; // map of systems

    /**
     * \brief Flat map of GEMC volume descriptors keyed by G4 name.
     *
     * Used during tree construction to attach pygemc fields (solid type,
     * parameters, position, rotation, mother, description) to each tree item.
     */
    std::unordered_map<std::string, const GVolume*> gvolumes_map;

    /**
     * \brief Main Qt tree view widget (three columns).
     */
    QTreeWidget* treeWidget = nullptr;

    // right-side panel
    /**
     * \brief Container widget for the right-side properties panel.
     */
    QWidget* rightPanel = nullptr;

    /**
     * \brief Panel that becomes visible only when an item is selected.
     */
    QWidget* bottomPanel = nullptr; // appears only when an item is selected

    /**
     * \brief Representation buttons (wireframe / solid / cloud / centre-and-twinkle).
     */
    GQTButtonsWidget* styleButtons;  // left bar buttons

    // info labels inside bottomPanel
    QLabel* typeLabel = nullptr;         ///< Selected item type ("System" or "G4 Volume").
    QLabel* daughtersLabel = nullptr;    ///< Number of direct children in the Qt tree.
    QLabel* nameLabel = nullptr;         ///< Display name of the selected item.
    QLabel* materialLabel = nullptr;     ///< Material name (volume items only).
    QLabel* massLabel = nullptr;         ///< Mass display string (volume items only).
    QLabel* volumeLabel = nullptr;       ///< Volume display string (volume items only).
    QLabel* densityLabel = nullptr;      ///< Density display string (volume items only).
    QLabel* solidTypeLabel = nullptr;    ///< Solid type, e.g. "G4Box – Simple Box".
    QTextEdit* parametersLabel = nullptr; ///< Solid parameters with units (scrollable).
    QLabel* positionLabel = nullptr;     ///< Placement position with units.
    QLabel* rotationLabel = nullptr;     ///< Placement rotation with units.
    QLabel* motherLabel = nullptr;       ///< Mother volume name.
    QLabel* descriptionLabel = nullptr;  ///< Volume description.

    /**
     * \brief Slider controlling opacity (alpha) in [0,1] mapped from [0,100].
     */
    QSlider* opacitySlider = nullptr;

    /**
     * \brief Label showing the numeric opacity value formatted as a decimal.
     */
    QLabel*  opacityLabel  = nullptr;

    /**
     * \brief Full-width button that opens the selected volume in a new viewer window.
     *
     * The label is updated to "Inspect <volumename>" whenever a volume item is selected.
     * Hidden while a system node is selected.
     */
    QPushButton* inspectButton = nullptr;

    /**
     * \brief Build the internal system/volume model from the supplied volume map.
     * \param g4volumes_map Map from full volume names to `G4Volume*` wrappers.
     */
    void build_tree(std::unordered_map<std::string, G4Volume*> g4volumes_map);

    /**
     * \brief Populate the `QTreeWidget` view from `g4_systems_tree`.
     *
     * @details
     * This constructs the hierarchy using a multi-pass strategy:
     * - create items without parents
     * - attach them to parents based on cached mother information
     * - attach per-item color button widgets
     */
    void populateTree();

    /**
     * \brief Send a visibility command for a volume.
     * \param fullName Full volume name as used by Geant4 UI commands.
     * \param visible True for visible, false for hidden.
     */
    void set_visibility(const std::string& fullName, bool visible);

    /**
     * \brief Send a color command (RGB) for a volume.
     * \param fullName Full volume name as used by Geant4 UI commands.
     * \param c New RGB color.
     */
    void set_color(const std::string& fullName, const QColor& c);

    /**
     * \brief Send a color command (RGB + alpha) for a volume and update cached alpha.
     * \param volumeName Full volume name as used by Geant4 UI commands.
     * \param opacity Alpha value in [0,1].
     */
    void set_opacity(const std::string& volumeName, double opacity);

    /**
     * \brief Build the right-side properties panel widget.
     * \return Newly created QWidget that becomes part of the widget layout.
     */
    QWidget* right_widget();

    /**
     * \brief Return the number of direct children in the Qt tree for an item.
     * \param item Tree item to query.
     * \return Direct child count, or 0 if item is null.
     */
    int get_ndaughters(QTreeWidgetItem* item) const;

    /**
     * \brief Locate the cached model record for a given full volume name.
     * \param fullName Full volume name key.
     * \return Pointer to the cached item, or nullptr if not found.
     */
    G4Ttree_item* findTreeItem(const std::string& fullName);

    /**
     * \brief Full volume name for the currently selected volume item.
     *
     * @details
     * Empty when a system node is selected or when nothing is selected.
     * Used by representation and opacity actions.
     */
    std::string current_volume_name;

    // Twinkle animation state
    QTimer*     twinkleTimer       = nullptr;
    int         twinkleTick        = 0;
    QColor      twinkleSavedColor;
    double      twinkleSavedOpacity = 1.0;
    std::string twinkleVolumeName;

    /**
     * \brief Centre the viewer on the selected volume and start the twinkle animation.
     */
    void centreTwinkle();

private slots:
    /**
     * \brief React to checkbox state changes in the tree.
     *
     * \param item Tree item that changed.
     * \param column Column index that changed (only column 0 is handled).
     *
     * @details
     * - If a system node changes, propagate its visibility state to direct volume children.
     * - If a volume node changes, apply to itself and propagate to direct volume children.
     */
    void onItemChanged(QTreeWidgetItem* item, int column);

    /**
     * \brief Open a color chooser and apply the selected color to the clicked volume.
     *
     * @details
     * The clicked button carries the full volume name as a Qt property.
     * The selected color is applied immediately via a Geant4 visualization command.
     */
    void onColorButtonClicked();

    /**
     * \brief Update the properties panel based on the clicked tree item.
     * \param item Clicked item.
     * \param column Column index (unused).
     */
    void onTreeItemClicked(QTreeWidgetItem* item, int column);

    /**
     * \brief Keep the properties panel synchronized with keyboard selection changes.
     * \param current Newly selected item.
     * \param previous Previously selected item.
     */
    void onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);

    /**
     * \brief Apply the representation corresponding to the currently pressed style button.
     *
     * @details
     * Button indices map to the following visualization commands:
     * - 0: force wireframe
     * - 1: force solid
     * - 2: force cloud
     * - 3: centre viewer on volume and twinkle colors to highlight it
     */
    void changeStyle();

    /**
     * \brief React to opacity slider movement and apply alpha to the selected volume.
     * \param value Slider integer value in [0,100].
     */
    void onOpacitySliderChanged(int value);

    /**
     * \brief Open the currently selected volume in a new viewer window.
     *
     * Detects the active viewer's driver at runtime so the new window uses
     * the same graphics back-end (e.g. TOOLSSG_QT_GLES).  A fresh scene
     * containing only the target volume is created, keeping the main scene
     * untouched.
     */
    void inspectVolume();

    /**
     * \brief Advance one step of the twinkle color animation; restore original on completion.
     */
    void onTwinkleStep();
};
