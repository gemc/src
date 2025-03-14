#include "dbselectView.h"
#include <cstdlib>
#include <iostream>

DBSelectView::DBSelectView(GOptions *gopts, QWidget *parent)
		: QWidget(parent), db(nullptr), m_ignoreItemChange(false)
{
	std::string database;
	if (gopts->doesOptionExist("gsystem")) {
		// database = gopts->getOption("gsystem");  // assuming this method exists
	} else {
		database = std::string(getenv("GEMC")) + "/examples/gemc.db";
	}
	if (sqlite3_open(database.c_str(), &db) != SQLITE_OK) {
		QMessageBox::critical(this, "Database Error", "Failed to open database");
	}
	setupUI();
	loadExperiments();
	loadAllVariations();
	loadAllRuns();

	// Update systems based on current filter.
	std::string currentVariation = variationComboBox->currentText().toStdString();
	int currentRun = runComboBox->currentText().isEmpty() ? -1 : runComboBox->currentText().toInt();
	filter_systems(currentVariation, currentRun);
}

DBSelectView::~DBSelectView() {
	if (db)
		sqlite3_close(db);
}

void DBSelectView::setupUI() {
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10,10,10,10);
	mainLayout->setSpacing(10);

	// Title label.
	QLabel *titleLabel = new QLabel("Experiment Selection", this);
	QFont titleFont("Arial", 18, QFont::Bold);
	titleLabel->setFont(titleFont);
	titleLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(titleLabel);

	// Experiment header label: will display total number of systems.
	experimentHeaderLabel = new QLabel("", this);
	experimentHeaderLabel->setAlignment(Qt::AlignCenter);
	mainLayout->addWidget(experimentHeaderLabel);

	// Create main horizontal layout to hold the experiment tree and filter controls.
	QHBoxLayout *mainHLayout = new QHBoxLayout();
	mainHLayout->setSpacing(20);
	mainLayout->addLayout(mainHLayout);

	// ----- Experiment Tree Container -----
	QWidget *experimentContainer = new QWidget(this);
	QVBoxLayout *expLayout = new QVBoxLayout(experimentContainer);
	expLayout->setSpacing(5);

	QLabel *expTitleLabel = new QLabel("Experiments", experimentContainer);
	QFont expTitleFont("Arial", 14, QFont::Bold);
	expTitleLabel->setFont(expTitleFont);
	expTitleLabel->setAlignment(Qt::AlignCenter);
	expLayout->addWidget(expTitleLabel);

	experimentTree = new QTreeView(experimentContainer);
	QFont expTreeFont("Arial", 12);
	experimentTree->setFont(expTreeFont);
	experimentTree->setStyleSheet("QTreeView { alternate-background-color: #f0f0f0; }");
	expLayout->addWidget(experimentTree);

	experimentModel = new QStandardItemModel(this);
	experimentTree->setModel(experimentModel);
	// Hide header to remove default "1".
	experimentTree->header()->hide();
	experimentTree->setSelectionMode(QAbstractItemView::SingleSelection);
	experimentTree->setSelectionBehavior(QAbstractItemView::SelectRows);

	// Connect the model's itemChanged signal to our combined slot.
	connect(experimentModel, &QStandardItemModel::itemChanged,
			this, &DBSelectView::onItemChanged);

	mainHLayout->addWidget(experimentContainer, /*stretch=*/2);

	// ----- Filters Container: Vertical layout for Variation and Run ---
	QWidget *filterContainer = new QWidget(this);
	QVBoxLayout *filterLayout = new QVBoxLayout(filterContainer);
	filterLayout->setSpacing(5);

	QLabel *varLabel = new QLabel("Variations", filterContainer);
	varLabel->setFont(QFont("Arial", 12, QFont::Bold));
	varLabel->setAlignment(Qt::AlignCenter);
	filterLayout->addWidget(varLabel);

	variationComboBox = new QComboBox(filterContainer);
	variationComboBox->setFont(QFont("Arial", 10));
	filterLayout->addWidget(variationComboBox);

	QLabel *runLabel = new QLabel("Runs", filterContainer);
	runLabel->setFont(QFont("Arial", 12, QFont::Bold));
	runLabel->setAlignment(Qt::AlignCenter);
	filterLayout->addWidget(runLabel);

	runComboBox = new QComboBox(filterContainer);
	runComboBox->setFont(QFont("Arial", 10));
	filterLayout->addWidget(runComboBox);

	mainHLayout->addWidget(filterContainer, /*stretch=*/1);

	// Connect filter combobox signals.
	connect(variationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &DBSelectView::onVariationSelected);
	connect(runComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, &DBSelectView::onRunSelected);
}

void DBSelectView::loadExperiments() {
	if (!experimentModel) {
		std::cerr << "experimentModel is not initialized!" << std::endl;
		return;
	}
	experimentModel->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT experiment FROM geometry";
	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const char *expText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		if(expText) {
			std::string expName = expText;
			std::cout << "Loaded experiment: " << expName << std::endl;
			QStandardItem *expItem = new QStandardItem(QString::fromUtf8(expText));
			// Make experiment item read‑only and checkable.
			expItem->setFlags(expItem->flags() & ~Qt::ItemIsEditable);
			expItem->setCheckable(true);
			expItem->setCheckState(Qt::Unchecked);
			loadSystemsForExperiment(expItem, expName);
			experimentModel->appendRow(expItem);
		} else {
			std::cerr << "Null experiment text encountered." << std::endl;
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
			// Make system item read‑only but checkable.
			sysItem->setFlags(sysItem->flags() & ~Qt::ItemIsEditable);
			sysItem->setCheckable(true);
			sysItem->setCheckState(Qt::Unchecked);
			experimentItem->appendRow(sysItem);
		}
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadAllVariations() {
	variationComboBox->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT variation FROM geometry";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			const char *varText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
			variationComboBox->addItem(QString::fromUtf8(varText));
		}
	} else {
		std::cerr << "Failed to load variations: " << sqlite3_errmsg(db) << std::endl;
	}
	sqlite3_finalize(stmt);
}

