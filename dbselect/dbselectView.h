#ifndef DBSELECT_H
#define DBSELECT_H 1

#include <QtWidgets>
#include "goptions.h"
#include <sqlite3.h>

class DBSelectView : public QWidget {
	Q_OBJECT

public:
	DBSelectView(GOptions *gopts, QWidget *parent = nullptr);
	~DBSelectView() override;

private:
	void setupUI();
	void setupDatabase();
	void loadExperiments();
	void loadSystemsForExperiment(QStandardItem *experimentItem, const std::string &experiment);
	void loadAllVariations();
	void loadAllRuns();
	// Update system appearance based on availability (status icon only).
	void updateSystemItemAppearance(QStandardItem *systemItem, bool available);
	// Update all systems in all experiments according to the current filter.
	void filter_systems(const std::string &variation, int run);
	// Returns true if the given system is available (i.e. present in DB for the given variation and run).
	bool systemAvailable(const std::string &system, const std::string &variation, int run);
	// Creates a small colored square icon.
	QIcon createStatusIcon(const QColor &color);

	// Combined slot to handle check state changes for both experiment and system items.
	void onItemChanged(QStandardItem *item);

	sqlite3 *db;
	QTreeView *experimentTree;      // The experiment tree (experiments + systems)
	QComboBox *variationComboBox;    // Variation dropdown
	QComboBox *runComboBox;          // Run dropdown
	QStandardItemModel *experimentModel;

	// A header label to display total number of systems for the selected experiment.
	QLabel *experimentHeaderLabel;

	// Flag to avoid recursive processing.
	bool m_ignoreItemChange = false;

private slots:
	void onVariationSelected(int index);
	void onRunSelected(int index);
};

#endif
