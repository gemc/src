#include "dbselectView.h"
#include "gsystemConventions.h"
#include "gsystemOptions.h"
#include "gsystemConventions.h"
#include <iostream>

using namespace std;

// Checks that the database has a non-empty "geometry" table.
bool DBSelectView::isGeometryTableValid(sqlite3* db) {
	if (!db) return false;
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='geometry'";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL Error: Failed to check geometry table existence: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool tableExists = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) { tableExists = sqlite3_column_int(stmt, 0) > 0; }
	sqlite3_finalize(stmt);
	if (!tableExists) return false;
	sql_query = "SELECT COUNT(*) FROM geometry";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL Error: Failed to count rows in geometry table: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	bool hasData = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) { hasData = sqlite3_column_int(stmt, 0) > 0; }
	sqlite3_finalize(stmt);
	return hasData;
}

// Applies selections from the GSystem vector to update the UI.
void DBSelectView::applyGSystemSelections(GOptions* gopts) {
	vector<GSystem> gsystems = gsystem::getSystems(gopts); // Get vector of GSystem objects.
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		if (!expItem) continue;
		// Mark the default experiment as checked if it matches.
		if (expItem->text() == QString::fromStdString(experiment)) { expItem->setCheckState(Qt::Checked); }
		// Process each child system.
		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem* sysItem = expItem->child(j, 0);
			QStandardItem* varItem = expItem->child(j, 2);
			QStandardItem* runItem = expItem->child(j, 3);
			if (!sysItem || !varItem || !runItem) continue;
			string sysName     = sysItem->text().toStdString();
			bool   systemFound = false;
			for (const GSystem& gsys : gsystems) {
				if (gsys.getName() == sysName) {
					systemFound = true;
					sysItem->setCheckState(Qt::Checked);
					QStringList availableVariations = getAvailableVariations(sysName);
					QString     selectedVar         = QString::fromStdString(gsys.getVariation());
					if (availableVariations.contains(selectedVar))
						varItem->setData(selectedVar, Qt::EditRole);
					else if (!availableVariations.isEmpty())
						varItem->setData(availableVariations.first(), Qt::EditRole);
					varItem->setData(availableVariations, Qt::UserRole);
					QStringList availableRuns = getAvailableRuns(sysName);
					QString     selectedRun   = QString::number(gsys.getRunno());
					if (availableRuns.contains(selectedRun))
						runItem->setData(selectedRun, Qt::EditRole);
					else if (!availableRuns.isEmpty())
						runItem->setData(availableRuns.first(), Qt::EditRole);
					runItem->setData(availableRuns, Qt::UserRole);
					updateSystemItemAppearance(sysItem);
					break;
				}
			}
			if (!systemFound) { sysItem->setCheckState(Qt::Unchecked); }
		}
	}
}

// Constructor. Initializes the widget, opens the database and sets up the UI.
DBSelectView::DBSelectView(GOptions* gopts, GDetectorConstruction* dc, QWidget* parent)
	: QWidget(parent), db(nullptr), m_ignoreItemChange(false), gDetectorConstruction(dc) {
	// Open the database.
	dbhost     = gopts->getScalarString("sql");
	experiment = gopts->getScalarString("experiment");

	if (sqlite3_open(dbhost.c_str(), &db) != SQLITE_OK || !isGeometryTableValid(db)) {
		std::cerr << "Database Warning: Failed to open or validate database: " << dbhost << std::endl;
		sqlite3_close(db);
		db = nullptr;

		std::filesystem::path gemcRoot = gutilities::gemc_root();
		dbhost                         = gemcRoot.string() + "/examples/gemc.db";

		if (sqlite3_open(dbhost.c_str(), &db) != SQLITE_OK || !isGeometryTableValid(db)) {
			std::cerr << "Database Error: Failed to open or validate backup database: " << dbhost << std::endl;
			sqlite3_close(db);
			db = nullptr;
			std::exit(1);
		}
		else { std::cerr << "Database Warning: Using backup database: " << dbhost << std::endl; }
	}

	setupUI();

	// Block signals during initial population so that onItemChanged is not triggered.
	experimentModel->blockSignals(true);
	loadExperiments();

	// Verify that the default experiment exists.
	bool expFound = false;
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		if (expItem && expItem->text() == QString::fromStdString(experiment)) {
			expItem->setCheckState(Qt::Checked);
			expFound = true;
			break;
		}
	}
	if (!expFound) {
		std::cerr << "Error: Experiment \"" << experiment << "\" not found in database. Exiting." << std::endl;
		std::exit(1);
	}

	// Apply selections from GSystem objects.
	applyGSystemSelections(gopts);

	// Update system appearances initially.
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem* sysItem = expItem->child(j, 0);
			updateSystemItemAppearance(sysItem);
		}
	}

	// Unblock signals now that initial population is complete.
	experimentModel->blockSignals(false);

	// Ensure modified starts as false.
	modified = false;
	updateModifiedUI();
}


