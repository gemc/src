#include "dbselectView.h"

using namespace std;

DBSelectView::DBSelectView(GOptions *gopts, QWidget *parent) : QWidget(parent) {

	string database = "notAvailable";

	if(gopts->doesOptionExist("gsystem")) {
		cout << "gsystem option found" << endl;
	} else {
		cout << "gsystem option not found, using $GEMC/gemc.db" << endl;
		database = std::string(getenv("GEMC")) + "/gemc.db";
	}





}