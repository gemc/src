// G4Dialog
#include "gcommands.h"



G4Commands::G4Commands(GOptions *gopt, QWidget *parent) : QTabWidget(parent) {

    //  Layout:
    //
    //  + +-------------------+ +
    //  | |    > Search       | |
    //  + +-------------------+ +
    //  | |         |         | |
    //  | |  Tree   |  Help   | |
    //  | |         |         | |
    //  | +-------------------+ |
    //  | +-------------------+ |
    //  | |                   | |
    //  | |      History      | |
    //  | |                   | |
    //  | +-------------------+ |
    //  | +-------------------+ |
    //  | |    > Promt        | |
    //  | +-------------------+ |
    //  +-----------------------+


    // search
    w_search = new QLineEdit();
    w_search->installEventFilter(this);
    w_search->activateWindow();
    w_search->setFocusPolicy(Qt::StrongFocus);
    w_search->setFocus(Qt::TabFocusReason);

    // commands tree and help
    QSplitter *commands_help_splitter = new QSplitter(Qt::Horizontal);
    QVBoxLayout *commands_help_layout = new QVBoxLayout(commands_help_splitter);

    // Left: the commands tree
    create_geant4_commands_widget();
    commands_help_layout->addWidget(w_commands);

    // Right: the help on individual commands
    w_help = new QTextEdit(commands_help_splitter);
    w_help->setReadOnly(true);

    // history area
    w_history = new QListWidget();
    w_history->setSelectionMode(QAbstractItemView::SingleSelection);
    w_history->installEventFilter(this);
    connect(w_history, SIGNAL(itemSelectionChanged()), SLOT(execute_history_item()));

    w_command = new QLineEdit();
    w_command->installEventFilter(this);
    w_command->activateWindow();
    w_command->setFocusPolicy ( Qt::StrongFocus );
    w_command->setFocus(Qt::TabFocusReason);
    connect(w_command, SIGNAL(returnPressed()), SLOT(execute_command()));

    // putting all together
    QVBoxLayout *v_layout = new QVBoxLayout(parent);
    v_layout->addWidget(new QLabel("Search Commands"));
    v_layout->addWidget(w_search);
    v_layout->addWidget(commands_help_splitter);
    v_layout->addWidget(new QLabel("History"));
    v_layout->addWidget(w_history);
    v_layout->addWidget(new QLabel("Enter Command"));
    v_layout->addWidget(w_command);

}

void G4Commands::create_geant4_commands_widget() {

    G4UImanager *ui_manager = G4UImanager::GetUIpointer();
    G4UIcommandTree *g4_commands_tree = ui_manager->GetTree();

    // build widget
    w_commands = new QTreeWidget();
    w_commands->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList labels;
    labels << QString("Commands");
    w_commands->setHeaderLabels(labels);

    G4int g4_commands_tree_size = g4_commands_tree->GetTreeEntry();
    QTreeWidgetItem *newItem;

    for (int a = 0; a < g4_commands_tree_size; a++) {
        // Creating new item
        newItem = new QTreeWidgetItem(w_commands);
        newItem->setText(0, QString((char *) (g4_commands_tree->GetTree(a + 1)->GetPathName()).data()).trimmed());

        // look for childs
        create_child_help_tree(newItem, g4_commands_tree->GetTree(a + 1));
    }


//    connect(fHelpTreeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(HelpTreeClicCallback()));
//    connect(fHelpTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem * , int)), this, SLOT(HelpTreeDoubleClicCallback()));
}

void G4Commands::create_child_help_tree(QTreeWidgetItem *aParent, G4UIcommandTree *aCommandTree) {
    if (aParent == nullptr) return;
    if (aCommandTree == nullptr) return;

    // Creating new item
    QTreeWidgetItem *newItem;

    // Get the Sub directories
    for (int a = 0; a < aCommandTree->GetTreeEntry(); a++) {

        newItem = new QTreeWidgetItem(aParent);
        newItem->setText(0, QString((char *) (aCommandTree->GetTree(a + 1)->GetPathName()).data()).trimmed());

        create_child_help_tree(newItem, aCommandTree->GetTree(a + 1));
    }

    // Get the Commands
    for (int a = 0; a < aCommandTree->GetCommandEntry(); a++) {
        newItem = new QTreeWidgetItem(aParent);
        newItem->setText(0, QString((char *) (aCommandTree->GetCommand(a + 1)->GetCommandPath()).data()).trimmed());
    }
}

void G4Commands::execute_command() {
    if (!w_command) {
        return;
    }

    QString command = w_command->text();
    if (command.isEmpty()) {
        return;
    }

    G4UImanager *ui_manager = G4UImanager::GetUIpointer();
    ui_manager->ApplyCommand(command.toStdString().c_str());

    w_history->addItem(command);
    w_command->clear();
}

// execute history item
void G4Commands::execute_history_item()
{
    QListWidgetItem* item =  nullptr;
    if (!w_history) {
        return;
    }

    QList<QListWidgetItem *> list = w_history->selectedItems();
    if(list.isEmpty()) {
        return;
    }

    item = list.first();
    if(!item) {
        return;
    }
    w_command->setText(item->text());

}
