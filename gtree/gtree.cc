// Qt
#include <QHeaderView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QColorDialog>
#include <QSignalBlocker>

// gtree
#include "gtree.h"
#include "gtree_options.h"

// gemc
#include "gsystemConventions.h"

// geant4
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "gtouchable.h"
#include "gutilities.h"
#include "G4SystemOfUnits.hh"   // for mm, cm, g/cm3, etc.

G4Ttree_item::G4Ttree_item(G4Volume* g4volume) {
    auto pvolume = g4volume->getPhysical();
    auto lvolume = g4volume->getLogical();
    auto svolume = g4volume->getSolid();

    std::string lname = lvolume->GetName();
    if (lname != ROOTWORLDGVOLUMENAME) {
        auto mlvolume = pvolume->GetMotherLogical();
        mother = mlvolume->GetName();
        material = lvolume->GetMaterial()->GetName();
        mass = lvolume->GetMass(false, true);
    }
    else {
        mother = MOTHEROFUSALL;
        material = "G4_Galactic";
    }

    auto visAttributes = lvolume->GetVisAttributes();
    auto gcolor = visAttributes->GetColour();

    double red = gcolor.GetRed();
    double green = gcolor.GetGreen();
    double blue = gcolor.GetBlue();
    auto alpha = gcolor.GetAlpha();

    color = QColor::fromRgbF(red, green, blue);

    opacity = alpha;

    is_visible = visAttributes->IsVisible();

    mass = lvolume->GetMass(false, true) / (CLHEP::g);
    volume = svolume->GetCubicVolume() / CLHEP::cm3;
    density = (mass / volume);
}


std::string G4Ttree_item::vname_from_v4name(std::string v4name) {
    // return name after '/'
    return v4name.substr(v4name.find_last_of('/') + 1);
}

std::string G4Ttree_item::system_from_v4name(std::string v4name) {
    // return name before '/'
    return v4name.substr(0, v4name.find_last_of('/'));
}


// GTree Constructor
// Initializes the Widget base class and the unique_ptr for the logger.
GTree::GTree(const std::shared_ptr<GOptions>& gopt,
             std::unordered_map<std::string, G4Volume*> g4volumes_map,
             QWidget* parent) :
    QWidget(parent),
    GBase(gopt, GTREE_LOGGER) {
    build_tree(g4volumes_map);

    // create the UI
    treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(3);
    QStringList headers;
    headers << "Visibility" << "Color" << "Name";
    treeWidget->setHeaderLabels(headers);
    treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    treeWidget->setRootIsDecorated(true);
    treeWidget->setAlternatingRowColors(true);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(treeWidget, /*stretch*/ 3);

    // Right: property panel has ~same width. Increase stretch to increase
    rightPanel = right_widget();
    mainLayout->addWidget(rightPanel, /*stretch*/ 3);

    setLayout(mainLayout);

    // populate the tree from g4_systems_tree
    populateTree();

    // react to checkboxes
    connect(treeWidget, &QTreeWidget::itemChanged,
            this, &GTree::onItemChanged);


    // react to clicks / selection to update the right panel
    connect(treeWidget, &QTreeWidget::itemClicked,
            this, &GTree::onTreeItemClicked);

    connect(treeWidget, &QTreeWidget::currentItemChanged,
            this, &GTree::onCurrentItemChanged);

    // connect GQTButtonsWidget signal button_pressed to slot changeStyle()
    connect(styleButtons->buttonsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
            this, SLOT(changeStyle()));


    // connect slider to slot
    connect(opacitySlider, &QSlider::valueChanged,
            this, &GTree::onOpacitySliderChanged);

    log->debug(NORMAL, SFUNCTION_NAME, "GTree added");
}

