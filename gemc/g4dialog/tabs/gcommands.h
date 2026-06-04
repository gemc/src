#pragma once

// qt
#include <QtWidgets>

// G4 headers
#include "G4UImanager.hh"
#include "G4UIcommandTree.hh"

/**
 * \class G4Commands
 * \brief Qt widget that exposes a searchable \c Geant4 UI command tree with contextual help.
 *
 * \details
 * This widget provides three main capabilities:
 * - **Discovery**: a hierarchical tree view of the \c Geant4 UI command structure.
 * - **Assistance**: contextual help for the currently selected command or command directory.
 * - **Execution**: a command prompt with a history list for quick recall.
 *
 * The widget is typically embedded as a tab inside G4Dialog, but it can be used independently
 * anywhere a \c QWidget is accepted.
 *
 * Internally the command tree is built by querying \c G4UImanager and walking the \c G4UIcommandTree.
 * Filtering is applied by recursively checking whether each tree node matches the search string
 * or contains matching descendants.
 */
class G4Commands : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Construct the commands widget.
	 *
	 * \param parent Parent widget, or nullptr to create a top-level widget.
	 */
	explicit G4Commands(QWidget* parent = nullptr);

private:
	// --- UI elements ---
	QLineEdit*   w_search   = nullptr; ///< Search field used to filter the command tree.
	QTreeView*   w_commands = nullptr; ///< Tree view showing the \c Geant4 command directories and commands.
	QTextEdit*   w_help     = nullptr; ///< Read-only text area showing contextual help for selections.
	QListWidget* w_history  = nullptr; ///< History list of executed commands (deduplicated).
	QLineEdit*   w_command  = nullptr; ///< Command entry line; hitting Enter executes the command.

	/**
	 * \brief Recursively populate the Qt tree model from a \c Geant4 command tree node.
	 *
	 * \details
	 * This method appends:
	 * - Child directories (sub-trees) as intermediate nodes.
	 * - Commands at the current node as leaf items.
	 *
	 * \param aParent Qt model item that will receive children.
	 * \param aCommandTree \c Geant4 tree node to traverse.
	 */
	void create_child_help_tree(QStandardItem* aParent, G4UIcommandTree* aCommandTree);

	/**
	 * \brief Build a formatted help text for a given \c Geant4 command.
	 *
	 * \details
	 * The resulting text includes:
	 * - Command path
	 * - Guidance lines
	 * - Parameter information (name, guidance, type, optional/default values, candidates, ranges)
	 *
	 * \param aCommand Command to describe.
	 * \return A Qt string suitable for display in the help text widget.
	 */
	QString get_command_g4help(const G4UIcommand* aCommand);

	/**
	 * \brief Slot handler that applies the current search string to the command tree view.
	 *
	 * \details
	 * This iterates the top-level items and hides or shows rows based on whether the node matches
	 * the filter or contains any matching descendants.
	 */
	void filterTreeItems();

	/**
	 * \brief Recursively filter a model item and its children.
	 *
	 * \details
	 * A node is considered visible if:
	 * - Its text contains \p search_text (case-insensitive), or
	 * - Any descendant node matches.
	 *
	 * \param item Current node to evaluate.
	 * \param search_text The normalized search string.
	 * \return True if the node should be visible, false otherwise.
	 */
	bool filterItem(QStandardItem* item, const QString& search_text);

private slots:
	/**
	 * \brief Execute the command currently present in the command entry field.
	 *
	 * \details
	 * - Trims whitespace.
	 * - Calls \c G4UImanager::ApplyCommand.
	 * - Updates history (deduplicated).
	 * - Clears the command field after execution.
	 */
	void execute_command();

	/**
	 * \brief Recall a history entry by double-clicking.
	 *
	 * \param item The clicked history list item.
	 */
	void recall_history_item_on_double_click(QListWidgetItem* item);

	/**
	 * \brief Display help corresponding to the current selection in the command tree view.
	 *
	 * \details
	 * If the selection corresponds to:
	 * - A command: display detailed command help.
	 * - A directory: display the directory title.
	 */
	void display_help_from_selection();

	/**
	 * \brief Paste the current selection into the command entry line.
	 *
	 * \details
	 * This also refreshes the help pane to match the current selection.
	 */
	void paste_help_selection_item();

	/**
	 * \brief Create the command tree widget and connect selection signals to help/paste actions.
	 */
	void create_geant4_commands_widget();
};
