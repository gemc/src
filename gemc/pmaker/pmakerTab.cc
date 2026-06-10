// pmaker
#include "pmakerTab.h"

// gemc
#include <gemc/gparticle/gparticleConventions.h>

// qt
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QSlider>
#include <QVBoxLayout>
#include <QFrame>

// clhep
#include <CLHEP/Units/SystemOfUnits.h>

// geant4
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"

namespace {

QDoubleSpinBox* momentumSpin(QWidget* parent) {
	auto* s = new QDoubleSpinBox(parent);
	s->setRange(0.0, 1.0e9);
	s->setDecimals(2);
	s->setSingleStep(100.0);
	s->setSuffix(" MeV");
	return s;
}

QDoubleSpinBox* angleSpin(QWidget* parent) {
	auto* s = new QDoubleSpinBox(parent);
	s->setRange(-360.0, 360.0);
	s->setDecimals(3);
	s->setSuffix(" deg");
	return s;
}

QDoubleSpinBox* vertexSpin(QWidget* parent) {
	auto* s = new QDoubleSpinBox(parent);
	s->setRange(-1.0e6, 1.0e6);
	s->setDecimals(3);
	s->setSuffix(" cm");
	return s;
}

QComboBox* makeCombo(const QStringList& items, QWidget* parent) {
	auto* c = new QComboBox(parent);
	c->addItems(items);
	return c;
}

QComboBox* makeParticleCombo(QWidget* parent) {
	// Preferred particles always appear first.
	static const QStringList preferred = {
		"e-", "e+", "proton", "pi+", "pi-", "pi0",
		"neutron", "geantino", "mu+", "mu-"
	};

	QStringList all = preferred;

	// Append every other particle known to G4ParticleTable, sorted.
	auto* table = G4ParticleTable::GetParticleTable();
	if (table && table->entries() > 0) {
		auto* iter = table->GetIterator();
		iter->reset();
		QStringList rest;
		while ((*iter)()) {
			QString pname = QString::fromStdString(iter->value()->GetParticleName());
			if (!preferred.contains(pname))
				rest << pname;
		}
		rest.sort(Qt::CaseInsensitive);
		all += rest;
	}

	auto* c = new QComboBox(parent);
	c->addItems(all);
	return c;
}

// Circular widget showing angular coverage as a grey pie + a red centre line.
// Convention: 0° = right (3 o'clock), CCW positive — identical to Qt's own system,
// so no coordinate mapping is needed.
// The grey sector spans [center - delta, center + delta]; delta = 180 fills the circle.
class AngleCoverageWidget : public QWidget {
public:
	explicit AngleCoverageWidget(QWidget* parent = nullptr) : QWidget(parent) {
		setFixedSize(90, 90);
	}
	void setCenter(double deg) { center_ = deg; update(); }
	void setDelta(double deg)  { delta_  = deg; update(); }

protected:
	void paintEvent(QPaintEvent*) override {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		constexpr int m = 4;
		QRect r(m, m, width() - 2*m, height() - 2*m);
		painter.setBrush(Qt::white);
		painter.setPen(Qt::NoPen);
		painter.drawEllipse(r);
		if (delta_ > 0.0) {
			// Qt: 0=right, CCW positive, units = 1/16th degree.
			int qtStart = qRound((center_ - delta_) * 16.0);
			int qtSpan  = qRound(qMin(delta_ * 2.0, 360.0) * 16.0);
			painter.setBrush(QColor(160, 160, 160));
			painter.drawPie(r, qtStart, qtSpan);
		}
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen(Qt::black, 1.5));
		painter.drawEllipse(r);
		// Red line at the centre angle (y-axis points down in screen coords)
		double rad    = qDegreesToRadians(center_);
		double cx     = r.center().x();
		double cy     = r.center().y();
		double radius = r.width() / 2.0 - 1;
		painter.setPen(QPen(Qt::red, 2.0));
		painter.drawLine(QPointF(cx, cy),
		                 QPointF(cx + radius * qCos(rad),
		                         cy - radius * qSin(rad)));
	}

private:
	double center_ = 0.0;
	double delta_  = 0.0;
};

} // namespace