void GTree::populateTree() {
    // for each system
    for (auto& [systemName, volMap] : g4_systems_tree) {
        // top-level item for the system
        auto* systemItem = new QTreeWidgetItem(treeWidget);
        systemItem->setText(2, QString::fromStdString(systemName));
        systemItem->setFlags(systemItem->flags() | Qt::ItemIsUserCheckable);
        systemItem->setCheckState(0, Qt::Checked);

        // we'll build the volume hierarchy inside this system
        std::map<std::string, QTreeWidgetItem*> itemLookup;

        // --------------------------------------------------------------------
        // 1st pass: create items WITHOUT parents, configure text/data/checkbox
        // --------------------------------------------------------------------
        for (auto& [volName, vptr] : volMap) {
            const G4Ttree_item* vitem = vptr.get();

            auto* item = new QTreeWidgetItem; // no parent yet
            item->setText(2, QString::fromStdString(G4Ttree_item::vname_from_v4name(volName)));
            item->setData(2, Qt::UserRole, QString::fromStdString(volName)); // store full v4 name

            // checkbox for visibility:
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, vitem->get_visibility() ? Qt::Checked : Qt::Unchecked);

            itemLookup[volName] = item;
        }

        // --------------------------------------------------------------------
        // 2nd pass: attach items to their parent (system or mother)
        // --------------------------------------------------------------------
        for (auto& [volName, vptr] : volMap) {
            const G4Ttree_item* vitem = vptr.get();
            auto mother = vitem->get_mother(); // full v4 name of mother
            auto* thisItem = itemLookup[volName];

            QTreeWidgetItem* parentItem = systemItem;

            if (!mother.empty() && mother != "root") {
                auto itM = itemLookup.find(mother);
                if (itM != itemLookup.end()) {
                    parentItem = itM->second;
                }
            }

            parentItem->addChild(thisItem);
        }

        // --------------------------------------------------------------------
        // 3rd pass: create color buttons now that items are in the tree
        // --------------------------------------------------------------------
        for (auto& [volName, vptr] : volMap) {
            const G4Ttree_item* vitem = vptr.get();
            QTreeWidgetItem* item = itemLookup[volName];

            auto* colorBtn = new QPushButton(treeWidget);
            QColor c = vitem->get_color();
            colorBtn->setFixedSize(20, 20);
            colorBtn->setFlat(true); // no 3D/bevel look
            colorBtn->setText(QString()); // no text

            colorBtn->setStyleSheet(
                QString("QPushButton { background-color: %1; border: 1px solid black; }")
                .arg(c.name())
            );

            colorBtn->setProperty("volumeName", QString::fromStdString(volName));
            connect(colorBtn, &QPushButton::clicked, this, &GTree::onColorButtonClicked);

            treeWidget->setItemWidget(item, 1, colorBtn);
        }
    }

    treeWidget->expandAll();
}


void GTree::build_tree(std::unordered_map<std::string, G4Volume*> g4volumes_map) {
    // loop over map
    for (auto [name, g4volume] : g4volumes_map) {
        // skip the Geant4 world / ROOTWORLDGVOLUMENAME
        // auto lvolume = g4volume->getLogical();
        //
        // if (lvolume && lvolume->GetName() == ROOTWORLDGVOLUMENAME) {
        //     log->info(2, "Skipping world volume >", name, "< from tree");
        //     continue;
        // }

        auto system_name = G4Ttree_item::system_from_v4name(name);

        // ensure the system exists
        auto& system_tree = g4_systems_tree[system_name];
        system_tree[name] = std::make_unique<G4Ttree_item>(g4volume);

        log->info(2, "Adding ", name, " to tree, system_name is ", system_name,
                  ", density: ", system_tree[name]->get_density(),
                  "g/cm3, mass: ", system_tree[name]->get_mass(),
                  "g, volume: ", system_tree[name]->get_volume(), "cm3");
    }
}

