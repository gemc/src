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

	~G4Dialog() override;


private:
	GBoard*             gboard;
	std::unique_ptr<GUI_Session> gui_session;
};