PmakerTab::PmakerTab(const GparticlePtr& gp, QWidget* parent)
	: QWidget(parent), gp_(gp) {
	double p_mev = gp->getMomentum();
	if (p_mev < 0.0)  // GPARTICLENOTDEFINED sentinel
		p_mev = 0.0;

	buildLayout(
		gp->getName(),
		gp->getMultiplicity(),
		p_mev,
		gp->getDeltaMomentum(),
		QString::fromStdString(gp->getMomentumModel()),
		gp->getTheta()           / CLHEP::degree,
		gp->getDeltaTheta()      / CLHEP::degree,
		QString::fromStdString(gp->getThetaModel()),
		gp->getPhi()             / CLHEP::degree,
		gp->getDeltaPhi()        / CLHEP::degree,
		gp->getVertex().x()      / CLHEP::cm,
		gp->getVertex().y()      / CLHEP::cm,
		gp->getVertex().z()      / CLHEP::cm,
		gp->getDeltaVertex().x() / CLHEP::cm,
		gp->getDeltaVertex().y() / CLHEP::cm,
		gp->getDeltaVertex().z() / CLHEP::cm,
		QString::fromStdString(gp->getVertexModel())
	);
}

PmakerTab::PmakerTab(QWidget* parent)
	: QWidget(parent),
	  gp_(std::make_shared<Gparticle>(
		  "e-", 1,
		  1000.0, 0.0, "uniform",
		  0.0, 0.0, "uniform",
		  0.0, 0.0,
		  0.0, 0.0, 0.0,
		  0.0, 0.0, 0.0,
		  "uniform",
		  std::make_shared<GLogger>())) {
	buildLayout("e-", 1,
	            1000.0, 0.0, "uniform",
	            0.0,    0.0, "uniform",
	            0.0,    0.0,
	            0.0, 0.0, 0.0,
	            0.0, 0.0, 0.0,
	            "uniform");
}

void PmakerTab::onParticleChanged(const QString& name) {
	if (multLabel)
		multLabel->setText("Number of " + name + "s per event:");
	if (gp_)
		gp_->setName(name.toStdString());
	emit nameChanged(name);
}

QString PmakerTab::tabLabel() const {
	return particleCombo ? particleCombo->currentText() : QStringLiteral("?");
}