// Destructor: Closes the database if it is open.
DBSelectView::~DBSelectView() { if (db) sqlite3_close(db); }

/**
 * setupUI() sets up the widget's layout:
 * - The header area consists of a left vertical layout containing the title label
 *   ("Experiment Selection") and the experiment tally label (experimentHeaderLabel),
 *   and a Reload button aligned to the right.
 * - The experiment tree is added below the header.
 */
void DBSelectView::setupUI() {
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(10);

	// Header layout: left side holds labels, right side holds the Reload button.
	QHBoxLayout* headerLayout = new QHBoxLayout();

	// Vertical layout for the two labels.
	QVBoxLayout* labelLayout = new QVBoxLayout();
	// Title label: displays "Experiment Selection" (will be updated if modified).
	titleLabel = new QLabel("Experiment Selection", this);
	QFont titleFont("Avenir", 20, QFont::Bold);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	labelLayout->addWidget(titleLabel);

	// Tally label: displays the total systems for the selected experiment.
	experimentHeaderLabel = new QLabel("", this);
	// Align to left for consistency with the title.
	experimentHeaderLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	experimentHeaderLabel->setWordWrap(true);
	experimentHeaderLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	labelLayout->addWidget(experimentHeaderLabel);

	headerLayout->addLayout(labelLayout);

	// Add a stretch to push the reload button to the far right.
	headerLayout->addStretch();

	// Reload button: when clicked, calls reload_geometry().
	reloadButton = new QPushButton("Reload", this);
	reloadButton->setEnabled(false); // initially disabled (modified is false)
	headerLayout->addWidget(reloadButton);
	connect(reloadButton, &QPushButton::clicked, this, &DBSelectView::reload_geometry);

	mainLayout->addLayout(headerLayout);

	// Create and set up the experiment tree.
	experimentTree = new QTreeView(this);
	experimentTree->setStyleSheet("QTreeView { alternate-background-color: #f0f0f0; }");
	experimentTree->setSelectionMode(QAbstractItemView::SingleSelection);
	experimentTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	experimentTree->header()->show();

	// Create a four-column model.
	experimentModel = new QStandardItemModel(this);
	// Set header labels: "exp/system", "volumes", "variation", "run"
	experimentModel->setHorizontalHeaderLabels(QStringList() << "exp/system" << "volumes" << "variation" << "run");
	experimentTree->setModel(experimentModel);
	// Set delegates for the variation and run columns.
	experimentTree->setItemDelegateForColumn(2, new ComboDelegate(this));
	experimentTree->setItemDelegateForColumn(3, new ComboDelegate(this));
	mainLayout->addWidget(experimentTree);

	connect(experimentModel, &QStandardItemModel::itemChanged,
	        this, &DBSelectView::onItemChanged);
}

