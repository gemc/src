#pragma once

#include <string>

/**
 * @brief Represents an uninitialized numeric quantity.
 */
#define UNINITIALIZEDNUMBERQUANTITY -123456

/**
 * @brief Represents an uninitialized string quantity.
 */
#define UNINITIALIZEDSTRINGQUANTITY "na"

// Exit codes for various error scenarios.
#define EC__FILENOTFOUND 301    ///< File not found error code.
#define EC__G4NUMBERERROR 302   ///< G4 number error code.

// ANSI color codes for terminal text formatting.
// These can be used to print colored text to the console.

/**
 * @brief ANSI code for bold text.
 */
#define KBOLD "\x1B[1m"

/**
 * @brief ANSI code for red colored text.
 */
#define KRED "\x1B[31m"

/**
 * @brief ANSI code for green colored text.
 */
#define KGRN "\x1B[32m"

/**
 * @brief ANSI code for yellow colored text.
 */
#define KYEL "\x1B[33m"

/**
 * @brief ANSI code for blue colored text.
 */
#define KBLU "\x1B[34m"

/**
 * @brief ANSI code for magenta colored text.
 */
#define KMAG "\x1B[35m"

/**
 * @brief ANSI code for cyan colored text.
 */
#define KCYN "\x1B[36m"

/**
 * @brief ANSI code for white colored text.
 */
#define KWHT "\x1B[37m"

/**
 * @brief ANSI code to reset text formatting.
 */
#define RST "\x1B[0m"

// Macros for colored text formatting with reset
#define FRED(x) KRED x RST  ///< Macro to wrap text in red color.
#define FGRN(x) KGRN x RST  ///< Macro to wrap text in green color.
#define FYEL(x) KYEL x RST  ///< Macro to wrap text in yellow color.
#define FBLU(x) KBLU x RST  ///< Macro to wrap text in blue color.
#define FMAG(x) KMAG x RST  ///< Macro to wrap text in magenta color.
#define FCYN(x) KCYN x RST  ///< Macro to wrap text in cyan color.
#define FWHT(x) KWHT x RST  ///< Macro to wrap text in white color.

#define BOLD(x) KBOLD x RST ///< Macro to wrap text in bold.
#define UNDL(x) "\x1B[4m" x RST ///< Macro to wrap text with underline.

#define FATALERRORL BOLD(FRED("Fatal Error: ")) ///< Fatal error label.
#define GWARNING BOLD(FYEL("Warning: "))  ///< Warning label.

#define CONSTRUCTORLOG "↑"  ///< Log symbol for constructor.
#define DESTRUCTORLOG "↓"   ///< Log symbol for destructor.

// List item symbols
#define POINTITEM "•"          ///< Symbol for point item.
#define CIRCLEITEM "◦"         ///< Symbol for circle item.
#define SQUAREITEM "◻︎"        ///< Symbol for square item.
#define ARROWITEM "➤"          ///< Symbol for arrow item.
#define GREENPOINTITEM BOLD(FGRN(POINTITEM)) ///< Green point item.
#define GREENCIRCLEITEM BOLD(FGRN(CIRCLEITEM)) ///< Green circle item.
#define GREENSQUAREITEM BOLD(FGRN(SQUAREITEM)) ///< Green square item.
#define GREENARROWITEM BOLD(FGRN(ARROWITEM)) ///< Green arrow item.
#define REDPOINTITEM BOLD(FRED(POINTITEM)) ///< Red point item.
#define REDCIRCLEITEM BOLD(FRED(CIRCLEITEM)) ///< Red circle item.
#define REDSQUAREITEM BOLD(FRED(SQUAREITEM)) ///< Red square item.
#define REDARROWITEM BOLD(FRED(ARROWITEM)) ///< Red arrow item.

// Tab spacing for indentation
#define GTAB "   "              ///< One tab spacing.
#define GTABTAB GTAB GTAB       ///< Two tab spacing.
#define GTABTABTAB GTAB GTAB GTAB ///< Three tab spacing.

// Single tab list items
#define TPOINTITEM GTAB POINTITEM       ///< Point item with single tab.
#define TCIRCLEITEM GTAB CIRCLEITEM     ///< Circle item with single tab.
#define TSQUAREITEM GTAB SQUAREITEM     ///< Square item with single tab.
#define TARROWITEM GTAB ARROWITEM       ///< Arrow item with single tab.
#define TGREENPOINTITEM GTAB GREENPOINTITEM  ///< Green point item with single tab.
#define TGREENCIRCLEITEM GTAB GREENCIRCLEITEM ///< Green circle item with single tab.
#define TGREENSQUAREITEM GTAB GREENSQUAREITEM ///< Green square item with single tab.
#define TGREENARROWITEM GTAB GREENARROWITEM  ///< Green arrow item with single tab.
#define TREDPOINTITEM GTAB REDPOINTITEM    ///< Red point item with single tab.
#define TREDCIRCLEITEM GTAB REDCIRCLEITEM  ///< Red circle item with single tab.
#define TREDSQUAREITEM GTAB REDSQUAREITEM  ///< Red square item with single tab.
#define TREDARROWITEM GTAB REDARROWITEM    ///< Red arrow item with single tab.

// Double tab list items
#define TTPOINTITEM GTABTAB POINTITEM       ///< Point item with double tab.
#define TTCIRCLEITEM GTABTAB CIRCLEITEM     ///< Circle item with double tab.
#define TTSQUAREITEM GTABTAB SQUAREITEM     ///< Square item with double tab.
#define TTARROWITEM GTABTAB ARROWITEM       ///< Arrow item with double tab.
#define TTGREENPOINTITEM GTABTAB GREENPOINTITEM  ///< Green point item with double tab.
#define TTGREENCIRCLEITEM GTABTAB GREENCIRCLEITEM ///< Green circle item with double tab.
#define TTGREENSQUAREITEM GTABTAB GREENSQUAREITEM ///< Green square item with double tab.
#define TTGREENARROWITEM GTABTAB GREENARROWITEM  ///< Green arrow item with double tab.
#define TTREDPOINTITEM GTABTAB REDPOINTITEM    ///< Red point item with double tab.
#define TTREDCIRCLEITEM GTABTAB REDCIRCLEITEM  ///< Red circle item with double tab.
#define TTREDSQUAREITEM GTABTAB REDSQUAREITEM  ///< Red square item with double tab.
#define TTREDARROWITEM GTABTAB REDARROWITEM    ///< Red arrow item with double tab.

// Highlight symbols
#define HHL "⏵"                ///< Highlight left symbol.
#define HHR "⏴"                ///< Highlight right symbol.
#define RSTHHR HHR RST         ///< Reset highlight right.
#define GREENHHL KGRN HHL      ///< Green highlight left.
#define REDHHL KRED HHL        ///< Red highlight left.
#define YELLOWHHL KYEL HHL     ///< Yellow highlight left.
#define BOLDWHHL BOLD(HHL)     ///< Bold white highlight left.

