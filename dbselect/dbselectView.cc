// dbselect
#include "dbselectView.h"
#include "dbselect_options.h"

// gemc
#include "gsystemConventions.h"

// qt
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QStringList>


// Implementation notes:
// - Doxygen documentation is authoritative in dbselectView.h.
// - This file uses short non-Doxygen comments to explain local implementation decisions.

DBSelectView::DBSelectView(const std::shared_ptr<GOptions>& gopts, GDetectorConstruction* dc, QWidget* parent)
	: QWidget(parent),
	  GBase(gopts, DBSELECT_LOGGER),
	  db(nullptr),
	  gDetectorConstruction(dc),
	  gopt(gopts) {

	// Read database path/key and default experiment from options.
	dbhost     = gopts->getScalarString("sql");
	experiment = gopts->getScalarString("experiment");

	// Search order for locating the database file:
	// 1) current directory
	// 2) GEMC installation root
	// 3) GEMC examples directory
	std::vector<std::string> dirs = {
		".",
		gutilities::gemc_root().string(),
		(gutilities::gemc_root() / "examples").string()
	};

	auto dbPath = gutilities::searchForFileInLocations(dirs, dbhost);
	if (!dbPath) {
		log->error(ERR_GSQLITEERROR, "Failed to find database file. Exiting.");
	}

	// Open read-only and ensure the expected table exists and is non-empty.
	if (sqlite3_open_v2(dbPath.value().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK || !isGeometryTableValid()) {
		sqlite3_close(db);
		db = nullptr;
		log->error(ERR_GSQLITEERROR, " Failed to open or validate database", dbhost);
	}

	log->info(1, "Opened database: " + dbhost, " found at ", dbPath.value());

	// Create UI widgets and model.
	setupUI();

	// During initial population we block itemChanged notifications to prevent
	// the model initialization from marking the view as user-modified.
	experimentModel->blockSignals(true);
	loadExperiments();

	// Verify that the default experiment exists and pre-check it.
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
		log->error(ERR_EXPERIMENTNOTFOUND, experiment, " not found in database.", dbhost);
	}

	// Apply selections from configured GSystem objects (if any).
	applyGSystemSelections();

	// Update system appearances initially so “volumes” and availability icons are correct.
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem* sysItem = expItem->child(j, 0);
			updateSystemItemAppearance(sysItem);
		}
	}

	// Initialization complete: restore signals.
	experimentModel->blockSignals(false);

	// Ensure the view starts unmodified.
	modified = false;
	updateModifiedUI();
}

bool DBSelectView::isGeometryTableValid() const {
	if (!db)
		return false;

	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='geometry'";

	// First check: table existence.
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "SQL Error: Failed to check geometry table existence:", sqlite3_errmsg(db));
	}

	bool tableExists = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		tableExists = sqlite3_column_int(stmt, 0) > 0;
	}
	sqlite3_finalize(stmt);

	if (!tableExists)
		return false;

	// Second check: table contains data.
	sql_query = "SELECT COUNT(*) FROM geometry";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "SQL Error: Failed to count rows in geometry table:", sqlite3_errmsg(db));
	}

	bool hasData = false;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		hasData = sqlite3_column_int(stmt, 0) > 0;
	}
	sqlite3_finalize(stmt);

	return hasData;
}