void DBSelectView::loadExperiments() {
	experimentModel->clear();
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT experiment FROM geometry";
	int           rc        = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare experiment query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char* expText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if (expText) {
			QString        expName = QString::fromUtf8(expText);
			QStandardItem* expItem = new QStandardItem(expName);
			expItem->setFlags(expItem->flags() & ~Qt::ItemIsEditable);
			expItem->setCheckable(true);
			expItem->setCheckState(Qt::Unchecked);
			QStandardItem* dummyEntries = new QStandardItem("");
			QStandardItem* dummyVar     = new QStandardItem("");
			QStandardItem* dummyRun     = new QStandardItem("");
			loadSystemsForExperiment(expItem, expName.toStdString());
			experimentModel->appendRow(QList<QStandardItem*>() << expItem << dummyEntries << dummyVar << dummyRun);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadSystemsForExperiment(QStandardItem* experimentItem, const string& experiment) {
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT system FROM geometry WHERE experiment = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char* sysText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			if (sysText) {
				QStandardItem* sysItem = new QStandardItem(QString::fromUtf8(sysText));
				sysItem->setFlags(sysItem->flags() & ~Qt::ItemIsEditable);
				sysItem->setCheckable(true);
				sysItem->setCheckState(Qt::Unchecked);
				QStandardItem* entriesItem = new QStandardItem("");
				QStandardItem* varItem     = new QStandardItem();
				QStringList    varList     = getAvailableVariations(sysText);
				if (!varList.isEmpty())
					varItem->setData(varList.first(), Qt::EditRole);
				else
					varItem->setData("", Qt::EditRole);
				varItem->setData(varList, Qt::UserRole);
				varItem->setBackground(QColor("lightblue"));
				QStandardItem* runItem = new QStandardItem();
				QStringList    runList = getAvailableRuns(sysText);
				if (!runList.isEmpty())
					runItem->setData(runList.first(), Qt::EditRole);
				else
					runItem->setData("", Qt::EditRole);
				runItem->setData(runList, Qt::UserRole);
				runItem->setBackground(QColor("lightgreen"));

				QList<QStandardItem*> rowItems;
				rowItems << sysItem << entriesItem << varItem << runItem;
				experimentItem->appendRow(rowItems);
			}
		}
	}
	sqlite3_finalize(stmt);
}

int DBSelectView::getGeometryCount(const string& experiment, const string& system, const string& variation, int run) {
	int           count = 0;
	string        query = "SELECT COUNT(*) FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt  = nullptr;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, system.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 4, run);
		if (sqlite3_step(stmt) == SQLITE_ROW) { count = sqlite3_column_int(stmt, 0); }
	}
	else { std::cerr << "getGeometryCount: prepare failed: " << sqlite3_errmsg(db) << std::endl; }
	sqlite3_finalize(stmt);
	return count;
}

QStringList DBSelectView::getAvailableVariations(const string& system) {
	QStringList   varList;
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT variation FROM geometry WHERE system = ?";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char* varText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			if (varText) { varList << QString::fromUtf8(varText); }
		}
	}
	sqlite3_finalize(stmt);
	return varList;
}

