#pragma once

// gemc
#include "gdetectorConstruction.h"
#include "gbase.h"

// qt
#include <QWidget>
#include <QTreeView>
#include <QStandardItem>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QStyledItemDelegate>

// c++
#include <sqlite3.h>
#include <string>
#include <vector>


/**
 * @ingroup dbselect_module
 *
 * @brief Qt widget used to select experiment/system configurations from an SQLite geometry database.
 *
 * \c DBSelectView displays a tree of experiments and their systems obtained from an SQLite database.
 * Each system row allows:
 * - enabling/disabling the system via a checkbox,
 * - selecting a variation from a drop-down list,
 * - selecting a run number from a drop-down list,
 * - viewing the number of matching geometry entries for the current selection,
 * - seeing an availability indicator (green/red icon) driven by database counts.
 *
 * The widget is designed to be used as a selection front-end for geometry reload:
 * - \ref DBSelectView::get_gsystems "get_gsystems()" builds a SystemList reflecting the checked systems.
 * - \ref DBSelectView::reload_geometry "reload_geometry()" triggers detector construction reload using that list.
 *
 * Ownership model:
 * - The widget owns its Qt child widgets through Qt parent-child relationships.
 * - The widget owns and closes the \c sqlite3 database handle opened read-only.
 * - The widget does not own the provided \c GDetectorConstruction pointer.
 *
 * Important Qt note:
 * GBase must appear after QWidget derivations; otherwise Qt's MOC expects a staticMetaObject.
 */
class DBSelectView : public QWidget, public GBase<DBSelectView> {
	Q_OBJECT
public:
	/**
	 * @brief Construct the view and populate the experiment/system model from the database.
	 *
	 * Construction steps (high-level):
	 * - Read the database location and default experiment from options.
	 * - Resolve the database path via a search in common locations and open it read-only.
	 * - Validate that the database contains a non-empty \c geometry table.
	 * - Create the UI, populate the model, and apply existing GSystem selections.
	 *
	 * @param gopts Shared options used for database selection, default experiment, and UI mode.
	 * @param dc Detector construction instance used to reload geometry on user request.
	 * @param parent Optional parent widget for Qt ownership.
	 */
	explicit DBSelectView(const std::shared_ptr<GOptions>& gopts,
	                      GDetectorConstruction* dc,
	                      QWidget* parent = nullptr);

	/**
	 * @brief Destructor.
	 *
	 * Closes the \c sqlite3 handle if it was successfully opened.
	 * Qt child widgets are cleaned up automatically by Qt parent ownership.
	 */
	~DBSelectView() override { if (db) sqlite3_close(db); }

	/**
	 * @brief Build and return the list of selected systems as a SystemList.
	 *
	 * The returned SystemList contains one \c GSystem for each checked system item in the model.
	 * For each enabled system, the selected variation and run are taken from the corresponding
	 * model columns.
	 *
	 * This function does not change the UI state; it is a pure extraction step.
	 *
	 * @return A SystemList representing the currently checked systems and their selections.
	 */
	SystemList get_gsystems();

private:
	/**
	 * @brief Create the full UI layout and connect model signals.
	 *
	 * Creates:
	 * - a header area with a title label and an experiment summary label,
	 * - a Reload button,
	 * - a \c QTreeView backed by a \c QStandardItemModel with four columns
	 *   (exp/system, volumes, variation, run),
	 * - drop-down delegates for variation and run columns.
	 */
	void setupUI();

	/**
	 * @brief Query the database for experiments and populate the top-level model items.
	 *
	 * Each experiment is added as a top-level, checkable item, and its systems are appended
	 * as child rows by querying the database.
	 */
	void loadExperiments();

	/**
	 * @brief Query the database for systems belonging to the current experiment and append them as children.
	 *
	 * Child rows are created with:
	 * - system item (checkable),
	 * - entry count item (filled later),
	 * - variation item with an editable value and the full list stored in \c Qt::UserRole,
	 * - run item with an editable value and the full list stored in \c Qt::UserRole.
	 *
	 * @param experimentItem The model item that represents the experiment parent row.
	 */
	void loadSystemsForExperiment(QStandardItem* experimentItem);

