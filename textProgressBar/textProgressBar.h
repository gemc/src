#pragma once

#include "textProgressBarConventions.h"

// c++
#include <string>
using std::string;

/**
 * \class TextProgressBar
 * \brief Renders an ASCII progress bar to standard output.
 *
 * TextProgressBar prints a single-line progress indicator (title + bar + percentage) to the console.
 * Updates overwrite the same line via carriage return (`\r`). When the progress reaches the final
 * step, the progress bar ends with a newline.
 *
 * \note Output is produced to \c std::cout by the implementation.
 *
 * \par Typical usage
 * \code
 * TextProgressBar bar(30, "  processing: ", 0, 100);
 * for (int i = 0; i <= 100; i++) {
 *   bar.setProgress(i);
 * }
 * \endcode
 *
 * \par Update throttling
 * The progress bar is intentionally throttled: it only prints when the input value advances by at
 * least one internal step. The internal step size is derived from the configured [min, max] range
 * and the module constant TEXTPROGRESSBARNSTEPS.
 */
class TextProgressBar
{
public:
	/**
	 * \brief Constructs a progress bar with a value range and visual configuration.
	 *
	 * The progress bar represents values in the inclusive range [minimum, maximum]. The output is
	 * throttled to at most TEXTPROGRESSBARNSTEPS printed updates across the full range.
	 *
	 * @param w        The visual width of the bar (number of character cells between the delimiters).
	 * @param t        The title/prefix printed before the bar (e.g. `"  processing: "`).
	 * @param minimum  The minimum progress value (start of the range).
	 * @param maximum  The maximum progress value (end of the range).
	 * @param s        The character used to open the bar (default: `'['`).
	 * @param e        The character used to close the bar (default: `']'`).
	 * @param m        The character used to fill completed portions of the bar (default: `'='`).
	 * @param a        The character used to indicate the current advancing position (default: `'>'`).
	 *
	 * \note If the range is too small to compute a meaningful step size, updates may be suppressed
	 * (see TextProgressBar::setProgress "setProgress()").
	 */
	TextProgressBar(int  w, string t, int minimum = 0, int maximum = 100, char s = '[', char e = ']', char m = '=',
	                char a                        = '>') :
		barWidth(w), title(t), min(minimum), max(maximum), startBarChar(s), endBarChar(e), middleBarChar(m),
		advanceBarChar(a) {
		// Determine the internal update granularity so we print at most TEXTPROGRESSBARNSTEPS updates.
		// If the range is too small, "singleStep" becomes 0 and updates will be suppressed.
		if (maximum - minimum > TEXTPROGRESSBARNSTEPS) {
			singleStep = (maximum - minimum) / TEXTPROGRESSBARNSTEPS;
		}
		else {
			singleStep = 0;
		}

		// The next progress threshold at which we will print an update.
		indexStep = minimum;
	}

	/**
	 * \brief Updates the progress bar based on the provided progress value.
	 *
	 * The progress bar prints only when the provided value reaches or exceeds the next internal
	 * threshold (tracked by \c indexStep). This prevents excessive console output in tight loops.
	 *
	 * Updates are ignored in the following cases:
	 * - The provided value is less than the next threshold (\c p < indexStep).
	 * - The bar's internal step size is zero (\c singleStep == 0), meaning the range is too small
	 *   for throttled printing.
	 * - The provided value is outside the configured range (\c p < min or \c p > max).
	 *
	 * @param p Current progress value (typically a loop index or processed item count).
	 */
	void setProgress(int p);

private:
	/// Visual width (number of characters between start and end delimiters).
	int barWidth;

	/// Title/prefix printed before the bar (e.g. `"  processing: "`).
	string title;

	/// Inclusive minimum and maximum values that define the progress range.
	int min, max;

	/// Visual characters used to render the bar.
	char startBarChar, endBarChar;

	/// Fill character for completed area and the "advancing" marker at current position.
	char middleBarChar, advanceBarChar;

	/// Step size (in progress units) between printed updates.
	int singleStep;

	/// Next progress value threshold at which an update will be printed.
	int indexStep;
};