void GTree::onItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0) return; // we care about visibility column only

    // is this a volume item? (has stored v4 name in UserRole)
    QVariant v = item->data(2, Qt::UserRole);

    // --------------------------------------------------------------------
    // SYSTEM item: no UserRole data → propagate to direct daughters
    // --------------------------------------------------------------------
    if (!v.isValid()) {
        bool visible = (item->checkState(0) == Qt::Checked);

        QSignalBlocker blocker(treeWidget); // avoid recursive itemChanged

        const int nChildren = item->childCount();
        for (int i = 0; i < nChildren; ++i) {
            QTreeWidgetItem* child = item->child(i);
            QVariant cv = child->data(2, Qt::UserRole);
            if (!cv.isValid())
                continue; // skip non-volume children (shouldn't happen)

            const QString fullName = cv.toString();

            // sync child checkbox
            child->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);

            // apply to Geant4
            set_visibility(fullName.toStdString(), visible);
        }

        return;
    }

    // --------------------------------------------------------------------
    // VOLUME item: apply to itself and its direct daughter volumes
    // --------------------------------------------------------------------
    const QString fullName = v.toString();
    bool visible = (item->checkState(0) == Qt::Checked);

    QSignalBlocker blocker(treeWidget); // avoid recursive itemChanged

    // 1) apply to this volume
    set_visibility(fullName.toStdString(), visible);

    // 2) apply same visibility to its direct daughters
    const int nChildren = item->childCount();
    for (int i = 0; i < nChildren; ++i) {
        QTreeWidgetItem* child = item->child(i);
        QVariant cv = child->data(2, Qt::UserRole);
        if (!cv.isValid())
            continue; // skip if not a volume

        const QString childName = cv.toString();

        // sync child checkbox
        child->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);

        // apply to Geant4
        set_visibility(childName.toStdString(), visible);
    }
}


void GTree::onColorButtonClicked() {
    auto* btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
        return;

    const QString volName = btn->property("volumeName").toString();
    if (volName.isEmpty())
        return;

    QColor initial = Qt::white;

    QColor c = QColorDialog::getColor(initial, this, tr("Select color"));
    if (!c.isValid())
        return;

    // Update button appearance
    btn->setStyleSheet(
        QString("QPushButton { background-color: %1; border: 1px solid black; }")
        .arg(c.name())
    );

    // tell your model
    set_color(volName.toStdString(), c);
}


void GTree::set_visibility(const std::string& volumeName, bool visible) {
    std::string vis_int = visible ? "1" : "0";

    std::string command = "/vis/geometry/set/visibility " + volumeName + " -1 " + vis_int;

    if (volumeName == ROOTWORLDGVOLUMENAME) {
        command = "/vis/geometry/set/visibility " + volumeName + " 0 " + vis_int;
    }

    gutilities::apply_uimanager_commands(command);
}

void GTree::set_color(const std::string& volumeName, const QColor& c) {
    int r, g, b;
    c.getRgb(&r, &g, &b);

    std::string command = "/vis/geometry/set/colour " + volumeName + " 0 "
        + std::to_string(r / 255.0) + " "
        + std::to_string(g / 255.0) + " "
        + std::to_string(b / 255.0);

    gutilities::apply_uimanager_commands(command);
}

int GTree::get_ndaughters(QTreeWidgetItem* item) const {
    if (!item) return 0;
    return item->childCount();
}

G4Ttree_item* GTree::findTreeItem(const std::string& fullName) {
    for (const auto& [systemName, volMap] : g4_systems_tree) {
        auto it = volMap.find(fullName);
        if (it != volMap.end()) {
            return it->second.get();
        }
    }
    return nullptr;
}


