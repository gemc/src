#ifndef  GOPTIONS_CONVENTIONS_H
#define  GOPTIONS_CONVENTIONS_H  1

#define HELPFILLSPACE "                               "

// standardizing verbosity
#define GVERBOSITY_SUMMARY 1 // prints only summary message
#define GVERBOSITY_DETAILS 2 // print all details
#define GVERBOSITY_CLASSES 3 // print all details and class constructors / destructors messages
#define GVERBOSITY_DESCRIPTION "Possible values: 0: shush; 1: summary; 2: details; 3: everything"   //

// exit codes: 100s
#define EC__NOOPTIONFOUND                  101
#define EC__DEFINED_OPTION_ALREADY_PRESENT 102
#define EC__DEFINED_SWITCHALREADYPRESENT   103

#include <string>
namespace goptions {
    const std::string NODFLT = "NODFLT";

}


#endif
