#include "dbselectView.h"
#include <cstdlib>

DBSelectView::DBSelectView(GOptions *gopts, QWidget *parent)
		: QWidget(parent), db(nullptr)
{
	std::string database;
	if (gopts->doesOptionExist("gsystem")) {
		//database = gopts->getOption("gsystem");  // assuming this method exists
	} else {
		database = std::string(getenv("GEMC")) + "/examples/gemc.db";
	}

	if (sqlite3_open(database.c_str(), &db) != SQLITE_OK) {
		QMessageBox::critical(this, "Database Error", "Failed to open database");
	}
	setupUI();
	loadExperiments();
}

DBSelectView::~DBSelectView() {
	if (db) {
		sqlite3_close(db);
	}
}

#include "dbselectView.h"
#include <QFont>
#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QPixmap>

void DBSelectView::setupUI() {
	// Main layout for the entire widget.
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(10);

	// Title label for the entire widget.
	QLabel *titleLabel = new QLabel("Experiment Selection", this);
	QFont titleFont("Arial", 18, QFont::Bold);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignCenter);
	// Optional: set an icon next to the title using QPixmap if desired.
	// QPixmap titleIcon("https://example.com/icon.png");
	// titleLabel->setPixmap(titleIcon);
	mainLayout->addWidget(titleLabel);

	// Create a container widget and layout for the tree views.
	QWidget *treeContainer = new QWidget(this);
	QHBoxLayout *treeLayout = new QHBoxLayout(treeContainer);
	treeLayout->setSpacing(20);

	// ----- Experiment Tree Container -----
	QWidget *experimentContainer = new QWidget(treeContainer);
	QVBoxLayout *expLayout = new QVBoxLayout(experimentContainer);
	expLayout->setSpacing(5);

	QLabel *expTitleLabel = new QLabel("Experiments", experimentContainer);
	QFont expTitleFont("Arial", 14, QFont::Bold);
	expTitleLabel->setFont(expTitleFont);
	expTitleLabel->setAlignment(Qt::AlignCenter);
	expLayout->addWidget(expTitleLabel);

	experimentTree = new QTreeView(experimentContainer);
	// Increase the font size of the experiment tree for emphasis.
	QFont expTreeFont("Arial", 12);
	experimentTree->setFont(expTreeFont);
	// Optionally adjust row height for better readability.
	experimentTree->setStyleSheet("QTreeView { alternate-background-color: #f0f0f0; }");
	expLayout->addWidget(experimentTree);

	// Set the model to the experiment tree later.
	// Create and set the experiment model
	experimentModel = new QStandardItemModel(this);
	experimentTree->setModel(experimentModel);

	treeLayout->addWidget(experimentContainer, /*stretch=*/2);

	// ----- Variation Tree Container -----
	QWidget *variationContainer = new QWidget(treeContainer);
	QVBoxLayout *varLayout = new QVBoxLayout(variationContainer);
	varLayout->setSpacing(5);

	QLabel *varTitleLabel = new QLabel("Variations", variationContainer);
	QFont varTitleFont("Arial", 12, QFont::Bold);
	varTitleLabel->setFont(varTitleFont);
	varTitleLabel->setAlignment(Qt::AlignCenter);
	varLayout->addWidget(varTitleLabel);

	variationModel = new QStandardItemModel(this);
	variationTree = new QTreeView(variationContainer);
	variationTree->setFont(QFont("Arial", 10));
	variationTree->setStyleSheet("QTreeView { alternate-background-color: #f9f9f9; }");
	varLayout->addWidget(variationTree);

	runModel = new QStandardItemModel(this);
	variationTree->setModel(variationModel);
	treeLayout->addWidget(variationContainer, /*stretch=*/1);

	// ----- Run Tree Container -----
	QWidget *runContainer = new QWidget(treeContainer);
	QVBoxLayout *runLayout = new QVBoxLayout(runContainer);
	runLayout->setSpacing(5);

	QLabel *runTitleLabel = new QLabel("Runs", runContainer);
	QFont runTitleFont("Arial", 12, QFont::Bold);
	runTitleLabel->setFont(runTitleFont);
	runTitleLabel->setAlignment(Qt::AlignCenter);
	runLayout->addWidget(runTitleLabel);

	runTree = new QTreeView(runContainer);
	runTree->setFont(QFont("Arial", 10));
	runTree->setStyleSheet("QTreeView { alternate-background-color: #f9f9f9; }");
	runLayout->addWidget(runTree);

	runTree->setModel(runModel);
	treeLayout->addWidget(runContainer, /*stretch=*/1);

	mainLayout->addWidget(treeContainer);

	// Set selection mode and behavior for experiment tree (and others if needed).
	experimentTree->setSelectionMode(QAbstractItemView::SingleSelection);
	experimentTree->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Connect signals to slots.
	connect(experimentTree->selectionModel(), &QItemSelectionModel::currentChanged,
			this, &DBSelectView::onExperimentSelected);
	connect(variationTree, &QTreeView::clicked,
			this, &DBSelectView::onVariationSelected);
	connect(runTree, &QTreeView::clicked,
			this, &DBSelectView::onRunSelected);
}



void DBSelectView::loadExperiments() {
	if (!experimentModel) {
		qCritical() << "experimentModel is not initialized!";
		return;
	}
	experimentModel->clear();

	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT experiment FROM geometry";

	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		qCritical() << "Failed to prepare statement:" << sqlite3_errmsg(db);
		return;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char *expText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if(expText) {
			QString expName = QString::fromUtf8(expText);
			// Create the experiment item.
			QStandardItem *expItem = new QStandardItem(expName);
			// Now load systems as children of the experiment.
			loadSystemsForExperiment(expItem, expName.toStdString());
			experimentModel->appendRow(expItem);
		} else {
			qWarning() << "Null experiment text encountered.";
		}
	}
	sqlite3_finalize(stmt);
}



