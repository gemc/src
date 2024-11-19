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
    //  | |    > Prompt       | |
    //  | +-------------------+ |
    //  +-----------------------+

    // search
    w_search = new QLineEdit();
    w_search->installEventFilter(this);
    w_search->activateWindow();
    w_search->setFocusPolicy(Qt::StrongFocus);
    w_search->setFocus(Qt::TabFocusReason);

    // every time w_search is changed, filter the tree
    connect(w_search, SIGNAL(textChanged(const QString &)), this, SLOT(create_geant4_commands_widget()));

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
    connect(w_history, SIGNAL(itemSelectionChanged()), SLOT(recall_history_item()));

    w_command = new QLineEdit();
    w_command->installEventFilter(this);
    w_command->activateWindow();
    w_command->setFocusPolicy(Qt::StrongFocus);
    w_command->setFocus(Qt::TabFocusReason);
    connect(w_command, SIGNAL(returnPressed()), SLOT(execute_command()));

    // putting all together
    commands_help_splitter->setSizes(QList<int>() << 600 << 200);

    QVBoxLayout *v_layout = new QVBoxLayout(parent);
    v_layout->addWidget(new QLabel("Search Commands"));
    v_layout->addWidget(w_search);
    v_layout->addWidget(commands_help_splitter, /* stretch factor */  2);
    v_layout->addWidget(new QLabel("History"));
    v_layout->addWidget(w_history);
    v_layout->addWidget(new QLabel("Enter Command"));
    v_layout->addWidget(w_command);

}

void G4Commands::create_geant4_commands_widget() {
    // Print search text on screen for debugging
    QString search_text = w_search->text();

    G4UImanager *ui_manager = G4UImanager::GetUIpointer();
    G4UIcommandTree *g4_commands_tree = ui_manager->GetTree();

    // if w_commands is not null, clear it
    if (w_commands) {
        w_commands->clear();
    } else {
        w_commands = new QTreeWidget();
    }

    w_commands->setSelectionMode(QAbstractItemView::SingleSelection);

    QStringList labels;
    labels << QString("Commands");
    w_commands->setHeaderLabels(labels);

    // Add commands to the tree
    G4int g4_commands_tree_size = g4_commands_tree->GetTreeEntry();
    for (int a = 0; a < g4_commands_tree_size; a++) {
        QTreeWidgetItem *newItem = new QTreeWidgetItem(w_commands);
        QString pathName = QString((char *) g4_commands_tree->GetTree(a + 1)->GetPathName().data());
        newItem->setText(0, pathName.trimmed());

        // Add child commands
        create_child_help_tree(newItem, g4_commands_tree->GetTree(a + 1));
    }

    // Now reset the tree and only display items that include the search text
    if (!search_text.isEmpty()) {
        filterTreeItems(w_commands, search_text);
        // expand all items
        w_commands->expandAll();
    }

    connect(w_commands, SIGNAL(itemSelectionChanged()), this, SLOT(display_help_from_selection()));
    connect(w_commands, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(paste_help_selection_item()));
}

// Recursive function to filter items based on search text
void G4Commands::filterTreeItems(QTreeWidget *tree, const QString &search_text) {
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = tree->topLevelItem(i);
        if (!filterItem(item, search_text)) {
            delete tree->takeTopLevelItem(tree->indexOfTopLevelItem(item));
            --i; // Adjust the index after removal
        }
    }
}

