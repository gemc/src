// pmaker
#include "pmakerView.h"
#include "pmakerTab.h"

// gemc
#include <gemc/glogging/glogger.h>

// qt
#include <QLabel>
#include <QTabBar>
#include <QVBoxLayout>

// c++
#include <algorithm>

// Three non-breaking spaces after the label push text away from the close button
// without touching the tab stylesheet (which breaks the close button rendering).
static QString paddedLabel(const QString& name) {
	return (name.isEmpty() ? QStringLiteral("?") : name) + "   ";
}

// Add a particle tab for `gp` before the '+' sentinel tab and wire the label.
static void insertParticleTab(QTabWidget* tw, const GparticlePtr& gp) {
	auto* tab = new PmakerTab(gp);
	int pos = tw->count() - 1;  // always before the '+' tab
	tw->insertTab(pos, tab, paddedLabel(tab->tabLabel()));
	QObject::connect(tab, &PmakerTab::nameChanged, tw, [tw, tab](const QString& name) {
		tw->setTabText(tw->indexOf(tab), paddedLabel(name));
	});
}


PmakerView::PmakerView(std::shared_ptr<std::vector<GparticlePtr>> particles,
                       const std::shared_ptr<GOptions>& gopts, QWidget* parent)
	: QWidget(parent), GBase(gopts, PMAKER_LOGGER),
	  sharedParticles_(std::move(particles)) {

	auto* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(10, 10, 10, 10);
	mainLayout->setSpacing(10);

	auto* titleLabel = new QLabel("Generator", this);
	QFont titleFont("Avenir", 20, QFont::Bold);
	titleLabel->setFont(titleFont);
	mainLayout->addWidget(titleLabel);

	tabWidget = new QTabWidget(this);
	tabWidget->setMovable(true);
	tabWidget->setTabsClosable(true);
	mainLayout->addWidget(tabWidget);

	// Always-last '+' tab — acts as an "add particle" button; no close button on it.
	tabWidget->addTab(new QWidget(this), "+");
	tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);

	// Populate one tab per gparticle from the shared list.
	if (sharedParticles_) {
		for (const auto& gp : *sharedParticles_)
			insertParticleTab(tabWidget, gp);
	}

	// When no particles are defined, start with one default tab so the UI is not empty.
	if (!sharedParticles_ || sharedParticles_->empty())
		addDefaultParticleTab();

	// Show the first particle tab, not the '+' sentinel.
	tabWidget->setCurrentIndex(0);

	connect(tabWidget, &QTabWidget::tabBarClicked,    this, &PmakerView::onTabClicked);
	connect(tabWidget, &QTabWidget::tabCloseRequested, this, &PmakerView::onTabCloseRequested);
}

void PmakerView::addDefaultParticleTab() {
	auto gp = std::make_shared<Gparticle>(
		"e-", 1,
		1000.0, 0.0, "uniform",
		0.0, 0.0, "uniform",
		0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		"uniform",
		std::make_shared<GLogger>());
	if (sharedParticles_)
		sharedParticles_->push_back(gp);

	auto* tab = new PmakerTab(gp, this);
	int pos = tabWidget->count() - 1;
	tabWidget->insertTab(pos, tab, paddedLabel(tab->tabLabel()));
	tabWidget->setCurrentIndex(pos);
	connect(tab, &PmakerTab::nameChanged, this, [this, tab](const QString& name) {
		tabWidget->setTabText(tabWidget->indexOf(tab), paddedLabel(name));
	});
}

void PmakerView::onTabClicked(int index) {
	if (index == tabWidget->count() - 1)  // '+' tab clicked
		addDefaultParticleTab();
}

void PmakerView::onTabCloseRequested(int index) {
	if (index == tabWidget->count() - 1)  // never remove the '+' tab
		return;
	// Remove the corresponding GparticlePtr from the shared list.
	if (sharedParticles_) {
		if (auto* tab = qobject_cast<PmakerTab*>(tabWidget->widget(index))) {
			auto gp = tab->currentGParticle();
			auto& v = *sharedParticles_;
			v.erase(std::remove(v.begin(), v.end(), gp), v.end());
		}
	}
	tabWidget->removeTab(index);
}
