#ifndef DBSELECT_H
#define DBSELECT_H 1

#include <QtWidgets>
#include "goptions.h"
#include <sqlite3.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

class DBSelectView : public QWidget {
	Q_OBJECT
public:
	DBSelectView(GOptions *gopts, QWidget *parent = nullptr);
	~DBSelectView() override;

private:
	void setupUI();
	void loadExperiments();
	void loadSystemsForExperiment(QStandardItem *experimentItem, const string &experiment);
	// Returns a list of available variations for a given system.
	QStringList getAvailableVariations(const string &system);
	// Returns a list of available runs for a given system.
	QStringList getAvailableRuns(const string &system);
	// Returns the number of entries in the geometry table for the given combination.
	int getGeometryCount(const string &experiment, const string &system, const string &variation, int run);
	// Update the status icon for a system item and update the "Entries" column.
	void updateSystemItemAppearance(QStandardItem *systemItem);
	// Returns true if a system is available (i.e. count > 0) for the given combination.
	bool systemAvailable(const string &system, const string &variation, int run);
	// Update the experiment header label (displaying the checked experiment name and total systems).
	void updateExperimentHeader();

	sqlite3 *db;
	string dbhost;
	string experiment;  // default experiment from options

	QTreeView *experimentTree;          // Tree showing experiments (top-level) and systems (children)
	QStandardItemModel *experimentModel;
	QLabel *experimentHeaderLabel;      // Displays the experiment name and total systems

	// Flag to prevent recursive processing.
	bool m_ignoreItemChange = false;

	QIcon createStatusIcon(const QColor &color);
	bool isGeometryTableValid(sqlite3 *db);
	void applyGSystemSelections(GOptions *gopts);

private slots:
	// Combined slot for handling changes (experiment checkboxes and system drop‑down changes).
	void onItemChanged(QStandardItem *item);
};

//
// A custom delegate for drop‑down editing (for both variations and runs).
//
class ComboDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	ComboDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
	QWidget *createEditor(QWidget *parent,
						  const QStyleOptionViewItem &option,
						  const QModelIndex &index) const override {
		QComboBox *editor = new QComboBox(parent);
		return editor;
	}
	void setEditorData(QWidget *editor, const QModelIndex &index) const override {
		QComboBox *combo = qobject_cast<QComboBox*>(editor);
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
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					  const QModelIndex &index) const override {
		QComboBox *combo = qobject_cast<QComboBox*>(editor);
		if (!combo)
			return;
		QString value = combo->currentText();
		model->setData(index, value, Qt::EditRole);
	}
	void updateEditorGeometry(QWidget *editor,
							  const QStyleOptionViewItem &option,
							  const QModelIndex &index) const override {
		editor->setGeometry(option.rect);
	}
};

#endif
