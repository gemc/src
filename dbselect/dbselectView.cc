#include "dbselectView.h"
#include "gsystemConventions.h"
#include "gsystemOptions.h"
#include <iostream>

using namespace std;

bool DBSelectView::isGeometryTableValid(sqlite3 *db) {

	if (!db) return false; // Ensure db is not null before querying

	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='geometry'";

	// Check if the 'geometry' table exists
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL Error: Failed to check geometry table existence: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	bool tableExists = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		tableExists = sqlite3_column_int(stmt, 0) > 0;
	}
	sqlite3_finalize(stmt);

	if (!tableExists) {
		return false;
	}

	// Now check if the table has at least one row
	sql_query = "SELECT COUNT(*) FROM geometry";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL Error: Failed to count rows in geometry table: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	bool hasData = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		hasData = sqlite3_column_int(stmt, 0) > 0;
	}
	sqlite3_finalize(stmt);

	if (!hasData) {
		return false;
	}

	return true;
}


void DBSelectView::applyGSystemSelections(GOptions *gopts) {
	std::vector<GSystem> gsystems = gsystem::getSystems(gopts);  // Get vector of GSystem objects

	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem *expItem = experimentModel->item(i, 0);
		if (!expItem) continue;

		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem *sysItem = expItem->child(j, 0);
			QStandardItem *varItem = expItem->child(j, 1);
			QStandardItem *runItem = expItem->child(j, 2);
			if (!sysItem || !varItem || !runItem) continue;

			std::string sysName = sysItem->text().toStdString();
			bool systemFound = false;

			// Iterate through the vector to find the system
			for (const GSystem &gsys : gsystems) {
				if (gsys.getName() == sysName) {
					systemFound = true;

					// Check the system checkbox
					sysItem->setCheckState(Qt::Checked);

					// Set the default variation in the combobox
					QStringList availableVariations = getAvailableVariations(sysName);
					QString selectedVar = QString::fromStdString(gsys.getVariation());
					if (availableVariations.contains(selectedVar)) {
						varItem->setData(selectedVar, Qt::EditRole);
					} else if (!availableVariations.isEmpty()) {
						varItem->setData(availableVariations.first(), Qt::EditRole);
					}
					varItem->setData(availableVariations, Qt::UserRole);

					// Set the default run in the combobox
					QStringList availableRuns = getAvailableRuns(sysName);
					QString selectedRun = QString::number(gsys.getRunno());
					if (availableRuns.contains(selectedRun)) {
						runItem->setData(selectedRun, Qt::EditRole);
					} else if (!availableRuns.isEmpty()) {
						runItem->setData(availableRuns.first(), Qt::EditRole);
					}
					runItem->setData(availableRuns, Qt::UserRole);

					// Update appearance
					updateSystemItemAppearance(sysItem);
					break;  // Stop searching once we find the system
				}
			}

			// If system was not found in gsystems, ensure it's unchecked
			if (!systemFound) {
				sysItem->setCheckState(Qt::Unchecked);
			}
		}
	}
}



DBSelectView::DBSelectView(GOptions *gopts, QWidget *parent)
		: QWidget(parent), db(nullptr), m_ignoreItemChange(false) {
	// Open the database.
	dbhost = gopts->getScalarString("sql");

	dbhost = gopts->getScalarString("sql");
	if (sqlite3_open(dbhost.c_str(), &db) != SQLITE_OK || !isGeometryTableValid(db)) {
		std::cerr << "Database Warning: Failed to open or validate database: " << dbhost << std::endl;

		// Close the failed database connection
		sqlite3_close(db);
		db = nullptr;

		// Revert to backup database: $GEMC/examples/gemc.db
		dbhost = string(getenv("GEMC")) + "/examples/gemc.db";
		if (sqlite3_open(dbhost.c_str(), &db) != SQLITE_OK || !isGeometryTableValid(db)) {
			std::cerr << "Database Error: Failed to open or validate backup database: " << dbhost << std::endl;
			sqlite3_close(db);
			db = nullptr; // Ensure db remains null if both attempts fail
		} else {
			std::cerr << "Database Warning: Using backup database: " << dbhost << std::endl;
		}
	}


	setupUI();
	loadExperiments();
	applyGSystemSelections(gopts);  // Apply system selections from gsystem map

	// Optionally, update all system items’ status initially:
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem *expItem = experimentModel->item(i, 0);
		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem *sysItem = expItem->child(j, 0);
			updateSystemItemAppearance(sysItem);
		}
	}
}

