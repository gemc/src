#include "physicsView.h"

#include <gemc/gphysics/gphysicsRuntime.h>
#include "G4SystemOfUnits.hh"

#include <QComboBox>
#include <QCompleter>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>

namespace {

void fillCombo(QComboBox* combo, const std::vector<std::string>& names) {
	for (const auto& name : names) { combo->addItem(QString::fromStdString(name)); }
	combo->setEditable(true);
	combo->setInsertPolicy(QComboBox::NoInsert);
	combo->completer()->setFilterMode(Qt::MatchContains);
	combo->completer()->setCompletionMode(QCompleter::PopupCompletion);
	combo->setMinimumContentsLength(28);
	combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
}

} // namespace

PhysicsView::PhysicsView(std::shared_ptr<GOptions> input, QWidget* parent)
	: QWidget(parent), options(std::move(input)) {
	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);
	auto* title = new QLabel(tr("Physics"), this);
	title->setFont(QFont("Avenir", 20, QFont::Bold));
	layout->addWidget(title);

	auto* configuration = new QGroupBox(tr("Physics list"), this);
	auto* form = new QFormLayout(configuration);
	reference = new QComboBox(configuration);
	reference->setObjectName("gemcPhysicsReference");
	fillCombo(reference, gphysics::RuntimePhysicsList::referenceLists());
	const auto* physics = gphysics::RuntimePhysicsList::current();
	const auto initial = QString::fromStdString(physics ? physics->selection() :
	                                         options->getRequiredScalarString("phys_list"));
	const auto parts = initial.split('+', Qt::SkipEmptyParts);
	if (!parts.empty() && reference->findText(parts.first().trimmed()) < 0) {
		reference->addItem(parts.first().trimmed());
	}
	reference->setCurrentText(parts.empty() ? "FTFP_BERT" : parts.first().trimmed());
	form->addRow(tr("Reference list"), reference);

	cut = new QDoubleSpinBox(configuration);
	cut->setObjectName("gemcProductionCut");
	cut->setDecimals(6);
	cut->setRange(0.000001, 1000000);
	cut->setSuffix(" mm");
	cut->setValue(physics ? physics->GetDefaultCutValue() / mm : 0.7);
	cut->setToolTip(tr("Range cut for producing secondary gamma, electron, positron, and proton tracks. "
	                   "This does not stop existing tracks below an energy threshold."));
	form->addRow(tr("Production range cut"), cut);
	layout->addWidget(configuration);

	additions = new QTabWidget(this);
	additions->setObjectName("gemcPhysicsConstructors");
	additions->setTabsClosable(true);
	additions->addTab(new QWidget(additions), "+");
	additions->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
	additions->setToolTip(tr("Add a physics constructor to the reference list."));
	for (int i = 1; i < parts.size(); ++i) { addConstructor(parts[i].trimmed()); }
	layout->addWidget(additions);
	connect(additions, &QTabWidget::tabBarClicked, this, [this](int index) {
		if (index == additions->count() - 1) { addConstructor(); markPending(); }
	});
	connect(additions, &QTabWidget::tabCloseRequested, this, [this](int index) {
		if (index == additions->count() - 1) { return; }
		auto* page = additions->widget(index);
		additions->removeTab(index);
		page->deleteLater();
		markPending();
	});

	auto* processes = new QGroupBox(tr("Processes"), this);
	auto* processLayout = new QVBoxLayout(processes);
	auto* description = new QLabel(
		tr("Check processes to enable them. Category checkboxes control the whole group. "
		   "Combined processes are controlled as a single entry."), processes);
	description->setWordWrap(true);
	processLayout->addWidget(description);
	auto* filter = new QLineEdit(processes);
	filter->setPlaceholderText(tr("Filter processes"));
	processLayout->addWidget(filter);
	processTree = new QTreeWidget(processes);
	processTree->setObjectName("gemcPhysicsProcesses");
	processTree->setHeaderLabels({tr("Process"), tr("Applies to")});
	processTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	processLayout->addWidget(processTree);
	layout->addWidget(processes, 1);
	connect(filter, &QLineEdit::textChanged, this, [this](const QString& text) {
		for (int i = 0; i < processTree->topLevelItemCount(); ++i) {
			auto* group = processTree->topLevelItem(i);
			bool visible = false;
			for (int j = 0; j < group->childCount(); ++j) {
				auto* item = group->child(j);
				const bool match = item->text(0).contains(text, Qt::CaseInsensitive) ||
				                   group->text(0).contains(text, Qt::CaseInsensitive);
				item->setHidden(!match);
				visible |= match;
			}
			group->setHidden(!visible);
		}
	});
	connect(processTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column) {
		if (refreshing || column != 0 || !item->parent() ||
		    !(item->flags() & Qt::ItemIsUserCheckable)) { return; }
		pendingActivation[item->data(0, Qt::UserRole).toString().toStdString()] =
			item->checkState(0) == Qt::Checked;
		markPending();
	});
	status = new QLabel(this);
	status->setObjectName("gemcPhysicsStatus");
	status->setWordWrap(true);
	layout->addWidget(status);
	auto* apply = new QPushButton(tr("Apply physics"), this);
	apply->setObjectName("gemcApplyPhysics");
	connect(apply, &QPushButton::clicked, this, [this]() { applyPending(); });
	layout->addWidget(apply, 0, Qt::AlignRight);
	connect(reference, &QComboBox::currentTextChanged, this, &PhysicsView::markPending);
	connect(cut, &QDoubleSpinBox::valueChanged, this, &PhysicsView::markPending);
	refreshProcesses();
	pending = false;
	status->setText(tr("Changes are applied before the next run, or with Apply physics."));
}

