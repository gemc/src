#pragma once

#include <string>

/**
 * @file gutsConventions.h
 * @brief Common constants and console-formatting macros used across gutilities and related code.
 *
 * This header centralizes:
 * - Sentinel values for "uninitialized" numeric and string quantities.
 * - Exit codes used by error handling paths.
 * - ANSI escape sequences and helper macros for colored/bold/underlined console output.
 * - Standardized glyphs used for list items and log decorations.
 *
 * The intent is to keep message formatting consistent across the codebase.
 */

/**
 * @def UNINITIALIZEDNUMBERQUANTITY
 * @brief Sentinel value representing an uninitialized numeric quantity.
 *
 * This constant is used in contexts where a numeric parameter is optional or not yet set
 * and a distinct "impossible" value is needed for detection.
 *
 * @warning Do not rely on this specific numeric value in persisted outputs; treat it as an internal sentinel.
 */
#define UNINITIALIZEDNUMBERQUANTITY -123456

/**
 * @def UNINITIALIZEDSTRINGQUANTITY
 * @brief Sentinel string representing an uninitialized string quantity.
 *
 * This constant is used as a conventional marker for unset or missing strings in configuration-like flows.
 *
 * @note Some utilities also treat YAML-style null spellings (e.g., @c "null", @c "~") as equivalent.
 */
#define UNINITIALIZEDSTRINGQUANTITY "NULL"

// Exit codes for various error scenarios.

/**
 * @def EC__FILENOTFOUND
 * @brief Process exit code used when an expected file cannot be opened or found.
 */
#define EC__FILENOTFOUND 301    ///< File not found error code.

/**
 * @def EC__G4NUMBERERROR
 * @brief Process exit code used when parsing a Geant4-style numeric string fails.
 */
#define EC__G4NUMBERERROR 302   ///< G4 number error code.

// ANSI color codes for terminal text formatting.
// These can be used to print colored text to the console.

/**
 * @def KBOLD
 * @brief ANSI escape sequence for bold text.
 */
#define KBOLD "\x1B[1m"

/**
 * @def KRED
 * @brief ANSI escape sequence for red foreground text.
 */
#define KRED "\x1B[31m"

/**
 * @def KGRN
 * @brief ANSI escape sequence for green foreground text.
 */
#define KGRN "\x1B[32m"

/**
 * @def KYEL
 * @brief ANSI escape sequence for yellow foreground text.
 */
#define KYEL "\x1B[33m"

/**
 * @def KBLU
 * @brief ANSI escape sequence for blue foreground text.
 */
#define KBLU "\x1B[34m"

/**
 * @def KMAG
 * @brief ANSI escape sequence for magenta foreground text.
 */
#define KMAG "\x1B[35m"

/**
 * @def KCYN
 * @brief ANSI escape sequence for cyan foreground text.
 */
#define KCYN "\x1B[36m"

/**
 * @def KWHT
 * @brief ANSI escape sequence for white foreground text.
 */
#define KWHT "\x1B[37m"

/**
 * @def RST
 * @brief ANSI escape sequence to reset text formatting to defaults.
 */
#define RST "\x1B[0m"

// Macros for colored text formatting with reset

/**
 * @def FRED(x)
 * @brief Wrap @p x in red text and reset formatting afterwards.
 */
#define FRED(x) KRED x RST  ///< Macro to wrap text in red color.

/**
 * @def FGRN(x)
 * @brief Wrap @p x in green text and reset formatting afterwards.
 */
#define FGRN(x) KGRN x RST  ///< Macro to wrap text in green color.

/**
 * @def FYEL(x)
 * @brief Wrap @p x in yellow text and reset formatting afterwards.
 */
#define FYEL(x) KYEL x RST  ///< Macro to wrap text in yellow color.

/**
 * @def FBLU(x)
 * @brief Wrap @p x in blue text and reset formatting afterwards.
 */
#define FBLU(x) KBLU x RST  ///< Macro to wrap text in blue color.

/**
 * @def FMAG(x)
 * @brief Wrap @p x in magenta text and reset formatting afterwards.
 */
#define FMAG(x) KMAG x RST  ///< Macro to wrap text in magenta color.

/**
 * @def FCYN(x)
 * @brief Wrap @p x in cyan text and reset formatting afterwards.
 */
#define FCYN(x) KCYN x RST  ///< Macro to wrap text in cyan color.

/**
 * @def FWHT(x)
 * @brief Wrap @p x in white text and reset formatting afterwards.
 */
#define FWHT(x) KWHT x RST  ///< Macro to wrap text in white color.

/**
 * @def BOLD(x)
 * @brief Wrap @p x in bold formatting and reset afterwards.
 */
#define BOLD(x) KBOLD x RST ///< Macro to wrap text in bold.

/**
 * @def UNDL(x)
 * @brief Wrap @p x in underline formatting and reset afterwards.
 */
#define UNDL(x) "\x1B[4m" x RST ///< Macro to wrap text with underline.

/**
 * @def FATALERRORL
 * @brief Standardized fatal error label prefix (bold red).
 *
 * Typically used at the start of an error message sent to @c std::cerr.
 */
#define FATALERRORL BOLD(FRED("Fatal Error: ")) ///< Fatal error label.

/**
 * @def GWARNING
 * @brief Standardized warning label prefix (bold yellow).
 *
 * Typically used at the start of a warning message sent to @c std::cerr.
 */
#define GWARNING BOLD(FYEL("Warning: "))  ///< Warning label.

/**
 * @def CONSTRUCTORLOG
 * @brief Glyph used to annotate constructor log messages.
 */
#define CONSTRUCTORLOG "↑"  ///< Log symbol for constructor.

/**
 * @def DESTRUCTORLOG
 * @brief Glyph used to annotate destructor log messages.
 */
#define DESTRUCTORLOG "↓"   ///< Log symbol for destructor.

// List item symbols

/**
 * @def POINTITEM
 * @brief Bullet glyph used for list formatting in console logs.
 */
#define POINTITEM "•"          ///< Symbol for point item.

/**
 * @def CIRCLEITEM
 * @brief Hollow bullet glyph used for list formatting in console logs.
 */
#define CIRCLEITEM "◦"         ///< Symbol for circle item.

/**
 * @def SQUAREITEM
 * @brief Square glyph used for list formatting in console logs.
 */
#define SQUAREITEM "◻︎"        ///< Symbol for square item.

/**
 * @def ARROWITEM
 * @brief Arrow glyph used for list formatting in console logs.
 */
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

/**
 * @def GTAB
 * @brief Standard indentation unit used by console-formatting macros.
 */
#define GTAB "   "
#define GTABTAB GTAB GTAB
#define GTABTABTAB GTAB GTAB GTAB
#define GTABX4 GTABTABTAB GTAB
#define GTABX5 GTABX4 GTAB
#define GTABX6 GTABX5 GTAB
#define GTABX7 GTABX6 GTAB
#define GTABX8 GTABX7 GTAB

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

/**
 * @def HHL
 * @brief Left highlight glyph.
 */
#define HHL "⏵"                ///< Highlight left symbol.

/**
 * @def HHR
 * @brief Right highlight glyph.
 */
#define HHR "⏴"                ///< Highlight right symbol.

#define RSTHHR HHR RST         ///< Reset highlight right.
#define GREENHHL KGRN HHL      ///< Green highlight left.
#define REDHHL KRED HHL        ///< Red highlight left.
#define YELLOWHHL KYEL HHL     ///< Yellow highlight left.
#define BOLDWHHL BOLD(HHL)     ///< Bold white highlight left.
