#pragma once

// g4dialog
#include "tabs/gboard.h"
#include "gui_session.h"
#include "g4dialog_options.h"

// qt
#include <QtWidgets>

// gemc
#include "gbase.h"

class G4Dialog : public GBase<G4Dialog>, public QWidget {

public:
	explicit G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent = nullptr);

	// TODO:
	// logger life cycle management: this will not be called
	// also this only works if log is declared at the end of the class
	// otherwise the lifetime of log is shorter then boards
	// maybe related: That means you can't predict exactly when GLogger will be destroyed
	// it could be after Qt and Geant4 are already partially torn down
	// it could be that qt destructor is called before GLogger destructor

private:
	std::shared_ptr<GBoard>      gboard;
	std::unique_ptr<GUI_Session> gui_session;
};
