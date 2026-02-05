/**
 * @defgroup gsplash_module GSplash module
 * @brief Splash-screen helper for GUI runs.
 *
 * This module provides the GSplash class, a small utility that shows a splash image and
 * allows overlaying status messages during application startup or long initialization phases.
 *
 * @{
 */

/**
 * @defgroup gsplash_core Core API
 * @ingroup gsplash_module
 * @brief Public classes and functions for the GSplash module.
 *
 * The core group contains the GSplash class and the small options helper in the gsplash namespace.
 * @{
 */

/** @} */ // end of gsplash_core

/**
 * @defgroup gsplash_examples Examples
 * @ingroup gsplash_module
 * @brief End-to-end example programs demonstrating GSplash usage.
 *
 * @{
 */

/** @} */ // end of gsplash_examples

/** @} */ // end of gsplash_module


/**
 * \mainpage
 *
 * \section gsplash_intro Introduction
 *
 * The GSplash module provides a lightweight splash-screen mechanism for GUI-capable runs.
 * When GUI mode is enabled, a splash image is displayed and informational messages can be
 * overlaid as the application starts.
 *
 * The main entry points are:
 * - The factory method \ref GSplash::create "create()" that returns a GSplash instance (or nullptr when GUI is disabled).
 * - The message utilities \ref GSplash::message "message()" and \ref GSplash::messageAfter "messageAfter()".
 *
 *
 * \section gsplash_ownership Ownership and lifecycle
 *
 * A GSplash object is created via \ref GSplash::create "create()" and returned as a
 * std::unique_ptr, which clearly expresses exclusive ownership. When GUI mode is disabled
 * (option \c --gui off), \ref GSplash::create "create()" returns nullptr and the caller
 * can proceed without any GUI dependencies.
 *
 * Internally, the splash widget is managed as a std::unique_ptr to \c QSplashScreen.
 * If image loading fails, the splash screen is not created and the object remains in an
 * inactive state where message calls become no-ops.
 *
 *
 * \section gsplash_arch Architecture
 *
 * \subsection gsplash_design_notes Design notes
 *
 * - Image selection supports both filesystem paths and \c Qt resource lookup:
 *   first a direct load is attempted, then the same name is tried as \c ":/<name>".
 * - If the special image token \c NOSPLASHIMAGESELECTED is used, the image path is read
 *   from the environment variable \c GSPLASH.
 * - Message updates pump the GUI event loop briefly via \c QCoreApplication::processEvents
 *   to keep the splash responsive while the application is still initializing.
 *
 *
 * \section gsplash_options Available Options and their usage
 *
 * This module defines its own options namespace helper:
 * - \c gsplash::defineOptions() returns a GOptions object configured for this module.
 *
 * Typical usage in an application:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gsplash::defineOptions());
 * auto splash = GSplash::create(gopts, "example.png");
 * if (splash) {
 *   splash->message("Initializing...");
 * }
 * \endcode
 *
 * The following runtime switches are expected by this module:
 * - \c --gui : when enabled, GSplash will create and show the splash screen; when disabled it returns nullptr.
 *
 *
 * \section gsplash_verbosity Module verbosity
 *
 * GSplash uses the standard logging infrastructure (classes derived from glogger through the base facilities).
 * The effective verbosity is controlled by the logger configuration associated with \c GSPLASH_LOGGER.
 *
 * - Verbosity level 0 typically prints high-level lifecycle messages (startup, major milestones).
 * - Verbosity level 1 typically prints additional configuration details (selected image source, GUI mode decisions).
 * - Verbosity level 2 typically prints fine-grained progress information.
 * - Debug output prints diagnostic details useful when investigating issues, such as image lookup fallbacks
 *   and event-loop timing behavior.
 *
 *
 * \section gsplash_examples_sec Examples
 *
 * The module includes example code that demonstrates GUI and CLI execution paths.
 *
 * - \ref gsplash_example "gsplash_example" : Minimal example showing GSplash creation, message overlay, and timed shutdown.
 *
 * Example snippet:
 * \code
 * auto gsplash = GSplash::create(gopts, "example.png");
 * gsplash->message("Some text I want to show");
 * gsplash->messageAfter(500, "Some other text I want to show");
 * \endcode
 *
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail : ungaro@jlab.org
 */
