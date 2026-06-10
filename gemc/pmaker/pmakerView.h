#pragma once

// qt
#include <QWidget>
#include <QTabWidget>

// gemc
#include <gemc/gbase/gbase.h>
#include <gemc/gparticle/gparticle.h>
#include <gemc/pmaker/pmaker_options.h>

class PmakerTab;

/**
 * \brief Right-panel page that manages the event generator configuration.
 *
 * Displays one QTabWidget tab per Gparticle loaded from the YAML options.
 * The last tab is always a '+' button that inserts a new default-particle tab
 * when clicked. Tab labels track the particle name field in real time.
 */
class PmakerView : public QWidget, public GBase<PmakerView> {
	Q_OBJECT
public:
	PmakerView(std::shared_ptr<std::vector<GparticlePtr>> particles,
	           const std::shared_ptr<GOptions>& gopts,
	           QWidget* parent = nullptr);

private:
	void addDefaultParticleTab();

	QTabWidget* tabWidget = nullptr;
	std::shared_ptr<std::vector<GparticlePtr>> sharedParticles_;

private slots:
	void onTabClicked(int index);
	void onTabCloseRequested(int index);
};
