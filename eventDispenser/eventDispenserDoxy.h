/**
 *
 * \mainpage Event Dispenser Module
 *
 * \section intro_sec Introduction
 * The Event Dispenser module is responsible for distributing simulation events
 * among different runs according to user–defined configurations and weights.
 * It manages a collection of GDynamicDigitization plugins and assigns events based
 * on a weighting scheme.
 *
 * \section details_sec Details
 * The EventDispenser uses parameters from a GOptions object and a global map of
 * GDynamicDigitization plugins to determine the number of events per run. It maintains
 * internal maps for run weights and event counts as well as a list of runs to process.
 *
 * \section usage_sec Usage
 * An instance of EventDispenser is created by supplying a pointer to GOptions and a pointer
 * to a global map of dynamic digitization plugins. The public API provides methods to retrieve
 * run event information, the current run number, the total number of events, and to process
 * events.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

