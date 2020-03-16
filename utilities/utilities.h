#ifndef GUTILITIES_H
#define GUTILITIES_H 1

// GOptions
// --------





// a simple struct to model the detector option
namespace goptions {

	// single option to activate gui
	// -----------------------------

	struct GConf {
		int gui;
		int nthreads;
        int stageMessageVerbosity;
	};

	void from_json(const json& j, GConf& c) {
		j.at("gui").get_to(c.gui);
		j.at("nthreads").get_to(c.nthreads);
        j.at("stageMessageVerbosity").get_to(c.stageMessageVerbosity);
	}

	// non groupable: method to return a single gui
	GConf getGConf(GOptions *gopts) {
        auto jConf = (*gopts)["gConf"].front();
		return jConf.get<GConf>();
	}

	bool getGui(GOptions *gopts) {
		auto jConf = (*gopts)["gConf"].front();

		auto gConf = jConf.get<GConf>();
		return gConf.gui == 1;
	}

    bool getSMV(GOptions *gopts) {
        auto jConf = (*gopts)["gConf"].front();

        auto gConf = jConf.get<GConf>();
        return gConf.stageMessageVerbosity;
}

}




// returns array of options definitions
vector<GOption> defineOptions();



	
// qt
#include <QApplication>

// distinguishing between graphical and batch mode
QCoreApplication* createQtApplication(int &argc, char *argv[], int gui);

#endif
