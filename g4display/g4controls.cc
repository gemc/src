// g4display
#include "g4controls.h"
#include "tabs/g4displaycamera.h"
#include "tabs/g4displayslice.h"

G4Controls::G4Controls(GOptions* gopt, QWidget* parent) : QTabWidget(parent)
{
	setStyleSheet("QTabBar::tab       { background-color: #ACB6B6;}"
					  "QTabBar::tab:focus { color: #000011; }");

	addTab(new G4DisplayCamera(gopt, this), "Camera / Light");
	addTab(new G4DisplaySlice(gopt, this),  "Slices");
	addTab(new QWidget(),"Display Options");
}