	/**
	 * @brief Retrieve available variations for a given system.
	 *
	 * Variations are collected from \c SELECT DISTINCT variation FROM geometry WHERE system = ?.
	 *
	 * @param system System name key used in the database.
	 * @return List of available variation strings.
	 */
	QStringList getAvailableVariations(const std::string& system) const;

	/**
	 * @brief Retrieve available runs for a given system.
	 *
	 * Runs are collected from \c SELECT DISTINCT run FROM geometry WHERE system = ?.
	 *
	 * @param system System name key used in the database.
	 * @return List of available run values converted to strings.
	 */
	QStringList getAvailableRuns(const std::string& system);

	/**
	 * @brief Count matching geometry rows for a selection tuple.
	 *
	 * The count is used to populate the “volumes” column and to determine whether the
	 * system selection is “available”.
	 *
	 * @param system System name.
	 * @param variation Variation name.
	 * @param run Run number.
	 * @return Number of matching rows in the \c geometry table.
	 */
	int getGeometryCount(const std::string& system, const std::string& variation, int run) const;

	/**
	 * @brief Update a system item’s icon and related “volumes” column based on current selection.
	 *
	 * Uses the current experiment, variation, and run associated with the row to compute:
	 * - entry count (shown in column 1),
	 * - an availability icon (green when count > 0, red otherwise).
	 *
	 * @param systemItem The system item in column 0 for the row to update.
	 */
	void updateSystemItemAppearance(QStandardItem* systemItem);

	/**
	 * @brief Check whether a tuple (system, variation, run) exists in the database.
	 *
	 * This is a boolean convenience query and may be used to gate UI states.
	 *
	 * @param system System name.
	 * @param variation Variation name.
	 * @param run Run number.
	 * @return True if at least one matching row exists, false otherwise.
	 */
	bool systemAvailable(const std::string& system, const std::string& variation, int run);

	/**
	 * @brief Update the experiment header label summarizing the selected experiment.
	 *
	 * The header label typically reports the selected experiment and a total system count.
	 */
	void updateExperimentHeader();

	/**
	 * @brief Update the “modified” visual state (title label and reload button state).
	 *
	 * When the model changes, the view is considered modified until the user triggers
	 * a reload. The title text is updated to reflect the state and the reload button is
	 * enabled/disabled accordingly.
	 */
	void updateModifiedUI();

	/**
	 * @brief Create a small square icon filled with a given color for status display.
	 *
	 * @param color Fill color to use.
	 * @return A \c QIcon containing a colored square pixmap.
	 */
	QIcon createStatusIcon(const QColor& color);

	/**
	 * @brief Validate that the database contains a usable \c geometry table.
	 *
	 * The validation checks:
	 * - the \c geometry table exists,
	 * - the \c geometry table contains at least one row.
	 *
	 * @return True if the table exists and is non-empty, false otherwise.
	 */
	bool isGeometryTableValid() const;

	/**
	 * @brief Apply current GSystem selections from options to the UI model.
	 *
	 * The current system selection vector is obtained from the configuration and used to:
	 * - check matching systems,
	 * - set variation/run choices to configured values when possible,
	 * - update the UI appearance (counts and icons).
	 *
	 * This is typically called once during construction after the model is populated.
	 */
	void applyGSystemSelections();

private:
	/// True when the model has been edited by the user and not yet reloaded.
	bool modified = false;

	/// Title label shown at the top of the widget (updated to indicate modification state).
	QLabel* titleLabel = nullptr;

	/// Reload button used to trigger geometry reload; enabled only when modified is true.
	QPushButton* reloadButton = nullptr;

	/// SQLite handle for the opened database (\c sqlite3_open_v2 read-only). Closed in destructor.
	sqlite3* db = nullptr;

	/// Database host/path option value (as provided by \c --sql).
	std::string dbhost;

	/// Default experiment name (as provided by \c --experiment) and/or current selection.
	std::string experiment;

	/// Tree view presenting experiments and systems.
	QTreeView* experimentTree = nullptr;

	/// Model backing the tree view. Column layout: exp/system, volumes, variation, run.
	QStandardItemModel* experimentModel = nullptr;

