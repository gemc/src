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
 * DBSelectView is a QWidget that displays experiments and their systems.
 * It allows the user to modify selections and reload geometry.
 * Notice GBase after QWidget derivations, otherwise moc would expect it to have a staticMetaObject
 */
class DBSelectView : public QWidget,  public GBase<DBSelectView> {
	Q_OBJECT
public:
	/**
	 * Constructor.
	 * @param gopts Pointer to options.
	 * @param dc Pointer to the detector construction (used to reload geometry).
	 * @param parent Parent widget.
	 */
	explicit DBSelectView(const std::shared_ptr<GOptions>& gopts, GDetectorConstruction *dc, QWidget *parent = nullptr);

	/// Destructor.
	~DBSelectView() override;

	/// Returns the updated list of GSystem objects.
	SystemList get_gsystems();

private:
	// UI setup and update methods.
	void setupUI();
	void loadExperiments();
	void loadSystemsForExperiment(QStandardItem *experimentItem, const std::string &experiment);
	QStringList getAvailableVariations(const std::string &system);
	QStringList getAvailableRuns(const std::string &system);
	int getGeometryCount(const std::string &experiment, const std::string &system, const std::string &variation, int run);
	void updateSystemItemAppearance(QStandardItem *systemItem);
	bool systemAvailable(const std::string &system, const std::string &variation, int run);
	void updateExperimentHeader();

	/**
	 * Updates the title label and reload button based on the modified flag.
	 * If modified is true, the title label will display an asterisk and the reload button is enabled.
	 */
	void updateModifiedUI();

	// --- New member variables ---
	bool modified = false;  // Flag indicating if the tree has been modified (starts false).
	QLabel *titleLabel = nullptr;      // Label for the title ("Experiment Selection").
	QPushButton *reloadButton = nullptr;  // Button to trigger reloading geometry.

	// Database and experiment information.
	sqlite3 *db;
	std::string dbhost;
	std::string experiment;  // Default experiment name from options.

	QTreeView *experimentTree;          // Tree showing experiments and systems.
	QStandardItemModel *experimentModel;  // Model for the tree.
	QLabel *experimentHeaderLabel;      // Displays the experiment tally info.

	// Flag to prevent recursive processing of changes.
	bool m_ignoreItemChange = false;

	// Helper functions.
	QIcon createStatusIcon(const QColor &color);
	bool isGeometryTableValid(sqlite3 *db);
	void applyGSystemSelections(std::shared_ptr<GOptions> gopts);

	// Pointer to the detector construction (geometry reloading).
	GDetectorConstruction *gDetectorConstruction;

	// logger
	std::shared_ptr<GOptions> gopt; ///< Logger instance for logging messages.

private slots:
	/// Slot called when an item in the tree is changed.
	void onItemChanged(QStandardItem *item);

public slots:

	/// Slot for the Reload button. Currently empty except for resetting the modified flag.
	void reload_geometry();
};

//
// A custom delegate for drop‑down editing (for both variations and runs).
//
class ComboDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	explicit ComboDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

	/**
	 * createEditor creates a QComboBox editor.
	 * The unused parameters are marked with Q_UNUSED.
	 */
	QWidget *createEditor(QWidget *parent,
						  const QStyleOptionViewItem &option,
						  const QModelIndex &index) const override {
		Q_UNUSED(option);
		Q_UNUSED(index);
		auto editor = new QComboBox(parent);
		return editor;
	}

	/**
	 * setEditorData populates the QComboBox with data from the model.
	 */
	void setEditorData(QWidget *editor, const QModelIndex &index) const override {
		auto combo = qobject_cast<QComboBox*>(editor);
		if (!combo)
			return;
		// Retrieve list from UserRole.
		QVariant var = index.model()->data(index, Qt::UserRole);
		QStringList options = var.toStringList();
		combo->clear();
		combo->addItems(options);
		QString currentText = index.model()->data(index, Qt::EditRole).toString();
		int idx = combo->findText(currentText);
		if (idx >= 0)
			combo->setCurrentIndex(idx);
	}

	/**
	 * setModelData writes the selected value back to the model.
	 */
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					  const QModelIndex &index) const override {
		auto combo = qobject_cast<QComboBox*>(editor);
		if (!combo)
			return;
		QString value = combo->currentText();
		model->setData(index, value, Qt::EditRole);
	}

	/**
	 * updateEditorGeometry sets the geometry for the editor.
	 * Only the 'option' parameter is used; 'index' is marked as unused.
	 */
	void updateEditorGeometry(QWidget *editor,
							  const QStyleOptionViewItem &option,
							  const QModelIndex &index) const override {
		Q_UNUSED(index);
		editor->setGeometry(option.rect);
	}
};