void PmakerTab::buildLayout(const std::string& name,
                             int    mult,
                             double p_mev,     double dp_mev,     const QString& pModel,
                             double theta_deg, double dtheta_deg, const QString& thetaModel,
                             double phi_deg,   double dphi_deg,
                             double vx_cm,     double vy_cm,     double vz_cm,
                             double dvx_cm,    double dvy_cm,    double dvz_cm,
                             const QString& vertexModel) {

	// ---- Identity ----
	particleCombo = makeParticleCombo(this);
	particleCombo->setCurrentText(QString::fromStdString(name));

	multSpin = new QSpinBox(this);
	multSpin->setRange(1, 10000);
	multSpin->setValue(mult);
	multLabel = new QLabel("Number of " + particleCombo->currentText() + "s per event:", this);

	auto* identityLayout = new QFormLayout;
	identityLayout->addRow("Particle:", particleCombo);
	identityLayout->addRow(multLabel, multSpin);
	auto* identityBox = new QGroupBox("Particle");
	identityBox->setLayout(identityLayout);

	connect(particleCombo, &QComboBox::currentTextChanged, this, &PmakerTab::onParticleChanged);

	// ---- Momentum ----
	pSpin  = momentumSpin(this);  pSpin->setValue(p_mev);
	dpSpin = momentumSpin(this);  dpSpin->setValue(dp_mev);
	pModelCombo = makeCombo({"uniform", "gaussian"}, this);
	pModelCombo->setCurrentText(pModel);

	auto* momRow = new QHBoxLayout;
	momRow->addWidget(new QLabel("p (MeV):"));
	momRow->addWidget(pSpin);
	momRow->addWidget(new QLabel("\302\261"));
	momRow->addWidget(dpSpin);
	momRow->addSpacing(8);
	momRow->addWidget(new QLabel("model:"));
	momRow->addWidget(pModelCombo);
	momRow->addStretch();
	auto* momBox = new QGroupBox("Momentum");
	auto* momLayout = new QVBoxLayout(momBox);
	momLayout->addLayout(momRow);

	// ---- Angles ----
	thetaSpin       = angleSpin(this);  thetaSpin->setRange(-180.0, 180.0);  thetaSpin->setValue(theta_deg);
	dthetaSpin      = angleSpin(this);  dthetaSpin->setRange(0.0, 180.0);   dthetaSpin->setValue(dtheta_deg);
	thetaModelCombo = makeCombo({"uniform", "cosine", "gaussian"}, this);
	thetaModelCombo->setCurrentText(thetaModel);
	phiSpin  = angleSpin(this);  phiSpin->setRange(0.0, 360.0);  phiSpin->setValue(phi_deg);
	dphiSpin = angleSpin(this);  dphiSpin->setRange(0.0, 180.0); dphiSpin->setValue(dphi_deg);

	// Theta group: spin/slider/model stacked on the left, coverage circle on the right.
	auto* thetaSlider  = new QSlider(Qt::Horizontal, this);
	thetaSlider->setRange(-180, 180);
	thetaSlider->setValue(qRound(theta_deg));
	auto* dthetaSlider = new QSlider(Qt::Horizontal, this);
	dthetaSlider->setRange(0, 180);
	dthetaSlider->setValue(qRound(dtheta_deg));

	auto* thetaLeft = new QGridLayout;
	thetaLeft->setColumnStretch(1, 1);
	thetaLeft->addWidget(new QLabel("\316\270 (deg):"),  0, 0, Qt::AlignRight | Qt::AlignVCenter);
	thetaLeft->addWidget(thetaSpin,                       0, 1);
	thetaLeft->addWidget(thetaSlider,                     1, 1);
	thetaLeft->addWidget(new QLabel("\302\261"),           2, 0, Qt::AlignRight | Qt::AlignVCenter);
	thetaLeft->addWidget(dthetaSpin,                      2, 1);
	thetaLeft->addWidget(dthetaSlider,                    3, 1);
	thetaLeft->addWidget(new QLabel("\316\270 model:"),  4, 0, Qt::AlignRight | Qt::AlignVCenter);
	thetaLeft->addWidget(thetaModelCombo,                 4, 1);

	auto* thetaCoverage = new AngleCoverageWidget(this);
	thetaCoverage->setCenter(theta_deg);
	thetaCoverage->setDelta(dtheta_deg);

	auto* thetaRow = new QHBoxLayout;
	thetaRow->addLayout(thetaLeft, 1);
	thetaRow->addSpacing(8);
	thetaRow->addWidget(thetaCoverage, 0, Qt::AlignVCenter);

	// Phi group: same structure, no model combo.
	auto* phiSlider  = new QSlider(Qt::Horizontal, this);
	phiSlider->setRange(0, 360);
	phiSlider->setValue(qRound(phi_deg));
	auto* dphiSlider = new QSlider(Qt::Horizontal, this);
	dphiSlider->setRange(0, 180);
	dphiSlider->setValue(qRound(dphi_deg));

	auto* phiLeft = new QGridLayout;
	phiLeft->setColumnStretch(1, 1);
	phiLeft->addWidget(new QLabel("\317\206 (deg):"),  0, 0, Qt::AlignRight | Qt::AlignVCenter);
	phiLeft->addWidget(phiSpin,                         0, 1);
	phiLeft->addWidget(phiSlider,                       1, 1);
	phiLeft->addWidget(new QLabel("\302\261"),           2, 0, Qt::AlignRight | Qt::AlignVCenter);
	phiLeft->addWidget(dphiSpin,                        2, 1);
	phiLeft->addWidget(dphiSlider,                      3, 1);

	auto* phiCoverage = new AngleCoverageWidget(this);
	phiCoverage->setCenter(phi_deg);
	phiCoverage->setDelta(dphi_deg);

	auto* phiRow = new QHBoxLayout;
	phiRow->addLayout(phiLeft, 1);
	phiRow->addSpacing(8);
	phiRow->addWidget(phiCoverage, 0, Qt::AlignVCenter);

	auto* angleSep = new QFrame(this);
	angleSep->setFrameShape(QFrame::HLine);
	angleSep->setFrameShadow(QFrame::Sunken);

	auto* anglesBox = new QGroupBox("Angles");
	auto* anglesLayout = new QVBoxLayout(anglesBox);
	anglesLayout->addLayout(thetaRow);
	anglesLayout->addWidget(angleSep);
	anglesLayout->addLayout(phiRow);

	// ---- Vertex ----
	vxSpin  = vertexSpin(this);  vxSpin->setValue(vx_cm);
	vySpin  = vertexSpin(this);  vySpin->setValue(vy_cm);
	vzSpin  = vertexSpin(this);  vzSpin->setValue(vz_cm);
	dvxSpin = vertexSpin(this);  dvxSpin->setValue(dvx_cm);
	dvySpin = vertexSpin(this);  dvySpin->setValue(dvy_cm);
	dvzSpin = vertexSpin(this);  dvzSpin->setValue(dvz_cm);
	vertexModelCombo = makeCombo({"uniform", "gaussian", "sphere"}, this);
	vertexModelCombo->setCurrentText(vertexModel);

	auto* vertexGrid = new QGridLayout;
	vertexGrid->setColumnStretch(1, 1);
	vertexGrid->setColumnStretch(3, 1);

	vertexGrid->addWidget(new QLabel("vx (cm):"), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	vertexGrid->addWidget(vxSpin,                  0, 1);
	vertexGrid->addWidget(new QLabel("\302\261"),   0, 2, Qt::AlignCenter);
	vertexGrid->addWidget(dvxSpin,                 0, 3);
	vertexGrid->addWidget(new QLabel("model:"),    0, 4, Qt::AlignRight | Qt::AlignVCenter);
	vertexGrid->addWidget(vertexModelCombo,         0, 5);

	vertexGrid->addWidget(new QLabel("vy (cm):"), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	vertexGrid->addWidget(vySpin,                  1, 1);
	vertexGrid->addWidget(new QLabel("\302\261"),   1, 2, Qt::AlignCenter);
	vertexGrid->addWidget(dvySpin,                 1, 3);

	vertexGrid->addWidget(new QLabel("vz (cm):"), 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	vertexGrid->addWidget(vzSpin,                  2, 1);
	vertexGrid->addWidget(new QLabel("\302\261"),   2, 2, Qt::AlignCenter);
	vertexGrid->addWidget(dvzSpin,                 2, 3);

	auto* vertexBox = new QGroupBox("Vertex");
	auto* vertexLayout = new QVBoxLayout(vertexBox);
	vertexLayout->addLayout(vertexGrid);

	// ---- Assemble inside a scroll area ----
	auto* inner = new QWidget;
	auto* innerLayout = new QVBoxLayout(inner);
	innerLayout->setSpacing(8);
	innerLayout->addWidget(identityBox);
	innerLayout->addWidget(momBox);
	innerLayout->addWidget(anglesBox);
	innerLayout->addWidget(vertexBox);
	innerLayout->addStretch();

	auto* scroll = new QScrollArea(this);
	scroll->setWidget(inner);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);

	auto* outerLayout = new QVBoxLayout(this);
	outerLayout->setContentsMargins(0, 0, 0, 0);
	outerLayout->addWidget(scroll);

	// Wire every widget to keep gp_ in sync.
	connect(multSpin,         QOverload<int>::of(&QSpinBox::valueChanged),
	        this, [this](int v)           { if (gp_) gp_->setMultiplicity(v); });
	connect(pSpin,            &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setMomentum(v); });
	connect(dpSpin,           &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaMomentum(v); });
	connect(pModelCombo,      &QComboBox::currentTextChanged,
	        this, [this](const QString& s){ if (gp_) gp_->setMomentumModel(s.toStdString()); });
	connect(thetaSpin,        &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setTheta(v * CLHEP::degree); });
	connect(dthetaSpin,       &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaTheta(v * CLHEP::degree); });
	connect(thetaModelCombo,  &QComboBox::currentTextChanged,
	        this, [this](const QString& s){ if (gp_) gp_->setThetaModel(s.toStdString()); });
	connect(phiSpin,          &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setPhi(v * CLHEP::degree); });
	connect(dphiSpin,         &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaPhi(v * CLHEP::degree); });
	connect(vxSpin,           &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setVertexX(v * CLHEP::cm); });
	connect(vySpin,           &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setVertexY(v * CLHEP::cm); });
	connect(vzSpin,           &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setVertexZ(v * CLHEP::cm); });
	connect(dvxSpin,          &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaVertexX(v * CLHEP::cm); });
	connect(dvySpin,          &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaVertexY(v * CLHEP::cm); });
	connect(dvzSpin,          &QDoubleSpinBox::valueChanged,
	        this, [this](double v)        { if (gp_) gp_->setDeltaVertexZ(v * CLHEP::cm); });
	connect(vertexModelCombo, &QComboBox::currentTextChanged,
	        this, [this](const QString& s){ if (gp_) gp_->setVertexModel(s.toStdString()); });

	// Angle slider ↔ spinbox bidirectional links
	connect(thetaSpin,    &QDoubleSpinBox::valueChanged,
	        thetaSlider,  [thetaSlider](double v){ thetaSlider->setValue(qRound(v)); });
	connect(thetaSlider,  &QSlider::valueChanged,
	        this, [this](int v){ thetaSpin->setValue(static_cast<double>(v)); });
	connect(dthetaSpin,   &QDoubleSpinBox::valueChanged,
	        dthetaSlider, [dthetaSlider](double v){ dthetaSlider->setValue(qRound(v)); });
	connect(dthetaSlider, &QSlider::valueChanged,
	        this, [this](int v){ dthetaSpin->setValue(static_cast<double>(v)); });
	connect(phiSpin,      &QDoubleSpinBox::valueChanged,
	        phiSlider,    [phiSlider](double v){ phiSlider->setValue(qRound(v)); });
	connect(phiSlider,    &QSlider::valueChanged,
	        this, [this](int v){ phiSpin->setValue(static_cast<double>(v)); });
	connect(dphiSpin,     &QDoubleSpinBox::valueChanged,
	        dphiSlider,   [dphiSlider](double v){ dphiSlider->setValue(qRound(v)); });
	connect(dphiSlider,   &QSlider::valueChanged,
	        this, [this](int v){ dphiSpin->setValue(static_cast<double>(v)); });

	// Coverage widgets repaint when the corresponding spinboxes change
	connect(thetaSpin,  &QDoubleSpinBox::valueChanged,
	        this, [thetaCoverage](double v){ thetaCoverage->setCenter(v); });
	connect(dthetaSpin, &QDoubleSpinBox::valueChanged,
	        this, [thetaCoverage](double v){ thetaCoverage->setDelta(v); });
	connect(phiSpin,    &QDoubleSpinBox::valueChanged,
	        this, [phiCoverage](double v){ phiCoverage->setCenter(v); });
	connect(dphiSpin,   &QDoubleSpinBox::valueChanged,
	        this, [phiCoverage](double v){ phiCoverage->setDelta(v); });
}
