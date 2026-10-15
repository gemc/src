#include "glogger.h"

GLogger::~GLogger() { debug(DESTRUCTOR, "%1 logger", logger_name); }
