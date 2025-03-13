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
	void loadVariations(const std::string &system);
	void loadRuns(const std::string &system);
	void filterSystemsByVariation(const std::string &variation);
	void filterSystemsByRun(int run);

	sqlite3 *db;
	QTreeView *experimentTree;  // Hierarchical tree with experiments & systems
	QTreeView *variationTree;
	QTreeView *runTree;

	QStandardItemModel *experimentModel;
	QStandardItemModel *variationModel;
	QStandardItemModel *runModel;

private slots:
	void onExperimentSelected(const QModelIndex &index);
	void onVariationSelected(const QModelIndex &index);
	void onRunSelected(const QModelIndex &index);
};

#endif
