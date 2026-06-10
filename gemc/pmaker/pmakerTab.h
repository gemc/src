#pragma once

// qt
#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>

// gemc
#include <gemc/gparticle/gparticle.h>

/**
 * \brief Form widget showing all configurable parameters for one Gparticle.
 *
 * Constructed either from an existing GparticlePtr (initial YAML values) or
 * with defaults (when the user adds a new particle via the '+' tab).
 * Emits nameChanged() so the parent QTabWidget can keep the tab label in sync.
 */
class PmakerTab : public QWidget {
	Q_OBJECT
public:
	explicit PmakerTab(const GparticlePtr& gp, QWidget* parent = nullptr);
	explicit PmakerTab(QWidget* parent = nullptr);

	/** \brief Current value of the name field, used as the tab label. */
	QString tabLabel() const;

	/** \brief Returns the live GparticlePtr that reflects the current widget values. */
	GparticlePtr currentGParticle() const { return gp_; }

signals:
	void nameChanged(const QString& name);

private slots:
	void onParticleChanged(const QString& name);

private:
	void buildLayout(const std::string& name,
	                 int    mult,
	                 double p_mev,     double dp_mev,     const QString& pModel,
	                 double theta_deg, double dtheta_deg, const QString& thetaModel,
	                 double phi_deg,   double dphi_deg,
	                 double vx_cm,     double vy_cm,     double vz_cm,
	                 double dvx_cm,    double dvy_cm,    double dvz_cm,
	                 const QString& vertexModel);

	QComboBox*      particleCombo    = nullptr;
	QLabel*         multLabel        = nullptr;
	QSpinBox*       multSpin         = nullptr;
	QDoubleSpinBox* pSpin            = nullptr;
	QDoubleSpinBox* dpSpin           = nullptr;
	QComboBox*      pModelCombo      = nullptr;
	QDoubleSpinBox* thetaSpin        = nullptr;
	QDoubleSpinBox* dthetaSpin       = nullptr;
	QComboBox*      thetaModelCombo  = nullptr;
	QDoubleSpinBox* phiSpin          = nullptr;
	QDoubleSpinBox* dphiSpin         = nullptr;
	QDoubleSpinBox* vxSpin           = nullptr;
	QDoubleSpinBox* vySpin           = nullptr;
	QDoubleSpinBox* vzSpin           = nullptr;
	QDoubleSpinBox* dvxSpin          = nullptr;
	QDoubleSpinBox* dvySpin          = nullptr;
	QDoubleSpinBox* dvzSpin          = nullptr;
	QComboBox*      vertexModelCombo = nullptr;

	GparticlePtr    gp_;
};