void DBSelectView::loadSystemsForExperiment(QStandardItem *experimentItem, const std::string &experiment) {
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT system FROM geometry WHERE experiment = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *sysText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			QStandardItem *sysItem = new QStandardItem(QString::fromUtf8(sysText));
			experimentItem->appendRow(sysItem);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadVariations(const std::string &system) {
	variationModel->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT variation FROM geometry WHERE system = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *varText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			QStandardItem *item = new QStandardItem(QString::fromUtf8(varText));
			variationModel->appendRow(item);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadRuns(const std::string &system) {
	runModel->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT run FROM geometry WHERE system = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int runVal = sqlite3_column_int(stmt, 0);
			QStandardItem *item = new QStandardItem(QString::number(runVal));
			runModel->appendRow(item);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::filterSystemsByVariation(const std::string &variation) {
	// Rebuild the experiment tree filtering systems that match the selected variation.
	experimentModel->clear();
	sqlite3_stmt *stmtExp = nullptr;
	const char *sql_queryExp = "SELECT DISTINCT experiment FROM geometry";
	if (sqlite3_prepare_v2(db, sql_queryExp, -1, &stmtExp, nullptr) == SQLITE_OK) {
		while (sqlite3_step(stmtExp) == SQLITE_ROW) {
			const char *expText = reinterpret_cast<const char*>(sqlite3_column_text(stmtExp, 0));
			QString expName = QString::fromUtf8(expText);
			QStandardItem *expItem = new QStandardItem(expName);

			// Load only systems that match the variation.
			sqlite3_stmt *stmtSys = nullptr;
			const char *sql_querySys = "SELECT DISTINCT system FROM geometry WHERE experiment = ? AND variation = ?";
			if (sqlite3_prepare_v2(db, sql_querySys, -1, &stmtSys, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(stmtSys, 1, expName.toStdString().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(stmtSys, 2, variation.c_str(), -1, SQLITE_TRANSIENT);
				while (sqlite3_step(stmtSys) == SQLITE_ROW) {
					const char *sysText = reinterpret_cast<const char*>(sqlite3_column_text(stmtSys, 0));
					QStandardItem *sysItem = new QStandardItem(QString::fromUtf8(sysText));
					expItem->appendRow(sysItem);
				}
			}
			sqlite3_finalize(stmtSys);

			// Only add experiment node if there is at least one matching system.
			if (expItem->rowCount() > 0)
				experimentModel->appendRow(expItem);
		}
	}
	sqlite3_finalize(stmtExp);
}

void DBSelectView::filterSystemsByRun(int run) {
	// Rebuild the experiment tree filtering systems that match the selected run.
	experimentModel->clear();
	sqlite3_stmt *stmtExp = nullptr;
	const char *sql_queryExp = "SELECT DISTINCT experiment FROM geometry";
	if (sqlite3_prepare_v2(db, sql_queryExp, -1, &stmtExp, nullptr) == SQLITE_OK) {
		while (sqlite3_step(stmtExp) == SQLITE_ROW) {
			const char *expText = reinterpret_cast<const char*>(sqlite3_column_text(stmtExp, 0));
			QString expName = QString::fromUtf8(expText);
			QStandardItem *expItem = new QStandardItem(expName);

			// Load only systems that match the run.
			sqlite3_stmt *stmtSys = nullptr;
			const char *sql_querySys = "SELECT DISTINCT system FROM geometry WHERE experiment = ? AND run = ?";
			if (sqlite3_prepare_v2(db, sql_querySys, -1, &stmtSys, nullptr) == SQLITE_OK) {
				sqlite3_bind_text(stmtSys, 1, expName.toStdString().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(stmtSys, 2, run);
				while (sqlite3_step(stmtSys) == SQLITE_ROW) {
					const char *sysText = reinterpret_cast<const char*>(sqlite3_column_text(stmtSys, 0));
					QStandardItem *sysItem = new QStandardItem(QString::fromUtf8(sysText));
					expItem->appendRow(sysItem);
				}
			}
			sqlite3_finalize(stmtSys);

			if (expItem->rowCount() > 0)
				experimentModel->appendRow(expItem);
		}
	}
	sqlite3_finalize(stmtExp);
}

void DBSelectView::onExperimentSelected(const QModelIndex &index) {
	if (!index.isValid())
		return;
	// Determine whether the selected item is an experiment (root) or a system (child).
	QStandardItem *item = experimentModel->itemFromIndex(index);
	if (item->parent() != nullptr) {
		// A system was selected—load variations and runs for that system.
		std::string system = item->text().toStdString();
		loadVariations(system);
		loadRuns(system);
	} else {
		// An experiment was selected, so clear variations and runs.
		variationModel->clear();
		runModel->clear();
	}
}

void DBSelectView::onVariationSelected(const QModelIndex &index) {
	if (!index.isValid())
		return;
	std::string variation = variationModel->itemFromIndex(index)->text().toStdString();
	filterSystemsByVariation(variation);
}

void DBSelectView::onRunSelected(const QModelIndex &index) {
	if (!index.isValid())
		return;
	int run = runModel->itemFromIndex(index)->text().toInt();
	filterSystemsByRun(run);
}