void DBSelectView::loadAllRuns() {
	runComboBox->clear();
	sqlite3_stmt *stmt = nullptr;
	const char *sql_query = "SELECT DISTINCT run FROM geometry";
	if (sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr) == SQLITE_OK) {
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int runVal = sqlite3_column_int(stmt, 0);
			runComboBox->addItem(QString::number(runVal));
		}
	} else {
		std::cerr << "Failed to load runs: " << sqlite3_errmsg(db) << std::endl;
	}
	sqlite3_finalize(stmt);
}

bool DBSelectView::systemAvailable(const std::string &system, const std::string &variation, int run) {
	std::string query;
	if (run == -1) {
		query = "SELECT COUNT(*) FROM geometry WHERE system = ? AND variation = ?";
	} else {
		query = "SELECT COUNT(*) FROM geometry WHERE system = ? AND variation = ? AND run = ?";
	}
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "systemAvailable: prepare failed: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	sqlite3_bind_text(stmt, 1, system.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_TRANSIENT);
	if (run != -1) {
		sqlite3_bind_int(stmt, 3, run);
	}
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

void DBSelectView::updateSystemItemAppearance(QStandardItem *systemItem, bool available) {
	QColor statusColor = available ? QColor("green") : QColor("red");
	systemItem->setIcon(createStatusIcon(statusColor));
	// Keep default colors.
	systemItem->setData(QColor("white"), Qt::BackgroundRole);
	systemItem->setData(QColor("black"), Qt::ForegroundRole);
}


void DBSelectView::filter_systems(const std::string &variation, int run) {
	bool foundSelected = false;
	for (int i = 0; i < experimentModel->rowCount(); ++i) {
		QStandardItem *expItem = experimentModel->item(i);
		if (expItem->checkState() == Qt::Checked) {
			foundSelected = true;
			for (int j = 0; j < expItem->rowCount(); ++j) {
				QStandardItem *systemItem = expItem->child(j);
				bool available = systemAvailable(systemItem->text().toStdString(), variation, run);
				// Force uncheck if not available.
				if (!available)
					systemItem->setCheckState(Qt::Unchecked);
				updateSystemItemAppearance(systemItem, available);
			}
			// Update header label with total systems count.
			int totalSystems = expItem->rowCount();
			experimentHeaderLabel->setText(QString("Total systems for experiment \"%1\": %2")
												   .arg(expItem->text()).arg(totalSystems));
			// Since experiments are mutually exclusive, break after finding the selected one.
			break;
		}
	}
	// If no experiment is selected, update every system with default icon (red) and clear header.
	if (!foundSelected) {
		for (int i = 0; i < experimentModel->rowCount(); ++i) {
			QStandardItem *expItem = experimentModel->item(i);
			for (int j = 0; j < expItem->rowCount(); ++j) {
				QStandardItem *systemItem = expItem->child(j);
				updateSystemItemAppearance(systemItem, false);
			}
		}
		experimentHeaderLabel->setText("");
	}
}


void DBSelectView::onItemChanged(QStandardItem *item) {
	if (m_ignoreItemChange)
		return;
	if (!item)
		return;
	m_ignoreItemChange = true;
	if (!item->parent()) {
		// Experiment item.
		if (item->checkState() == Qt::Checked) {
			// Uncheck all other experiments.
			for (int i = 0; i < experimentModel->rowCount(); ++i) {
				QStandardItem *expItem = experimentModel->item(i);
				if (expItem != item)
					expItem->setCheckState(Qt::Unchecked);
			}
			// Update header with the experiment's name and total systems count.
			int totalSystems = item->rowCount();
			experimentHeaderLabel->setText(QString("Total systems for experiment \"%1\": %2")
												   .arg(item->text()).arg(totalSystems));
		} else {
			// Experiment unchecked: uncheck all its child systems.
			for (int i = 0; i < item->rowCount(); ++i) {
				QStandardItem *systemItem = item->child(i);
				systemItem->setCheckState(Qt::Unchecked);
			}
			experimentHeaderLabel->setText("");
		}
	} else {
		// System item.
		std::string currentVariation = variationComboBox->currentText().toStdString();
		int currentRun = runComboBox->currentText().isEmpty() ? -1 : runComboBox->currentText().toInt();
		bool available = systemAvailable(item->text().toStdString(), currentVariation, currentRun);
		if (!available && item->checkState() == Qt::Checked)
			item->setCheckState(Qt::Unchecked);
		updateSystemItemAppearance(item, available);
	}
	m_ignoreItemChange = false;
}


void DBSelectView::onVariationSelected(int index) {
	if (index == -1)
		return;
	std::string selectedVariation = variationComboBox->currentText().toStdString();
	std::cout << "Variation selected: " << selectedVariation << std::endl;
	int selectedRun = runComboBox->currentText().isEmpty() ? -1 : runComboBox->currentText().toInt();
	filter_systems(selectedVariation, selectedRun);
}

void DBSelectView::onRunSelected(int index) {
	if (index == -1)
		return;
	int selectedRun = runComboBox->currentText().isEmpty() ? -1 : runComboBox->currentText().toInt();
	std::cout << "Run selected: " << selectedRun << std::endl;
	std::string selectedVariation = variationComboBox->currentText().toStdString();
	filter_systems(selectedVariation, selectedRun);
}