QStringList DBSelectView::getAvailableRuns(const string& system) {
	QStringList   runList;
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT run FROM geometry WHERE system = ?";
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

bool DBSelectView::systemAvailable(const string& system, const string& variation, int run) {
	string        query = "SELECT COUNT(*) FROM geometry WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt  = nullptr;
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

QIcon DBSelectView::createStatusIcon(const QColor& color) {
	QPixmap pixmap(12, 12);
	pixmap.fill(color);
	return QIcon(pixmap);
}

void DBSelectView::updateSystemItemAppearance(QStandardItem* systemItem) {
	QStandardItem* parentItem = systemItem->parent();
	if (!parentItem)
		return;
	int            row            = systemItem->row();
	QStandardItem* varItem        = parentItem->child(row, 2); // Variation column.
	QStandardItem* runItem        = parentItem->child(row, 3); // Run column.
	QString        varStr         = varItem ? varItem->data(Qt::EditRole).toString() : "";
	QString        runStr         = runItem ? runItem->data(Qt::EditRole).toString() : "";
	int            run            = runStr.toInt();
	QString        expStr         = parentItem->text();
	string         experimentName = expStr.toStdString();
	string         systemName     = systemItem->text().toStdString();
	string         variation      = varStr.toStdString();
	int            count          = getGeometryCount(experimentName, systemName, variation, run);
	QStandardItem* entriesItem    = parentItem->child(row, 1);
	if (entriesItem) { entriesItem->setText(QString::number(count)); }
	bool   available   = (count > 0);
	QColor statusColor = available ? QColor("green") : QColor("red");
	systemItem->setIcon(createStatusIcon(statusColor));
	systemItem->setData(QColor("white"), Qt::BackgroundRole);
	systemItem->setData(QColor("black"), Qt::ForegroundRole);
}

void DBSelectView::updateExperimentHeader() {
	QStandardItem* selectedExp = nullptr;
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		if (expItem && expItem->checkState() == Qt::Checked) {
			selectedExp = expItem;
			break;
		}
	}
	if (selectedExp) {
		int totalSystems = selectedExp->rowCount();
		experimentHeaderLabel->setText(QString("Total systems for experiment \"%1\": %2")
		                               .arg(selectedExp->text()).arg(totalSystems));
	}
	else { experimentHeaderLabel->setText(""); }

	experimentModel->setHorizontalHeaderLabels(QStringList() << "exp/system" << "volumes" << "variation" << "run");
}

void DBSelectView::onItemChanged(QStandardItem* item) {
	if (m_ignoreItemChange || !item)
		return;
	m_ignoreItemChange = true;
	if (!item->parent()) {
		if (item->checkState() == Qt::Checked) {
			for (int i = 0; i < experimentModel->rowCount(); ++i) {
				QStandardItem* expItem = experimentModel->item(i, 0);
				if (expItem != item)
					expItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		}
		else {
			for (int i = 0; i < item->rowCount(); ++i) {
				QStandardItem* sysItem = item->child(i, 0);
				if (sysItem)
					sysItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		}
	}
	else {
		if (item->column() == 0) { updateSystemItemAppearance(item); }
		else if (item->column() == 2 || item->column() == 3) {
			QStandardItem* sysItem = item->parent()->child(item->row(), 0);
			updateSystemItemAppearance(sysItem);
		}
	}
	m_ignoreItemChange = false;

	// Mark the view as modified (if not already) and update the UI.
	if (!modified) { modified = true; }
	updateModifiedUI();
}

SystemList DBSelectView::get_gsystems() {
	SystemList> updatedSystems;

	for (int i = 0; i < experimentModel->rowCount(); i++) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		if (!expItem)
			continue;
		for (int j = 0; j < expItem->rowCount(); j++) {
			QStandardItem* sysItem = expItem->child(j, 0);
			QStandardItem* varItem = expItem->child(j, 2);
			QStandardItem* runItem = expItem->child(j, 3);
			if (!sysItem || !varItem || !runItem)
				continue;

			if (sysItem->checkState() == Qt::Checked) {
				string  systemName = sysItem->text().toStdString();
				string  variation  = varItem->data(Qt::EditRole).toString().toStdString();
				int     run        = runItem->data(Qt::EditRole).toInt();
				GSystem updatedSystem(systemName, GSYSTEMSQLITETFACTORYLABEL, variation, /*verbosity*/ 0, run);
				// updatedSystem.set_dbhost(dbhost); only set at command line?
				updatedSystems.emplace_back(updatedSystem);
			}
		}
	}
	return updatedSystems;
}

/**
 * updateModifiedUI() updates the title label text and the enabled state of the Reload button
 * based on whether the view has been modified.
 */
void DBSelectView::updateModifiedUI() {
	updateExperimentHeader();

	if (modified)
		titleLabel->setText("Experiment Selection* (modified)");
	else
		titleLabel->setText("Experiment Selection");
	reloadButton->setEnabled(modified);

	experimentTree->resizeColumnToContents(0);
	experimentTree->setColumnWidth(1, 100);
	experimentTree->setColumnWidth(2, 150);
	experimentTree->setColumnWidth(3, 150);
	experimentTree->header()->setStretchLastSection(false);
	experimentTree->expandAll();
}

/**
 * reload_geometry() is the slot for the Reload button.
 * Currently it resets the modified flag and updates the UI.
 * (Geometry reloading logic can be added here.)
 */
void DBSelectView::reload_geometry() {
	// Reload the geometry using the updated GSystem objects.
	gDetectorConstruction->reload_geometry(get_gsystems());

	// Reset the modified flag.
	modified = false;
	updateModifiedUI();
}