void DBSelectView::applyGSystemSelections() {
	// Pull the current system selection from configuration and mirror it into the UI model.
	auto gsystems = gsystem::getSystems(gopt);

	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem* expItem = experimentModel->item(i, 0);
		if (!expItem)
			continue;

		// Mark the default experiment as checked if it matches.
		if (expItem->text() == QString::fromStdString(experiment)) {
			expItem->setCheckState(Qt::Checked);
		}

		// Process each child system row under this experiment.
		for (int j = 0; j < expItem->rowCount(); ++j) {
			QStandardItem* sysItem = expItem->child(j, 0);
			QStandardItem* varItem = expItem->child(j, 2);
			QStandardItem* runItem = expItem->child(j, 3);
			if (!sysItem || !varItem || !runItem)
				continue;

			std::string sysName     = sysItem->text().toStdString();
			bool        systemFound = false;

			for (auto const& gsys : gsystems) {
				if (gsys->getName() == sysName) {
					systemFound = true;
					sysItem->setCheckState(Qt::Checked);

					// Variations: select configured value if present, otherwise default to first.
					QStringList availableVariations = getAvailableVariations(sysName);
					QString     selectedVar         = QString::fromStdString(gsys->getVariation());
					if (availableVariations.contains(selectedVar))
						varItem->setData(selectedVar, Qt::EditRole);
					else if (!availableVariations.isEmpty())
						varItem->setData(availableVariations.first(), Qt::EditRole);
					varItem->setData(availableVariations, Qt::UserRole);

					// Runs: select configured value if present, otherwise default to first.
					QStringList availableRuns = getAvailableRuns(sysName);
					QString     selectedRun   = QString::number(gsys->getRunno());
					if (availableRuns.contains(selectedRun))
						runItem->setData(selectedRun, Qt::EditRole);
					else if (!availableRuns.isEmpty())
						runItem->setData(availableRuns.first(), Qt::EditRole);
					runItem->setData(availableRuns, Qt::UserRole);

					updateSystemItemAppearance(sysItem);
					break;
				}
			}

			// If no configured system matches, keep it unchecked.
			if (!systemFound) {
				sysItem->setCheckState(Qt::Unchecked);
			}
		}
	}
}

void DBSelectView::setupUI() {
	auto mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(10);

	// Header: title + experiment summary on the left, reload button on the right.
	auto headerLayout = new QHBoxLayout();

	auto labelLayout = new QVBoxLayout();

	titleLabel = new QLabel("Experiment Selection", this);
	QFont titleFont("Avenir", 20, QFont::Bold);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	labelLayout->addWidget(titleLabel);

	experimentHeaderLabel = new QLabel("", this);
	experimentHeaderLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	experimentHeaderLabel->setWordWrap(true);
	experimentHeaderLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	labelLayout->addWidget(experimentHeaderLabel);

	headerLayout->addLayout(labelLayout);
	headerLayout->addStretch();

	reloadButton = new QPushButton("Reload", this);
	reloadButton->setEnabled(false);
	headerLayout->addWidget(reloadButton);
	connect(reloadButton, &QPushButton::pressed, this, &DBSelectView::reload_geometry);

	mainLayout->addLayout(headerLayout);

	// Tree view and model.
	experimentTree = new QTreeView(this);
	experimentTree->setStyleSheet("QTreeView { alternate-background-color: #f0f0f0; }");
	experimentTree->setSelectionMode(QAbstractItemView::SingleSelection);
	experimentTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	experimentTree->header()->show();

	experimentModel = new QStandardItemModel(this);
	experimentModel->setHorizontalHeaderLabels(QStringList() << "exp/system" << "volumes" << "variation" << "run");

	experimentTree->setModel(experimentModel);

	// Variation/run columns are edited via drop-downs.
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

	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Failed to prepare experiment query:", sqlite3_errmsg(db));
	}

	// Populate one top-level item per experiment.
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char* expText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if (expText) {
			QString expName = QString::fromUtf8(expText);

			// Note: this assigns the member used by loadSystemsForExperiment().
			experiment = expName.toStdString();

			auto* expItem = new QStandardItem(expName);
			expItem->setFlags(expItem->flags() & ~Qt::ItemIsEditable);
			expItem->setCheckable(true);
			expItem->setCheckState(Qt::Unchecked);

			// Dummy columns for the experiment row; only column 0 is meaningful.
			auto* dummyEntries = new QStandardItem("");
			auto* dummyVar     = new QStandardItem("");
			auto* dummyRun     = new QStandardItem("");

			loadSystemsForExperiment(expItem);

			experimentModel->appendRow(QList<QStandardItem*>() << expItem << dummyEntries << dummyVar << dummyRun);
		}
	}

	sqlite3_finalize(stmt);
}

