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


    void create_geant4_commands_widget();

    void create_child_help_tree(QTreeWidgetItem *aParent, G4UIcommandTree *aCommandTree);

private slots:

    void execute_command();

    void execute_history_item();

};

#endif
