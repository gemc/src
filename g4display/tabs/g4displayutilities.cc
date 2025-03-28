// g4display
#include "g4displayutilities.h"

// c++
#include <string>
using namespace std;


G4DisplayUtilities::G4DisplayUtilities([[maybe_unused]] GOptions* gopt, GLogger * const logger, QWidget* parent) : QWidget(parent), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayUtilities");
}