void DBSelectView::loadSystemsForExperiment(QStandardItem* experimentItem) {
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT system FROM geometry WHERE experiment = ?";

	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		// Bind current experiment selection (member variable).
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char* sysText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			if (sysText) {
				auto* sysItem = new QStandardItem(QString::fromUtf8(sysText));
				sysItem->setFlags(sysItem->flags() & ~Qt::ItemIsEditable);
				sysItem->setCheckable(true);
				sysItem->setCheckState(Qt::Unchecked);

				// Column 1: count of matching geometry entries (set later).
				auto* entriesItem = new QStandardItem("");

				// Column 2: variation (editable, backed by UserRole list).
				auto*       varItem = new QStandardItem();
				QStringList varList = getAvailableVariations(sysText);
				if (!varList.isEmpty())
					varItem->setData(varList.first(), Qt::EditRole);
				else
					varItem->setData("", Qt::EditRole);
				varItem->setData(varList, Qt::UserRole);
				varItem->setBackground(QColor("lightblue"));

				// Column 3: run (editable, backed by UserRole list).
				auto*       runItem = new QStandardItem();
				QStringList runList = getAvailableRuns(sysText);
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

int DBSelectView::getGeometryCount(const std::string& system, const std::string& variation, int run) const {
	int         count = 0;
	std::string query = "SELECT COUNT(*) FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, system.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 4, run);

		if (sqlite3_step(stmt) == SQLITE_ROW) {
			count = sqlite3_column_int(stmt, 0);
		}
	}
	else {
		log->error(ERR_GSQLITEERROR, "SQL Error: Failed togetGeometryCounte:", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	return count;
}

QStringList DBSelectView::getAvailableVariations(const std::string& system) const {
	QStringList   varList;
	sqlite3_stmt* stmt      = nullptr;
	const char*   sql_query = "SELECT DISTINCT variation FROM geometry WHERE system = ?";

	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char* varText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			if (varText) {
				varList << QString::fromUtf8(varText);
			}
		}
	}

	sqlite3_finalize(stmt);
	return varList;
}

QStringList DBSelectView::getAvailableRuns(const std::string& system) {
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

bool DBSelectView::systemAvailable(const std::string& system, const std::string& variation, int run) {
	std::string   query = "SELECT COUNT(*) FROM geometry WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt  = nullptr;

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "SQL Error:systemAvailable: prepare failed:e:", sqlite3_errmsg(db));
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

	// Determine selection tuple from row state.
	int            row        = systemItem->row();
	QStandardItem* varItem    = parentItem->child(row, 2);
	QStandardItem* runItem    = parentItem->child(row, 3);

	QString varStr = varItem ? varItem->data(Qt::EditRole).toString() : "";
	QString runStr = runItem ? runItem->data(Qt::EditRole).toString() : "";

	int     run    = runStr.toInt();
	QString expStr = parentItem->text();

	// Note: the member is updated so subsequent queries use the selected experiment.
	experiment = expStr.toStdString();

	std::string systemName = systemItem->text().toStdString();
	std::string variation  = varStr.toStdString();

	int count = getGeometryCount(systemName, variation, run);

	// Column 1 is the per-row entry count (“volumes”).
	QStandardItem* entriesItem = parentItem->child(row, 1);
	if (entriesItem) {
		entriesItem->setText(QString::number(count));
	}

	// Update availability icon based on whether any matching geometry entries exist.
	bool   available   = (count > 0);
	QColor statusColor = available ? QColor("green") : QColor("red");
	systemItem->setIcon(createStatusIcon(statusColor));

	// Keep the system item readable regardless of icon state.
	systemItem->setData(QColor("white"), Qt::BackgroundRole);
	systemItem->setData(QColor("black"), Qt::ForegroundRole);
}

void DBSelectView::updateExperimentHeader() {
	QStandardItem* selectedExp = nullptr;

	// Find the single checked top-level experiment.
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
	else {
		experimentHeaderLabel->setText("");
	}

	// Ensure headers remain visible after model clear/reset patterns.
	experimentModel->setHorizontalHeaderLabels(QStringList() << "exp/system" << "volumes" << "variation" << "run");
}

void DBSelectView::onItemChanged(QStandardItem* item) {
	if (m_ignoreItemChange || !item)
		return;

	// Guard against recursive updates while changing check states programmatically.
	m_ignoreItemChange = true;

	// Top-level item: experiment selection.
	if (!item->parent()) {
		if (item->checkState() == Qt::Checked) {
			// Enforce only one experiment checked at a time.
			for (int i = 0; i < experimentModel->rowCount(); ++i) {
				QStandardItem* expItem = experimentModel->item(i, 0);
				if (expItem != item)
					expItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		}
		else {
			// If experiment unchecked, also uncheck its systems.
			for (int i = 0; i < item->rowCount(); ++i) {
				QStandardItem* sysItem = item->child(i, 0);
				if (sysItem)
					sysItem->setCheckState(Qt::Unchecked);
			}
			updateExperimentHeader();
		}
	}
	else {
		// Child item: system row change.
		if (item->column() == 0) {
			updateSystemItemAppearance(item);
		}
		else if (item->column() == 2 || item->column() == 3) {
			QStandardItem* sysItem = item->parent()->child(item->row(), 0);
			updateSystemItemAppearance(sysItem);
		}
	}

	m_ignoreItemChange = false;

	// Mark the view as modified and reflect the state in the header/title and reload button.
	if (!modified) {
		modified = true;
	}
	updateModifiedUI();
}

SystemList DBSelectView::get_gsystems() {
	SystemList updatedSystems;

	// Walk the model and build one GSystem per checked system row.
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
				std::string systemName = sysItem->text().toStdString();
				std::string variation  = varItem->data(Qt::EditRole).toString().toStdString();
				int         run        = runItem->data(Qt::EditRole).toInt();

				log->info(2, SFUNCTION_NAME, ": adding systemName: ", systemName, " , variation: ", variation, ", for run:", run);

				updatedSystems.emplace_back(
					std::make_shared<GSystem>(
						gopt,
						dbhost,
						systemName,
						GSYSTEMSQLITETFACTORYLABEL,
						experiment,
						run,
						variation
					));
			}
		}
	}

	return updatedSystems;
}

void DBSelectView::updateModifiedUI() {
	// Keep header text and layout in sync with model state.
	updateExperimentHeader();

	if (modified)
		titleLabel->setText("Experiment Selection* (modified)");
	else
		titleLabel->setText("Experiment Selection");

	reloadButton->setEnabled(modified);

	// Column sizing and tree expansion provide a readable default view after changes.
	experimentTree->resizeColumnToContents(0);
	experimentTree->setColumnWidth(1, 100);
	experimentTree->setColumnWidth(2, 150);
	experimentTree->setColumnWidth(3, 150);
	experimentTree->header()->setStretchLastSection(false);
	experimentTree->expandAll();
}

void DBSelectView::reload_geometry() {
	log->info(0, SFUNCTION_NAME, ": Reloading geometry...");

	// Extract selection into a SystemList and provide visibility into what is being reloaded.
	auto reloaded_system = get_gsystems();
	for (auto& gsys : reloaded_system) {
		log->info(2, SFUNCTION_NAME, ": reloaded system: ", gsys->getName());
	}

	// Delegate the actual reload to detector construction.
	gDetectorConstruction->reload_geometry(reloaded_system);

	// Reload completes the edit cycle: clear modified state.
	modified = false;
	updateModifiedUI();
}
