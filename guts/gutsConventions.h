#ifndef  GUTS_CONVENTIONS_H
#define  GUTS_CONVENTIONS_H  1

#define UNINITIALIZEDNUMBERQUANTITY -123456
#define UNINITIALIZEDSTRINGQUANTITY "na"

// exit codes: 300
#define EC__FILENOTFOUND     301
#define EC__G4NUMBERERROR    302

// use of colors in iostream
// example:
// cout << BOLD(FBLU("I'm blue-bold.")) << endl;
// taken from:
// https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal

#define KBOLD "\x1B[1m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RST   "\x1B[0m" // reset string

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#define FATALERRORL BOLD(FRED(" Error: "))
#define GWARNING    BOLD(FYEL(" Warning: "))

#define CONSTRUCTORLOG " ↑ "
#define DESTRUCTORLOG " ↓ "

// list bullets
#define POINTITEM  " • "
#define CIRCLEITEM " ◦ "
#define SQUAREITEM " ◻︎ "
#define ARROWITEM " ➤ "
#define GREENPOINTITEM  BOLD(FGRN(POINTITEM))
#define GREENCIRCLEITEM BOLD(FGRN(CIRCLEITEM))
#define GREENSQUAREITEM BOLD(FGRN(SQUAREITEM))
#define GREENARROWITEM  BOLD(FGRN(ARROWITEM))
#define REDPOINTITEM    BOLD(FRED(POINTITEM))
#define REDCIRCLEITEM   BOLD(FRED(CIRCLEITEM))
#define REDSQUAREITEM   BOLD(FRED(SQUAREITEM))
#define REDARROWITEM    BOLD(FRED(ARROWITEM))

// tab is three spaces
#define GTAB "   "
#define GTABTAB GTAB GTAB
#define GTABTABTAB GTAB GTAB GTAB

// with one GTAB
#define TPOINTITEM       GTAB POINTITEM
#define TCIRCLEITEM      GTAB CIRCLEITEM
#define TSQUAREITEM      GTAB SQUAREITEM
#define TARROWITEM       GTAB ARROWITEM
#define TGREENPOINTITEM  GTAB GREENPOINTITEM
#define TGREENCIRCLEITEM GTAB GREENCIRCLEITEM
#define TGREENSQUAREITEM GTAB GREENSQUAREITEM
#define TGREENARROWITEM  GTAB GREENARROWITEM
#define TREDPOINTITEM    GTAB REDPOINTITEM
#define TREDCIRCLEITEM   GTAB REDCIRCLEITEM
#define TREDSQUAREITEM   GTAB REDSQUAREITEM
#define TREDARROWITEM    GTAB REDARROWITEM

// with two GTABS
#define TTPOINTITEM       GTABTAB POINTITEM
#define TTCIRCLEITEM      GTABTAB CIRCLEITEM
#define TTSQUAREITEM      GTABTAB SQUAREITEM
#define TTARROWITEM       GTABTAB ARROWITEM
#define TTGREENPOINTITEM  GTABTAB GREENPOINTITEM
#define TTGREENCIRCLEITEM GTABTAB GREENCIRCLEITEM
#define TTGREENSQUAREITEM GTABTAB GREENSQUAREITEM
#define TTGREENARROWITEM  GTABTAB GREENARROWITEM
#define TTREDPOINTITEM    GTABTAB REDPOINTITEM
#define TTREDCIRCLEITEM   GTABTAB REDCIRCLEITEM
#define TTREDSQUAREITEM   GTABTAB REDSQUAREITEM
#define TTREDARROWITEM    GTABTAB REDARROWITEM

// item highlight
#define HHL "→"
#define HHR "←"
#define RSTHHR          HHR RST   // rest color after right arrow
#define GREENHHL        KGRN HHL  // notice this will highlight the item green
#define REDHHL          KRED HHL
#define GREENHHL        KGRN HHL  // notice this will highlight the item green
#define YELLOWHHL       KYEL HHL
#define BOLDWHHL        "\x1B[1m" HHL

#include <string>

// thread safe message systems
void gexit(int error);
void gLogClassConstruct(std::string className);
void gLogCopyConstruct(std::string className);
void gLogMoveConstruct(std::string className);
void gLogClassDestruct(std::string className);
void gLogMessage(std::string message);

#endif
