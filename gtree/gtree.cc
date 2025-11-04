// Qt
#include <QHeaderView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QColorDialog>

// gtree
#include "gtree.h"
#include "gtree_options.h"

// gemc
#include "gsystemConventions.h"

// geant4
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4UImanager.hh" // Geant4 UI manager access
#include "gtouchable.h"
#include "gutilities.h"

G4Ttree_item::G4Ttree_item(G4Volume* g4volume) {
    auto pvolume = g4volume->getPhysical();
    auto lvolume = g4volume->getLogical();

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
    double green =gcolor.GetGreen();
    double blue = gcolor.GetBlue();
    auto alpha = gcolor.GetAlpha();

    color = QColor::fromRgbF(red, green, blue);

    opacity = alpha;

    is_visible = visAttributes->IsVisible();


    // auto representation = visAttributes->GetRepresentation();
    // auto opacity = visAttributes->GetOpacity();
    // auto density = pvolume->GetDensity();
    // auto mass = pvolume->GetMass();
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
    headers << "Visible" << "Color" << "Volume";
    treeWidget->setHeaderLabels(headers);
    treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    treeWidget->setRootIsDecorated(true);
    treeWidget->setAlternatingRowColors(true);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(treeWidget);
    setLayout(layout);

    // populate the tree from g4_systems_tree
    populateTree();

    // react to checkboxes
    connect(treeWidget, &QTreeWidget::itemChanged,
            this, &GTree::onItemChanged);

    log->debug(NORMAL, SFUNCTION_NAME, "GTree added");
}


void GTree::populateTree() {
    // top-level root
    auto* rootItem = new QTreeWidgetItem(treeWidget);
    rootItem->setText(2, "root");
    rootItem->setFlags(rootItem->flags()  | Qt::ItemIsUserCheckable);
    rootItem->setCheckState(0, Qt::Checked);
    treeWidget->addTopLevelItem(rootItem);

    // for each system
    for (auto& [systemName, volMap] : g4_systems_tree) {

        auto* systemItem = new QTreeWidgetItem(rootItem);
        systemItem->setText(2, QString::fromStdString(systemName));
        systemItem->setFlags(systemItem->flags() | Qt::ItemIsUserCheckable);
        systemItem->setCheckState(0, Qt::Checked);

        // we need to build the volume hierarchy inside this system
        // since we have mother-child relationships, we will first create items
        // and remember them by volume name
        std::map<std::string, QTreeWidgetItem*> itemLookup;

        // first pass: create items (not yet parented properly)
        for (auto& [volName, vptr] : volMap) {
            const G4Ttree_item* vitem = vptr.get();
            // create item and insert into lookup; we’ll parent properly below
            auto* dummyParent = systemItem; // temporary
            auto* item = new QTreeWidgetItem(dummyParent);
            item->setText(2, QString::fromStdString(G4Ttree_item::vname_from_v4name(volName)));
            item->setData(2, Qt::UserRole, QString::fromStdString(volName)); // store full v4 name

            // checkbox for visibility:
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, vitem->get_visibility() ? Qt::Checked : Qt::Unchecked);

            // color button:
            auto* colorBtn = new QPushButton(this);
            QColor c = vitem->get_color();
            colorBtn->setFixedSize(20, 20);
            colorBtn->setFlat(true);        // no 3D/bevel look
            colorBtn->setText(QString());   // no text

            // Use stylesheet so the *entire* button is filled
            colorBtn->setStyleSheet(
                QString("QPushButton { background-color: %1; border: 1px solid black; }")
                    .arg(c.name())
            );


            // store volume name on button so we know which to update
            colorBtn->setProperty("volumeName", QString::fromStdString(volName));
            connect(colorBtn, &QPushButton::clicked, this, &GTree::onColorButtonClicked);

            treeWidget->setItemWidget(item, 1, colorBtn);

            itemLookup[volName] = item;
        }

        // second pass: re-parent according to mother
        for (auto& [volName, vptr] : volMap) {
            const G4Ttree_item* vitem = vptr.get();
            auto mother = vitem->get_mother(); // full v4 name of mother
            auto* thisItem = itemLookup[volName];

            if (mother.empty() || mother == "root") {
                // already under systemItem – OK
                continue;
            }

            auto itM = itemLookup.find(mother);
            if (itM != itemLookup.end()) {
                // move this item under mother
                QTreeWidgetItem* motherItem = itM->second;
                // remove from current parent
                QTreeWidgetItem* oldParent = thisItem->parent();
                if (oldParent) {
                    oldParent->removeChild(thisItem);
                }
                motherItem->addChild(thisItem);
            }
            else {
                // mother not found in this system — leave it under system
            }
        }
    }

    treeWidget->expandAll();
}

void GTree::build_tree(std::unordered_map<std::string, G4Volume*> g4volumes_map) {
    // loop over map
    for (auto [name, g4volume] : g4volumes_map) {
        auto system_name = G4Ttree_item::system_from_v4name(name);
        log->info(2, "gtree", "Adding ", name, " to tree, system_name is ", system_name);

        // ensure the system exists
        auto& system_tree = g4_systems_tree[system_name];
        system_tree[name] = std::make_unique<G4Ttree_item>(g4volume);
    }
}

void GTree::onItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0) return; // we care about visibility column only

    // only volume items will have UserRole data with full name
    QVariant v = item->data(2, Qt::UserRole);
    if (!v.isValid()) {
        // could be root or system: propagate to children if you want
        return;
    }

    const QString fullName = v.toString();
    bool visible = (item->checkState(0) == Qt::Checked);
    set_visibility(fullName.toStdString(), visible);
}


void GTree::onColorButtonClicked()
{
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


void GTree::set_visibility(const std::string& volumeName, bool visible)
{
    G4UImanager* g4uim = G4UImanager::GetUIpointer();
    if (g4uim == nullptr) { return; }

    std::string vis_int = visible ? "1" : "0";

    std::string command = "/vis/geometry/set/visibility " + volumeName + " 0 " + vis_int;


    g4uim->ApplyCommand(command);
}

void GTree::set_color(const std::string& volumeName, const QColor& c)
{
    G4UImanager* g4uim = G4UImanager::GetUIpointer();
    if (g4uim == nullptr) { return; }

    int r, g, b;
    c.getRgb(&r, &g, &b);

    std::string command = "/vis/geometry/set/colour " + volumeName + " 0 "
    + std::to_string(r/255.0) + " "
    + std::to_string(g/255.0) + " "
    + std::to_string(b/255.0);

    g4uim->ApplyCommand(command);

}