DBSelectView::~DBSelectView() {
	if (db) sqlite3_close(db);
}

void DBSelectView::setupUI() {
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(10);

	QLabel *titleLabel = new QLabel("Experiment Selection", this);
	QFont titleFont("Avenir", 20, QFont::Bold);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(titleLabel);

	// Header label to display the selected experiment and total systems.
	experimentHeaderLabel = new QLabel("", this);
	experimentHeaderLabel->setAlignment(Qt::AlignCenter);
	experimentHeaderLabel->setWordWrap(true);
	experimentHeaderLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	mainLayout->addWidget(experimentHeaderLabel);

	// Create the experiment tree.
	experimentTree = new QTreeView(this);
	experimentTree->setStyleSheet("QTreeView { alternate-background-color: #f0f0f0; }");
	experimentTree->setSelectionMode(QAbstractItemView::SingleSelection);
	experimentTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	// Unhide header so that we can use its resize modes.
	experimentTree->header()->show();

	// Create a three‑column model.
	experimentModel = new QStandardItemModel(this);
	experimentTree->setModel(experimentModel);
	// Set custom delegates for columns 1 and 2.
	experimentTree->setItemDelegateForColumn(1, new ComboDelegate(this));
	experimentTree->setItemDelegateForColumn(2, new ComboDelegate(this));

	mainLayout->addWidget(experimentTree);

	// Connect the model's itemChanged signal.
	connect(experimentModel, &QStandardItemModel::itemChanged,
			this, &DBSelectView::onItemChanged);

	// Expand all nodes so that system items are visible.
	experimentTree->expandAll();
}


void DBSelectView::loadExperiments() {
	experimentModel->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT experiment FROM geometry";
	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare experiment query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char *expText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
		if (expText) {
			QString expName = QString::fromUtf8(expText);
			QStandardItem *expItem = new QStandardItem(expName);
			// Make experiment item read‑only and checkable.
			expItem->setFlags(expItem->flags() & ~Qt::ItemIsEditable);
			expItem->setCheckable(true);
			expItem->setCheckState(Qt::Unchecked);
			// For experiments, columns 1 and 2 are empty.
			QStandardItem *dummyVar = new QStandardItem("");
			QStandardItem *dummyRun = new QStandardItem("");
			// Load systems for this experiment.
			loadSystemsForExperiment(expItem, expName.toStdString());
			// Append the experiment row (a row with three columns).
			experimentModel->appendRow(QList<QStandardItem *>() << expItem << dummyVar << dummyRun);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadSystemsForExperiment(QStandardItem *experimentItem, const string &experiment) {
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT system FROM geometry WHERE experiment = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *sysText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
			if (sysText) {
				// Column 0: system item.
				QStandardItem *sysItem = new QStandardItem(QString::fromUtf8(sysText));
				sysItem->setFlags(sysItem->flags() & ~Qt::ItemIsEditable);
				sysItem->setCheckable(true);
				sysItem->setCheckState(Qt::Unchecked);

				// Column 1: Variation drop‑down item.
				QStandardItem *varItem = new QStandardItem();
				QStringList varList = getAvailableVariations(sysText);
				if (!varList.isEmpty())
					varItem->setData(varList.first(), Qt::EditRole);
				else
					varItem->setData("", Qt::EditRole);
				// Store the available options in UserRole.
				varItem->setData(varList, Qt::UserRole);
				// Set a distinct background color for variation.
				varItem->setBackground(QColor("lightblue"));

				// Column 2: Run drop‑down item.
				QStandardItem *runItem = new QStandardItem();
				QStringList runList = getAvailableRuns(sysText);
				if (!runList.isEmpty())
					runItem->setData(runList.first(), Qt::EditRole);
				else
					runItem->setData("", Qt::EditRole);
				runItem->setData(runList, Qt::UserRole);
				// Set a distinct background color for runs.
				runItem->setBackground(QColor("lightgreen"));



				// Append a new row (three columns) to the experiment item.
				QList < QStandardItem * > rowItems;
				rowItems << sysItem << varItem << runItem;
				experimentItem->appendRow(rowItems);
			}
		}
	}
	sqlite3_finalize(stmt);
}

