#ifndef GDIALOG_COMMANDS_H
#define GDIALOG_COMMANDS_H 1

// qt
#include <QtWidgets>

// gemc
#include "goptions.h"

// G4 headers
#include "G4UImanager.hh"
#include "G4UIcommandTree.hh"

class G4Commands : public QTabWidget {

    Q_OBJECT

public:
    G4Commands(GOptions *gopt, QWidget *parent = Q_NULLPTR);

private:
    QLineEdit *w_search;
    QTreeWidget *w_commands;
    QTextEdit *w_help;
    QListWidget *w_history;
    QLineEdit *w_command;

    void create_child_help_tree(QTreeWidgetItem *aParent, G4UIcommandTree *aCommandTree);

    // gets the G4 help on a command
    QString get_command_g4help (const G4UIcommand *aCommand);
    void filterTreeItems(QTreeWidget *tree, const QString &search_text);
    bool filterItem(QTreeWidgetItem *item, const QString &search_text);

private slots:

     void execute_command();

    void recall_history_item();

    void display_help_from_selection();

    void paste_help_selection_item();

    void create_geant4_commands_widget();

};

#endif
