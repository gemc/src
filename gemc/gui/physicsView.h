#pragma once

#include <gemc/goptions/goptions.h>
#include <QWidget>
#include <map>
#include <memory>

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QTabWidget;
class QTreeWidget;

/** Physics configuration and installed-process activation, applied only between runs. */
class PhysicsView : public QWidget {
	Q_OBJECT
public:
	explicit PhysicsView(std::shared_ptr<GOptions> options, QWidget* parent = nullptr);
	bool applyPending();
	void refreshProcesses();

private:
	void addConstructor(const QString& name = {});
	QString selection() const;
	void markPending();

	std::shared_ptr<GOptions> options;
	QComboBox* reference = nullptr;
	QDoubleSpinBox* cut = nullptr;
	QTabWidget* additions = nullptr;
	QTreeWidget* processTree = nullptr;
	QLabel* status = nullptr;
	std::map<std::string, bool> pendingActivation;
	bool pending = false;
	bool refreshing = false;
};