	/// Label presenting a short summary for the selected experiment (e.g. total systems).
	QLabel* experimentHeaderLabel = nullptr;

	/// Guard to prevent recursion when programmatically changing model items in itemChanged handler.
	bool m_ignoreItemChange = false;

	/// Detector construction used as an external service to reload geometry (not owned).
	GDetectorConstruction* gDetectorConstruction = nullptr;

	/// Shared options retained to rebuild GSystem selections on reload.
	std::shared_ptr<GOptions> gopt;

private slots:
	/**
	 * @brief Slot invoked when any model item changes.
	 *
	 * Responsibilities:
	 * - Enforce single-selection behavior for experiments (only one checked at a time).
	 * - Update system row appearance when system checkbox, variation, or run changes.
	 * - Mark the view as modified and refresh the header and layout.
	 *
	 * @param item The changed item in the model.
	 */
	void onItemChanged(QStandardItem* item);

public slots:
	/**
	 * @brief Slot invoked by the Reload button to reload geometry based on current selections.
	 *
	 * High-level behavior:
	 * - Build a SystemList from the model selections.
	 * - Invoke detector construction reload using that list.
	 * - Clear the modified state and refresh the UI.
	 */
	void reload_geometry();
};


//
// A custom delegate for drop-down editing (for both variations and runs).
//
/**
 * @ingroup dbselect_module
 *
 * @brief Item delegate that edits a cell using a \c QComboBox populated from \c Qt::UserRole.
 *
 * The model is expected to store a \c QStringList in \c Qt::UserRole for the edited index.
 * The delegate uses that list to populate the combo box and writes the selected value back
 * to \c Qt::EditRole.
 */
class ComboDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	/**
	 * @brief Construct the delegate.
	 * @param parent Optional Qt parent object.
	 */
	explicit ComboDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

	/**
	 * @brief Create a \c QComboBox editor for the given cell.
	 *
	 * The editor is created empty; content is populated in setEditorData().
	 *
	 * @param parent Parent widget for the editor.
	 * @param option Style options (unused).
	 * @param index Model index (unused).
	 * @return Newly created editor widget.
	 */
	QWidget* createEditor(QWidget* parent,
	                      const QStyleOptionViewItem& option,
	                      const QModelIndex& index) const override {
		Q_UNUSED(option);
		Q_UNUSED(index);
		auto editor = new QComboBox(parent);
		return editor;
	}

	/**
	 * @brief Populate the editor from the model.
	 *
	 * Reads \c Qt::UserRole as a \c QStringList, fills the combo box, and selects the
	 * current value from \c Qt::EditRole (if present).
	 *
	 * @param editor Editor widget previously created by createEditor().
	 * @param index Model index being edited.
	 */
	void setEditorData(QWidget* editor, const QModelIndex& index) const override {
		auto combo = qobject_cast<QComboBox*>(editor);
		if (!combo)
			return;

		// Retrieve list from UserRole.
		QVariant var     = index.model()->data(index, Qt::UserRole);
		QStringList opts = var.toStringList();

		combo->clear();
		combo->addItems(opts);

		QString currentText = index.model()->data(index, Qt::EditRole).toString();
		int idx = combo->findText(currentText);
		if (idx >= 0)
			combo->setCurrentIndex(idx);
	}

	/**
	 * @brief Write the selected editor value back to the model.
	 *
	 * The selected text is assigned to \c Qt::EditRole.
	 *
	 * @param editor Editor widget.
	 * @param model Model to update.
	 * @param index Model index being edited.
	 */
	void setModelData(QWidget* editor, QAbstractItemModel* model,
	                  const QModelIndex& index) const override {
		auto combo = qobject_cast<QComboBox*>(editor);
		if (!combo)
			return;

		QString value = combo->currentText();
		model->setData(index, value, Qt::EditRole);
	}

	/**
	 * @brief Position the editor within the cell rectangle.
	 *
	 * @param editor Editor widget.
	 * @param option Style option containing the target rectangle.
	 * @param index Model index (unused).
	 */
	void updateEditorGeometry(QWidget* editor,
	                          const QStyleOptionViewItem& option,
	                          const QModelIndex& index) const override {
		Q_UNUSED(index);
		editor->setGeometry(option.rect);
	}
};
