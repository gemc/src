#pragma once


// gemc
#include "gQtButtonsWidget.h" // Custom toggle button widget
#include "g4volume.h"
#include "gbase.h" // Provides application options/configuration

// qt
#include <QWidget>
#include <QTreeWidgetItem>
#include <QColor>

// cpp
#include <map>
#include <memory>

class G4Ttree_item
{
public:
    explicit G4Ttree_item(G4Volume* g4volume);

private:
    std::string mother;

    QColor color;
    bool is_visible;
    int representation; // 0 = wireframe ; 1 = solid; 2 = cloud
    double opacity;

    std::string material;
    double density;
    double mass;

public:
    [[nodiscard]] std::string get_mother() const { return mother; }
    [[nodiscard]] QColor get_color() const { return color; }
    [[nodiscard]] bool get_visibility() const { return is_visible; }
    [[nodiscard]] int get_representation() const { return representation; }
    [[nodiscard]] double get_opacity() const { return opacity; }
    [[nodiscard]] double get_density() const { return density; }
    [[nodiscard]] double get_mass() const { return mass; }
    [[nodiscard]] std::string get_material() const { return material; }

    static std::string vname_from_v4name(std::string v4name);
    static std::string system_from_v4name(std::string v4name);
};

// map for a single system
using g4tree_map = std::map<std::string, std::unique_ptr<G4Ttree_item>>;

class GTree : public QWidget, public GBase<GTree>
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the G4Display widget.
     * @param gopt Pointer to the GOptions object containing application settings. Used to configure the logger.
     * @param g4volumes_map unordered std map of g4volumes
     * @param parent Optional parent QWidget for Qt's memory management.
     */
    explicit GTree(const std::shared_ptr<GOptions>& gopt,
                   std::unordered_map<std::string, G4Volume*> g4volumes_map,
                   QWidget* parent = nullptr);

    // Disable copy constructor and assignment operator to prevent slicing
    // and issues with unique_ptr ownership.
    GTree(const GTree&) = delete;
    GTree& operator=(const GTree&) = delete;

private:
    std::map<std::string, g4tree_map> g4_systems_tree; // map of systems
    QTreeWidget* treeWidget = nullptr;

    void build_tree(std::unordered_map<std::string, G4Volume*> g4volumes_map); // already implemented
    void populateTree(); // build the Qt tree from g4_systems_tree

    void set_visibility(const std::string& fullName, bool visible);
    void set_color(const std::string& fullName, const QColor& c);

private slots:
    void onItemChanged(QTreeWidgetItem* item, int column);
    void onColorButtonClicked();
};
