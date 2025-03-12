// cpp
using namespace std;

// G4Dialog
#include "gcommands.h"

G4Commands::G4Commands(QWidget *parent) : QTabWidget(parent) {

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
	connect(w_search, &QLineEdit::textChanged, this, &G4Commands::filterTreeItems);

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
	connect(w_history, &QListWidget::itemDoubleClicked, this, &G4Commands::recall_history_item_on_double_click);

	w_command = new QLineEdit();
	w_command->installEventFilter(this);
	w_command->activateWindow();
	w_command->setFocusPolicy(Qt::StrongFocus);
	w_command->setFocus(Qt::TabFocusReason);
	connect(w_command, &QLineEdit::returnPressed, this, &G4Commands::execute_command);

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

	// Create model for QTreeView
	QStandardItemModel *model = new QStandardItemModel();
	w_commands = new QTreeView();
	w_commands->setModel(model);
	w_commands->setSelectionMode(QAbstractItemView::SingleSelection);

	// Set header label for the QTreeView model
	model->setHorizontalHeaderLabels(QStringList() << "Commands");

	// Add commands to the model
	G4int g4_commands_tree_size = g4_commands_tree->GetTreeEntry();
	for (int a = 0; a < g4_commands_tree_size; a++) {
		QStandardItem *newItem = new QStandardItem(
				QString((char *) g4_commands_tree->GetTree(a + 1)->GetPathName().data()).trimmed());
		model->appendRow(newItem);

		// Add child commands
		create_child_help_tree(newItem, g4_commands_tree->GetTree(a + 1));
	}


	connect(w_commands->selectionModel(), &QItemSelectionModel::selectionChanged, this,
			&G4Commands::display_help_from_selection);
	connect(w_commands, &QTreeView::doubleClicked, this, &G4Commands::paste_help_selection_item);
}


void G4Commands::filterTreeItems() {
	QString search_text = w_search->text().trimmed();
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(w_commands->model());
	if (!model) return;

	for (int i = 0; i < model->rowCount(); ++i) {
		QStandardItem *item = model->item(i);
		bool showItem = filterItem(item, search_text);
		w_commands->setRowHidden(i, QModelIndex(), !showItem);
	}
}


bool G4Commands::filterItem(QStandardItem *item, const QString &search_text) {
	bool matches = item->text().contains(search_text, Qt::CaseInsensitive);
	bool childMatches = false;

	// Check child items recursively
	for (int i = 0; i < item->rowCount(); ++i) {
		QStandardItem *childItem = item->child(i);
		if (filterItem(childItem, search_text)) {
			childMatches = true;
		}
	}

	// Show this item if it matches or has a matching child
	bool showItem = matches || childMatches;
	w_commands->setRowHidden(item->row(), item->index().parent(), !showItem);

	return showItem;
}


void G4Commands::create_child_help_tree(QStandardItem *parent, G4UIcommandTree *aCommandTree) {
	if (parent == nullptr || aCommandTree == nullptr) return;

	// Add child directories
	for (int a = 0; a < aCommandTree->GetTreeEntry(); a++) {
		QStandardItem *newItem = new QStandardItem(
				QString((char *) (aCommandTree->GetTree(a + 1)->GetPathName()).data()).trimmed());
		parent->appendRow(newItem);
		create_child_help_tree(newItem, aCommandTree->GetTree(a + 1));
	}

	// Add commands
	for (int a = 0; a < aCommandTree->GetCommandEntry(); a++) {
		QStandardItem *newItem = new QStandardItem(
				QString((char *) (aCommandTree->GetCommand(a + 1)->GetCommandPath()).data()).trimmed());
		parent->appendRow(newItem);
	}
}


void G4Commands::execute_command() {
	if (!w_command) return;

	QString command = w_command->text().trimmed();
	if (command.isEmpty()) return;

	G4UImanager *ui_manager = G4UImanager::GetUIpointer();
	ui_manager->ApplyCommand(command.toStdString().c_str());

	// Avoid duplicate history entries
	bool exists = false;
	for (int i = 0; i < w_history->count(); ++i) {
		if (w_history->item(i)->text() == command) {
			exists = true;
			break;
		}
	}

	if (!exists) {
		w_history->addItem(command);
	}

	w_command->clear();
}




void G4Commands::recall_history_item_on_double_click(QListWidgetItem *item) {
	if (!item) return;

	// Set the command line input to the selected history item
	w_command->setText(item->text());
}


// paste history item onto command line
void G4Commands::display_help_from_selection() {
	if (!w_commands || !w_help) return;

	QModelIndexList selectedIndexes = w_commands->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty()) return;

	QModelIndex index = selectedIndexes.first();
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(w_commands->model());
	if (!model) return;

	QStandardItem *item = model->itemFromIndex(index);
	if (!item) return;

	std::string itemText = item->text().toStdString();
	G4UIcommandTree *treeTop = G4UImanager::GetUIpointer()->GetTree();
	G4UIcommand *command = treeTop->FindPath(itemText.c_str());

	if (command) {
		w_help->setText(get_command_g4help(command));
	} else {
		G4UIcommandTree *path = treeTop->FindCommandTree(itemText.c_str());
		if (path) {
			w_help->setText(QString::fromStdString(path->GetTitle()));
		}
	}
}


// display help on selected item
void G4Commands::paste_help_selection_item() {
	// Display help from the selection
	display_help_from_selection();

	if (!w_commands || !w_help) {
		return;
	}

	// Get the selected items from the selection model of QTreeView
	QModelIndexList selectedIndexes = w_commands->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty()) {
		return;
	}

	// Assuming the first selected item is the one to retrieve
	QModelIndex index = selectedIndexes.first();
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(w_commands->model());
	if (!model) {
		return;
	}

	// Retrieve the item at the given index
	QStandardItem *item = model->itemFromIndex(index);
	if (!item) {
		return;
	}

	// Clear the command input field and set the selected item's text
	w_command->clear();
	w_command->setText(item->text());
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
			if (param->GetParameterGuidance() != "") {
				txt += QString((char *) (param->GetParameterGuidance()).data()) + "\n";
			}
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