void PhysicsView::addConstructor(const QString& requested) {
	auto* page = new QWidget(additions);
	auto* layout = new QFormLayout(page);
	auto* combo = new QComboBox(page);
	combo->setObjectName("gemcPhysicsConstructor");
	fillCombo(combo, gphysics::RuntimePhysicsList::constructors());
	QString name = requested;
	if (name.isEmpty()) {
		name = "G4OpticalPhysics";
		if (selection().split('+').contains(name)) {
			for (int i = 0; i < combo->count(); ++i) {
				if (!selection().split('+').contains(combo->itemText(i))) { name = combo->itemText(i); break; }
			}
		}
	}
	if (combo->findText(name) < 0) { combo->addItem(name); }
	combo->setCurrentText(name);
	layout->addRow(tr("Physics constructor"), combo);
	const int index = additions->insertTab(additions->count() - 1, page, name + "   ");
	additions->setCurrentIndex(index);
	connect(combo, &QComboBox::currentTextChanged, this, [this, page](const QString& text) {
		additions->setTabText(additions->indexOf(page), text + "   ");
		markPending();
	});
}

QString PhysicsView::selection() const {
	QStringList parts{reference->currentText().trimmed()};
	for (int i = 0; i < additions->count() - 1; ++i) {
		parts.push_back(additions->widget(i)->findChild<QComboBox*>()->currentText().trimmed());
	}
	return parts.join('+');
}

void PhysicsView::markPending() {
	pending = true;
	status->setText(tr("Physics changes pending. They will be applied before the next run."));
}

bool PhysicsView::applyPending() {
	if (!pending) { return true; }
	auto* physics = gphysics::RuntimePhysicsList::current();
	if (!physics) {
		status->setText(tr("Physics controls require a GEMC run manager."));
		return false;
	}
	std::string error;
	if (!physics->select(selection().toStdString(), error) ||
	    !physics->setProductionCut(cut->value(), error)) {
		status->setText(QString::fromStdString(error));
		return false;
	}
	const auto available = physics->processes();
	for (const auto& [name, enabled] : pendingActivation) {
		if (std::any_of(available.begin(), available.end(), [&](const auto& item) { return item.name == name; }) &&
		    !physics->setProcessEnabled(name, enabled, error)) {
			status->setText(QString::fromStdString(error));
			return false;
		}
	}
	options->setOptionValueFromString("phys_list", physics->selection());
	options->setOptionValueFromString("production_cut", std::to_string(cut->value()));
	pendingActivation.clear();
	pending = false;
	refreshProcesses();
	status->setText(tr("Physics applied. The next run uses these settings."));
	return true;
}

void PhysicsView::refreshProcesses() {
	refreshing = true;
	processTree->clear();
	const auto* physics = gphysics::RuntimePhysicsList::current();
	if (physics) {
		std::map<std::string, QTreeWidgetItem*> groups;
		const std::map<std::string, QString> labels{
			{"Electromagnetic", tr("Electromagnetic")}, {"Hadronic", tr("Hadronic")},
			{"Optical", tr("Optical")}, {"Decay", tr("Decay")}, {"General", tr("General")},
			{"Transportation", tr("Transportation")}, {"Other", tr("Other")},
			{"Parameterisation", tr("Parameterisation")}, {"Parallel worlds", tr("Parallel worlds")}};
		for (const auto& process : physics->processes()) {
			auto*& group = groups[process.category];
			if (!group) {
				group = new QTreeWidgetItem(processTree, {labels.at(process.category)});
				group->setFlags(group->flags() | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable);
			}
			QStringList particles;
			for (const auto& name : process.particles) { particles.push_back(QString::fromStdString(name)); }
			const auto coverage = particles.size() == 1 ? particles.first() :
			                      tr("%1 particle types").arg(particles.size());
			auto* item = new QTreeWidgetItem(group, {QString::fromStdString(process.name), coverage});
			item->setToolTip(1, particles.join(", "));
			item->setData(0, Qt::UserRole, QString::fromStdString(process.name));
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			const auto pendingChoice = pendingActivation.find(process.name);
			item->setCheckState(0, pendingChoice != pendingActivation.end() ?
				(pendingChoice->second ? Qt::Checked : Qt::Unchecked) :
				process.mixed ? Qt::PartiallyChecked : process.active ? Qt::Checked : Qt::Unchecked);
			if (!process.editable) {
				item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
				group->setFlags(group->flags() & ~Qt::ItemIsUserCheckable);
				item->setToolTip(0, tr("Transportation is required for tracking."));
			}
		}
		processTree->sortItems(0, Qt::AscendingOrder);
		for (const auto& [name, group] : groups) {
			group->setExpanded(name == "Optical");
		}
	}
	if (!processTree->topLevelItemCount()) {
		new QTreeWidgetItem(processTree, {tr("Processes appear after physics initialization.")});
	}
	refreshing = false;
}
