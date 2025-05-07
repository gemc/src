#pragma once

// qt
#include <QtWidgets>

// G4 headers
#include "G4UImanager.hh"
#include "G4UIcommandTree.hh"

class G4Commands : public QWidget {

    Q_OBJECT

public:
    G4Commands(QWidget *parent = nullptr);

private:
    QLineEdit *w_search;
	QTreeView *w_commands;
    QTextEdit *w_help;
    QListWidget *w_history;
    QLineEdit *w_command;

    void create_child_help_tree(QStandardItem *aParent, G4UIcommandTree *aCommandTree);

    // gets the G4 help on a command
    QString get_command_g4help (const G4UIcommand *aCommand);
    void filterTreeItems();
	bool filterItem(QStandardItem *item, const QString &search_text);

private slots:

	void execute_command();

	void recall_history_item_on_double_click(QListWidgetItem *item);

	void display_help_from_selection();

    void paste_help_selection_item();

    void create_geant4_commands_widget();

};
