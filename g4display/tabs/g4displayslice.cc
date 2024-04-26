// g4display
#include "g4displayslice.h"

// c++
#include <string>
using namespace std;


G4DisplaySlice::G4DisplaySlice([[maybe_unused]] GOptions* gopt, QWidget* parent) : QWidget(parent)
{
	// x slice
	sliceXEdit = new QLineEdit(tr("0"));
	sliceXEdit->setMaximumWidth(100);

	sliceXActi = new QCheckBox(tr("&On"));
	sliceXActi->setChecked(false);

	sliceXInve = new QCheckBox(tr("&Flip"));
	sliceXInve->setChecked(false);

	QHBoxLayout *sliceXLayout = new QHBoxLayout;
	sliceXLayout->addWidget(new QLabel(tr("X: ")));
	sliceXLayout->addWidget(sliceXEdit);
	sliceXLayout->addStretch(1);
	sliceXLayout->addWidget(sliceXActi);
	sliceXLayout->addWidget(sliceXInve);
	sliceXLayout->addStretch(1);


	// y slice
	sliceYEdit = new QLineEdit(tr("0"));
	sliceYEdit->setMaximumWidth(100);

	sliceYActi = new QCheckBox(tr("&On"));
	sliceYActi->setChecked(false);

	sliceYInve = new QCheckBox(tr("&Flip"));
	sliceYInve->setChecked(false);

	QHBoxLayout *sliceYLayout = new QHBoxLayout;
	sliceYLayout->addWidget(new QLabel(tr("Y: ")));
	sliceYLayout->addWidget(sliceYEdit);
	sliceYLayout->addStretch(1);
	sliceYLayout->addWidget(sliceYActi);
	sliceYLayout->addWidget(sliceYInve);
	sliceYLayout->addStretch(1);



	// z slice
	sliceZEdit = new QLineEdit(tr("0"));
	sliceZEdit->setMaximumWidth(100);

	sliceZActi = new QCheckBox(tr("&On"));
	sliceZActi->setChecked(false);

	sliceZInve = new QCheckBox(tr("&Flip"));
	sliceZInve->setChecked(false);

	QHBoxLayout *sliceZLayout = new QHBoxLayout;
	sliceZLayout->addWidget(new QLabel(tr("Z: ")));
	sliceZLayout->addWidget(sliceZEdit);
	sliceZLayout->addStretch(1);
	sliceZLayout->addWidget(sliceZActi);
	sliceZLayout->addWidget(sliceZInve);
	sliceZLayout->addStretch(1);


	// clear sice button
	QPushButton *clearSliceButton = new QPushButton(tr("Clear Slices"));
	clearSliceButton->setToolTip("Clear Slice Planes");
	clearSliceButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	connect ( clearSliceButton , SIGNAL(clicked()), this, SLOT( clearSlices() ) );


	// slice style: Intersection or Union
	QGroupBox *sliceChoiceBox = new QGroupBox(tr("Slices Style"));
	sliceSectn = new QRadioButton(tr("&Intersection"), sliceChoiceBox);
	sliceUnion = new QRadioButton(tr("&Union"),        sliceChoiceBox);
	sliceSectn->setChecked(true);

	connect ( sliceSectn , SIGNAL(clicked()), this, SLOT( slice() ) );
	connect ( sliceUnion , SIGNAL(clicked()), this, SLOT( slice() ) );

	QHBoxLayout *sliceChoiceLayout = new QHBoxLayout;
	sliceChoiceLayout->addWidget(sliceSectn);
	sliceChoiceLayout->addWidget(sliceUnion);
	sliceChoiceBox->setLayout(sliceChoiceLayout);


	// slices layout
	QVBoxLayout *sliceLayout = new QVBoxLayout;
	sliceLayout->addLayout(sliceXLayout);
	sliceLayout->addLayout(sliceYLayout);
	sliceLayout->addLayout(sliceZLayout);
	sliceLayout->addWidget(sliceChoiceBox);
	sliceLayout->addWidget(clearSliceButton);


	// connecting widgets to slice
	connect ( sliceXEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );
	connect ( sliceYEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );
	connect ( sliceZEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );

	connect ( sliceXActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceYActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceZActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );

	connect ( sliceXInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceYInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceZInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );


	// all layouts together
	// --------------------
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(sliceLayout);
	setLayout(mainLayout);
}


void G4DisplaySlice::slice()
{
	G4UImanager *g4uim = G4UImanager::GetUIpointer();
    if (g4uim == nullptr) {
        return;
    }

	// can't have a mix of wireframe / solid when doing a slice.
	// forcing all to be solid
//	if(!solidVis) {
//		g4uim->ApplyCommand("/vis/geometry/set/forceSolid all -1 1");
//	}

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if(sliceSectn->isChecked()) {
		g4uim->ApplyCommand("/vis/viewer/set/cutawayMode intersection");
	} else if(sliceUnion->isChecked()) {
		g4uim->ApplyCommand("/vis/viewer/set/cutawayMode union");
	}

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if(sliceXActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane "     + sliceXEdit->text().toStdString() + " 0  0 mm " + to_string(sliceXInve->isChecked() ? -1 : 1) + " 0 0 ";
		cout << "X " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if(sliceYActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane 0 "   + sliceYEdit->text().toStdString() + " 0 mm 0 " + to_string(sliceYInve->isChecked() ? -1 : 1) + " 0 ";
		cout << "Y " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if(sliceZActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane 0 0 " + sliceZEdit->text().toStdString() + " mm 0 0 " + to_string(sliceZInve->isChecked() ? -1 : 1);
		cout << "Z " << command << endl;
		g4uim->ApplyCommand(command);
	}

	//solidVis = true;

}

void G4DisplaySlice::clearSlices()
{
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	sliceXActi->setChecked(false);
	sliceYActi->setChecked(false);
	sliceZActi->setChecked(false);
}