void GTree::onTreeItemClicked(QTreeWidgetItem* item, int /*column*/) {
    if (!bottomPanel)
        return;

    if (!item) {
        bottomPanel->setVisible(false);
        current_volume_name.clear();
        return;
    }

    bottomPanel->setVisible(true);

    // Is it a volume? (volume items store the full v4 name in UserRole)
    QVariant v = item->data(2, Qt::UserRole);
    bool isVolume = v.isValid();

    // Type label
    if (isVolume) {
        typeLabel->setText(QStringLiteral("<b>G4 Volume</b>"));
        current_volume_name = v.toString().toStdString();
    }
    else {
        typeLabel->setText(QStringLiteral("<b>System</b>"));
        current_volume_name.clear();
    }

    // Number of direct daughters
    int nd = get_ndaughters(item);
    daughtersLabel->setText(tr("Daughters: %1").arg(nd));

    // Name (column 2 text)
    QString itemName = item->text(2);
    nameLabel->setText(tr("Name: %1").arg(itemName));

    // Material / density / mass
    if (isVolume) {
        styleButtons->setVisible(true);

        const std::string fullName = v.toString().toStdString();
        const G4Ttree_item* titem = findTreeItem(fullName);

        if (titem) {
            materialLabel->setText(
                tr("Material: %1").arg(QString::fromStdString(titem->get_material()))
            );
            auto mass = titem->get_mass();
            if (mass < 1000) {
                massLabel->setText(tr("Total Mass: %1 g").arg(mass));
            }
            else {
                massLabel->setText(tr("Total Mass: %1 kg").arg(mass / 1000));
            }
            auto volume = titem->get_volume();
            if (volume < 1000000) {
                volumeLabel->setText(tr("Volume: %1 cm3").arg(volume));
            }
            else {
                volumeLabel->setText(tr("Volume: %1 m3").arg(volume / 1000000));
            }

            densityLabel->setText(
                tr("Average Density: %1 g / cm3").arg(titem->get_density())
            );
            double op = titem->get_opacity();
            int sliderVal = static_cast<int>(op * 100.0 + 0.5);
            {
                QSignalBlocker blocker(opacitySlider);
                opacitySlider->setValue(sliderVal);
            }
            opacityLabel->setText(QString::number(op, 'f', 2));
            opacitySlider->setVisible(true);
        }
    }
    else {
        styleButtons->setVisible(false);
        opacitySlider->setVisible(false);
        // Systems don't have a single material etc.
        materialLabel->setText(tr(""));
        massLabel->setText(tr(""));
        volumeLabel->setText(tr(""));
        densityLabel->setText(tr(""));
    }
}

void GTree::onCurrentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
    Q_UNUSED(previous);
    if (!current)
        return;

    // reset styleButtons
    styleButtons->reset_buttons();

    // Reuse the same logic as mouse clicks
    onTreeItemClicked(current, 0);
}

void GTree::changeStyle() {

    int button_index = styleButtons->button_pressed();

    std::string command;

    if (button_index == 0) {
        command = "/vis/geometry/set/forceWireframe " + current_volume_name + " 0 1 ";
    }
    else if (button_index == 1) {
        command = "/vis/geometry/set/forceSolid " + current_volume_name + " 0 1 ";
    }
    else if (button_index == 2) {
        command = "/vis/geometry/set/forceCloud " + current_volume_name + " 0 1 ";
    }

    gutilities::apply_uimanager_commands(command);
}

void GTree::onOpacitySliderChanged(int value) {
    if (current_volume_name.empty())
        return; // no volume selected

    double opacity = value / 100.0;

    if (opacityLabel) {
        opacityLabel->setText(QString::number(opacity, 'f', 2));
    }

    set_opacity(current_volume_name, opacity);
}

void GTree::set_opacity(const std::string& volumeName, double opacity) {

    // find current color for this volume from our tree model
    G4Ttree_item* item = findTreeItem(volumeName);
    if (!item) return;

    QColor c = item->get_color();
    double r = c.redF();
    double g = c.greenF();
    double b = c.blueF();

    // Geant4: r g b alpha
    std::string command = "/vis/geometry/set/colour " + volumeName + " 0 "
        + std::to_string(r) + " "
        + std::to_string(g) + " "
        + std::to_string(b) + " "
        + std::to_string(opacity);

    item->set_color(c);
    item->set_opacity(opacity);

    gutilities::apply_uimanager_commands(command);
}
