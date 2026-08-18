#pragma once

/**
 * \file gutsConventions.h
 * \brief Typed sentinel, error, and console-formatting constants shared by GEMC modules.
 */

namespace guts {

/** Internal numeric marker retained for serialized and external interfaces that require a sentinel. */
inline constexpr int UNINITIALIZEDNUMBERQUANTITY = -123456;

/** Serialized marker retained for formats and option schemas that cannot express typed absence. */
inline constexpr char UNINITIALIZEDSTRINGQUANTITY[] = "NULL";

/** Process exit code used when an expected file cannot be opened or found. */
inline constexpr int EC__FILENOTFOUND = 301;

/** Process exit code used when parsing a Geant4-style numeric string fails. */
inline constexpr int EC__G4NUMBERERROR = 302;

// ANSI terminal attributes.
inline constexpr char KBOLD[] = "\x1B[1m";
inline constexpr char KRED[] = "\x1B[31m";
inline constexpr char KGRN[] = "\x1B[32m";
inline constexpr char KYEL[] = "\x1B[33m";
inline constexpr char KBLU[] = "\x1B[34m";
inline constexpr char KMAG[] = "\x1B[35m";
inline constexpr char KCYN[] = "\x1B[36m";
inline constexpr char KWHT[] = "\x1B[37m";
inline constexpr char RST[] = "\x1B[0m";

// Standard log labels. The duplicated reset preserves the prior nested-macro byte sequence.
inline constexpr char FATALERRORL[] = "\x1B[1m\x1B[31mFatal Error: \x1B[0m\x1B[0m";
inline constexpr char GWARNING[] = "\x1B[1m\x1B[33mWarning: \x1B[0m\x1B[0m";

inline constexpr char CONSTRUCTORLOG[] = "↑";
inline constexpr char DESTRUCTORLOG[] = "↓";

// List symbols.
inline constexpr char POINTITEM[] = "•";
inline constexpr char CIRCLEITEM[] = "◦";
inline constexpr char SQUAREITEM[] = "◻︎";
inline constexpr char ARROWITEM[] = "➤";

inline constexpr char GREENPOINTITEM[] = "\x1B[1m\x1B[32m•\x1B[0m\x1B[0m";
inline constexpr char GREENCIRCLEITEM[] = "\x1B[1m\x1B[32m◦\x1B[0m\x1B[0m";
inline constexpr char GREENSQUAREITEM[] = "\x1B[1m\x1B[32m◻︎\x1B[0m\x1B[0m";
inline constexpr char GREENARROWITEM[] = "\x1B[1m\x1B[32m➤\x1B[0m\x1B[0m";
inline constexpr char REDPOINTITEM[] = "\x1B[1m\x1B[31m•\x1B[0m\x1B[0m";
inline constexpr char REDCIRCLEITEM[] = "\x1B[1m\x1B[31m◦\x1B[0m\x1B[0m";
inline constexpr char REDSQUAREITEM[] = "\x1B[1m\x1B[31m◻︎\x1B[0m\x1B[0m";
inline constexpr char REDARROWITEM[] = "\x1B[1m\x1B[31m➤\x1B[0m\x1B[0m";

// Indentation strings.
inline constexpr char GTAB[] = "   ";
inline constexpr char GTABTAB[] = "      ";
inline constexpr char GTABTABTAB[] = "         ";
inline constexpr char GTABX4[] = "            ";
inline constexpr char GTABX5[] = "               ";
inline constexpr char GTABX6[] = "                  ";
inline constexpr char GTABX7[] = "                     ";
inline constexpr char GTABX8[] = "                        ";

inline constexpr char TPOINTITEM[] = "   •";
inline constexpr char TCIRCLEITEM[] = "   ◦";
inline constexpr char TSQUAREITEM[] = "   ◻︎";
inline constexpr char TARROWITEM[] = "   ➤";
inline constexpr char TGREENPOINTITEM[] = "   \x1B[1m\x1B[32m•\x1B[0m\x1B[0m";
inline constexpr char TGREENCIRCLEITEM[] = "   \x1B[1m\x1B[32m◦\x1B[0m\x1B[0m";
inline constexpr char TGREENSQUAREITEM[] = "   \x1B[1m\x1B[32m◻︎\x1B[0m\x1B[0m";
inline constexpr char TGREENARROWITEM[] = "   \x1B[1m\x1B[32m➤\x1B[0m\x1B[0m";
inline constexpr char TREDPOINTITEM[] = "   \x1B[1m\x1B[31m•\x1B[0m\x1B[0m";
inline constexpr char TREDCIRCLEITEM[] = "   \x1B[1m\x1B[31m◦\x1B[0m\x1B[0m";
inline constexpr char TREDSQUAREITEM[] = "   \x1B[1m\x1B[31m◻︎\x1B[0m\x1B[0m";
inline constexpr char TREDARROWITEM[] = "   \x1B[1m\x1B[31m➤\x1B[0m\x1B[0m";

inline constexpr char TTPOINTITEM[] = "      •";
inline constexpr char TTCIRCLEITEM[] = "      ◦";
inline constexpr char TTSQUAREITEM[] = "      ◻︎";
inline constexpr char TTARROWITEM[] = "      ➤";
inline constexpr char TTGREENPOINTITEM[] = "      \x1B[1m\x1B[32m•\x1B[0m\x1B[0m";
inline constexpr char TTGREENCIRCLEITEM[] = "      \x1B[1m\x1B[32m◦\x1B[0m\x1B[0m";
inline constexpr char TTGREENSQUAREITEM[] = "      \x1B[1m\x1B[32m◻︎\x1B[0m\x1B[0m";
inline constexpr char TTGREENARROWITEM[] = "      \x1B[1m\x1B[32m➤\x1B[0m\x1B[0m";
inline constexpr char TTREDPOINTITEM[] = "      \x1B[1m\x1B[31m•\x1B[0m\x1B[0m";
inline constexpr char TTREDCIRCLEITEM[] = "      \x1B[1m\x1B[31m◦\x1B[0m\x1B[0m";
inline constexpr char TTREDSQUAREITEM[] = "      \x1B[1m\x1B[31m◻︎\x1B[0m\x1B[0m";
inline constexpr char TTREDARROWITEM[] = "      \x1B[1m\x1B[31m➤\x1B[0m\x1B[0m";

// Highlight symbols.
inline constexpr char HHL[] = "⏵";
inline constexpr char HHR[] = "⏴";
inline constexpr char RSTHHR[] = "⏴\x1B[0m";
inline constexpr char GREENHHL[] = "\x1B[32m⏵";
inline constexpr char REDHHL[] = "\x1B[31m⏵";
inline constexpr char YELLOWHHL[] = "\x1B[33m⏵";
inline constexpr char BOLDWHHL[] = "\x1B[1m⏵\x1B[0m";

} // namespace guts