bool G4Commands::filterItem(QTreeWidgetItem *item, const QString &search_text) {
    bool containsText = item->text(0).contains(search_text, Qt::CaseInsensitive);

    // Check if any child items contain the search text
    for (int i = 0; i < item->childCount(); ++i) {
        if (filterItem(item->child(i), search_text)) {
            containsText = true;
        } else {
            delete item->takeChild(i);
            --i; // Adjust the index after removal
        }
    }

    return containsText;
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
void G4Commands::recall_history_item() {
    QTreeWidgetItem *item = nullptr;
    if (!w_commands || !w_help)
        return;

    QList < QTreeWidgetItem * > list = w_commands->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    item = list.first();
    if (!item) {
        return;
    }

    G4UIcommandTree *treeTop = G4UImanager::GetUIpointer()->GetTree();

    string itemText = item->text(0).toStdString();
    G4UIcommand *command = treeTop->FindPath(itemText.c_str());

    // if it's a valid command, display the help
    if (command) {
        w_help->setText(get_command_g4help(command));
    } else {
        // this is not a command, may be a subdir
        G4UIcommandTree *path = treeTop->FindCommandTree(itemText.c_str());
        if (path) {
            // subdir, display the title
            w_help->setText(path->GetTitle().data());
        }
    }
}

// paste history item onto command line
void G4Commands::paste_help_selection_item() {
    display_help_from_selection();

    QTreeWidgetItem *item = nullptr;

    if (!w_commands || !w_help) { return; }

    QList < QTreeWidgetItem * > list = w_commands->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    item = list.first();

    if (!item) {
        return;
    }

    w_command->clear();
    w_command->setText(item->text(0));

}

// display help on selected item
void G4Commands::display_help_from_selection() {
    QTreeWidgetItem *item = nullptr;
    if (!w_commands || !w_help) { return; }

    QList < QTreeWidgetItem * > list = w_commands->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    item = list.first();
    if (!item) {
        return;
    }

    G4UIcommandTree *treeTop = G4UImanager::GetUIpointer()->GetTree();

    string itemText = item->text(0).toStdString();
    G4UIcommand *command = treeTop->FindPath(itemText.c_str());

    // if it's a valid command, display the help
    if (command) {
        w_help->setText(get_command_g4help(command));
    } else {
        // this is not a command, may be a subdir
        G4UIcommandTree *path = treeTop->FindCommandTree(itemText.c_str());
        if (path) {
            // subdir, display the title
            w_help->setText(path->GetTitle().data());
        }
    }
}

QString G4Commands::get_command_g4help(const G4UIcommand *aCommand) {

    QString txt = "";
    if (aCommand == nullptr)
        return txt;

    G4String commandPath = aCommand->GetCommandPath();
    G4String rangeString = aCommand->GetRange();
    G4int n_guidanceEntry = aCommand->GetGuidanceEntries();
    G4int n_parameterEntry = aCommand->GetParameterEntries();

    if ((commandPath == "") && (rangeString == "") && (n_guidanceEntry == 0) && (n_parameterEntry == 0)) {
        return txt;
    }

    if ((commandPath.length() - 1) != '/') {
        txt += "Command " + QString((char *) (commandPath).data()) + "\n";
    }
    txt += "Guidance :\n";

    for (G4int i_thGuidance = 0; i_thGuidance < n_guidanceEntry; i_thGuidance++) {
        txt += QString((char *) (aCommand->GetGuidanceLine(i_thGuidance)).data()) + "\n";
    }
    if (rangeString != "") {
        txt += " Range of parameters : " + QString((char *) (rangeString).data()) + "\n";
    }
    if (n_parameterEntry > 0) {
        G4UIparameter *param;

        // Re-implementation from G4UIparameter.cc
        for (G4int i_thParameter = 0; i_thParameter < n_parameterEntry; i_thParameter++) {
            param = aCommand->GetParameter(i_thParameter);
            txt += "\nParameter : " + QString((char *) (param->GetParameterName()).data()) + "\n";
            if (param->GetParameterGuidance() != "") { txt += QString((char *) (param->GetParameterGuidance()).data()) + "\n"; }
            txt += " Parameter type  : " + QString(QChar(param->GetParameterType())) + "\n";
            if (param->IsOmittable()) {
                txt += " Omittable       : True\n";
            } else {
                txt += " Omittable       : False\n";
            }

            if (param->GetCurrentAsDefault()) {
                txt += " Default value   : taken from the current value\n";
            } else if (param->GetDefaultValue() != "") {
                txt += " Default value   : " + QString((char *) (param->GetDefaultValue()).data()) + "\n";
            }

            if (param->GetParameterRange() != "") {
                txt += " Parameter range : " + QString((char *) (param->GetParameterRange()).data()) + "\n";
            }

            if (param->GetParameterCandidates() != "") {
                txt += " Candidates      : " + QString((char *) (param->GetParameterCandidates()).data()) + "\n";
            }
        }
    }
    return txt;
}
