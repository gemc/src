#pragma once

// qt
#include <QtWidgets>

// gemc
#include "gbase.h"

class G4Dialog : public GBase<G4Dialog>, public QWidget {

public:
	explicit G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent = nullptr);
	~G4Dialog() override = default;

};