QStringList DBSelectView::getAvailableVariations(const string &system) {
	QStringList varList;
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT variation FROM geometry WHERE system = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *varText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
			if (varText) {
				varList << QString::fromUtf8(varText);
			}
		}
	}
	sqlite3_finalize(stmt);
	return varList;
}

QStringList DBSelectView::getAvailableRuns(const string &system) {
	QStringList runList;
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT run FROM geometry WHERE system = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int runVal = sqlite3_column_int(stmt, 0);
			runList << QString::number(runVal);
		}
	}
	sqlite3_finalize(stmt);
	return runList;
}

bool DBSelectView::systemAvailable(const string &system, const string &variation, int run) {
	string query = "SELECT COUNT(*) FROM geometry WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "systemAvailable: prepare failed: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, run);
	bool available = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int count = sqlite3_column_int(stmt, 0);
		available = (count > 0);
	}
	sqlite3_finalize(stmt);
	return available;
}

QIcon DBSelectView::createStatusIcon(const QColor &color) {
	QPixmap pixmap(12, 12);
	pixmap.fill(color);
	return QIcon(pixmap);
}

void DBSelectView::updateSystemItemAppearance(QStandardItem *systemItem) {
	// Get the corresponding variation and run items from the same row.
	QStandardItem *parentItem = systemItem->parent();
	if (!parentItem)
		return;
	int row = systemItem->row();
	QStandardItem *varItem = parentItem->child(row, 1);
	QStandardItem *runItem = parentItem->child(row, 2);
	QString varStr = varItem ? varItem->data(Qt::EditRole).toString() : "";
	QString runStr = runItem ? runItem->data(Qt::EditRole).toString() : "";
	int run = runStr.toInt();
	bool available = systemAvailable(systemItem->text().toStdString(), varStr.toStdString(), run);
	QColor statusColor = available ? QColor("green") : QColor("red");
	systemItem->setIcon(createStatusIcon(statusColor));
	// Do not change background or text colors.
	systemItem->setData(QColor("white"), Qt::BackgroundRole);
	systemItem->setData(QColor("black"), Qt::ForegroundRole);
}

void DBSelectView::updateExperimentHeader() {
	// Find the checked experiment (they are mutually exclusive).
	QStandardItem *selectedExp = nullptr;
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem *expItem = experimentModel->item(i, 0);
		if (expItem && expItem->checkState() == Qt::Checked) {
			selectedExp = expItem;
			break;
		}
	}
	if (selectedExp) {
		int totalSystems = selectedExp->rowCount();
		experimentHeaderLabel->setText(QString("Total systems for experiment \"%1\": %2")
											   .arg(selectedExp->text()).arg(totalSystems));
	} else {
		experimentHeaderLabel->setText("");
	}
}

void DBSelectView::onItemChanged(QStandardItem *item) {
	if (m_ignoreItemChange)
		return;
	if (!item)
		return;
	m_ignoreItemChange = true;
	// If this is a top-level experiment item:
	if (!item->parent()) {
		if (item->checkState() == Qt::Checked) {
			// Uncheck all other experiments.
			for (int i = 0; i < experimentModel->rowCount(); ++i) {
				QStandardItem *expItem = experimentModel->item(i, 0);
				if (expItem != item)
					expItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		} else {
			// Experiment unchecked: uncheck all its child systems.
			for (int i = 0; i < item->rowCount(); ++i) {
				QStandardItem *sysItem = item->child(i, 0);
				if (sysItem)
					sysItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		}
	} else {
		// This is a child item (a system row).
		// If the changed item is in column 0 (system checkbox), update its appearance.
		if (item->column() == 0) {
			updateSystemItemAppearance(item);
		}
			// If the changed item is in column 1 or 2 (variation or run drop‑down), update the corresponding system item.
		else if (item->column() == 1 || item->column() == 2) {
			QStandardItem *sysItem = item->parent()->child(item->row(), 0);
			updateSystemItemAppearance(sysItem);
		}
	}
	m_ignoreItemChange = false;

	// resize columns to contents if necessary
	experimentTree->resizeColumnToContents(0);
	experimentTree->setColumnWidth(1, 200);
	experimentTree->setColumnWidth(2, 100);
	experimentTree->header()->setStretchLastSection(false);
	experimentModel->setHorizontalHeaderLabels(QStringList() << "Experiment" << "Variation" << "Run");

}

