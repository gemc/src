#pragma once

/**
 * \def TEXTPROGRESSBARNSTEPS
 * \brief Number of discrete progress bar steps used for throttled printing.
 *
 * This constant defines the maximum number of printed updates that may occur across the full
 * configured progress range. Larger values can produce smoother visual updates but may increase
 * console output in tight loops.
 */
#define TEXTPROGRESSBARNSTEPS 100
