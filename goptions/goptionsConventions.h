#ifndef  GOPTIONS_CONVENTIONS_H
#define  GOPTIONS_CONVENTIONS_H  1

#define HELPFILLSPACE "                               "

#define GVERSION_STRING "version"

// exit codes: 100s
#define EC__NOOPTIONFOUND                  101
#define EC__DEFINED_OPTION_ALREADY_PRESENT 102
#define EC__DEFINED_SWITCHALREADYPRESENT   103
#define EC__YAML_PARSING_ERROR             104
#define EC__MANDATORY_NOT_FILLED           105
#define EC__BAD_CONVERSION                 106
#define EC_WRONG_VERBOSITY_LEVEL           107


#include <string>
namespace goptions {
    const std::string NODFLT = "NODFLT";
}


#endif
