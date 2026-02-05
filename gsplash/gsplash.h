#pragma once

// gemc
#include "gbase.h"

// Qt
#include <QtWidgets>
#include <QTimer>

#define GSPLASHENVIRONMENT    "GSPLASH"
#define NOSPLASHIMAGESELECTED "NOSPLASHIMAGESELECTED"

// exit codes: 190s
#define ERR_NOSPLASHENVFOUND  191

/**
 * @brief Default logger name used by this module.
 *
 * This string is passed to the base logging facilities to tag messages produced by GSplash.
 */
constexpr const char* GSPLASH_LOGGER = "gsplash";

/**
 * @ingroup gsplash_core
 * @brief Namespace utilities for the GSplash module.
 */
namespace gsplash {

/**
 * @brief Defines the module options for GSplash.
 *
 * This helper returns a GOptions instance configured for the GSplash module namespace.
 * Applications typically merge/collect options from multiple modules and construct a shared
 * GOptions instance from argc/argv and module definitions.
 *
 * Expected options consumed by GSplash at runtime include:
 * - \c --gui : when enabled, GSplash will create and show a splash screen; otherwise GSplash::create returns nullptr.
 *
 * @return A GOptions definition for the GSplash module.
 */
inline GOptions defineOptions() { return GOptions(GSPLASH_LOGGER); }

} // namespace gsplash


/**
 * @ingroup gsplash_core
 * @brief Splash screen utility for GUI runs.
 *
 * GSplash shows a splash image and supports overlaying messages on top of it.
 * It is designed to be optional: when GUI mode is disabled, \ref GSplash::create "create()"
 * returns nullptr so callers can safely use it in both GUI and headless workflows.
 *
 * Image selection rules:
 * - If @p imageName is not \c NOSPLASHIMAGESELECTED, GSplash tries to load it first as a filesystem path,
 *   then as a \c Qt resource by prefixing \c ":/".
 * - If @p imageName is \c NOSPLASHIMAGESELECTED, GSplash reads the image filename from the environment
 *   variable \c GSPLASH.
 *
 * Message behavior:
 * - \ref GSplash::message "message()" sets the overlay text immediately.
 * - \ref GSplash::messageAfter "messageAfter()" schedules a message update after a delay using \c QTimer.
 * - When no splash is active (e.g. image failed to load), message calls are no-ops.
 */
class GSplash : public GBase<GSplash> {
public:
	/**
	 * @brief Factory method for creating a GSplash instance.
	 *
	 * This method centralizes the “GUI enabled?” policy:
	 * - If @p gopts is null or the \c --gui switch is not enabled, it returns nullptr.
	 * - Otherwise it constructs GSplash and loads the selected image.
	 *
	 * Ownership is returned as a std::unique_ptr to enforce a single owner and avoid ambiguous lifetime.
	 *
	 * @param gopts Shared application options.
	 * @param imageName Splash image name or path. When set to \c NOSPLASHIMAGESELECTED, the image path is taken from
	 *        the environment variable \c GSPLASH. Defaults to \c "gemcArchitecture".
	 * @return A std::unique_ptr to GSplash when GUI is enabled, otherwise nullptr.
	 */
	static std::unique_ptr<GSplash>
	create(const std::shared_ptr<GOptions>& gopts,
	       const std::string&               imageName = "gemcArchitecture");

	/**
	 * @brief Displays a message on the splash screen immediately.
	 *
	 * If the splash is inactive (no valid image was loaded or GUI is disabled), this method does nothing.
	 * Internally, the message is shown via \c QSplashScreen::showMessage and the event loop is processed briefly
	 * to keep the UI responsive during initialization.
	 *
	 * @param msg The message to display.
	 */
	void message(const std::string& msg);

	/**
	 * @brief Displays a message on the splash screen after a delay.
	 *
	 * This schedules an update via \c QTimer::singleShot. A guarded pointer is used so the callback is safe even if
	 * the splash widget is destroyed before the timer fires.
	 *
	 * If the splash is inactive (no valid image was loaded or GUI is disabled), this method does nothing.
	 *
	 * @param delay Delay in milliseconds before displaying @p msg.
	 * @param msg The message to display.
	 */
	void messageAfter(int delay, const std::string& msg);

	/**
	 * @brief Finishes the splash screen and returns focus to the calling window.
	 *
	 * This is typically invoked by applications once initialization is complete and the main UI is ready.
	 * If the splash is inactive, this method does nothing.
	 *
	 * @param callingWindow The window that should receive focus after closing the splash.
	 */
	void finish(QWidget* callingWindow) const { if (splash != nullptr) { splash->finish(callingWindow); } }

private:
	/**
	 * @brief Constructs a GSplash instance.
	 *
	 * Construction is private to enforce the factory pattern via \ref GSplash::create "create()",
	 * which ensures consistent behavior in headless runs.
	 *
	 * @param gopts Shared application options.
	 * @param imageName Splash image name or path. See class documentation for the selection rules.
	 */
	GSplash(const std::shared_ptr<GOptions>& gopts,
	        const std::string&               imageName);

	/**
	 * @brief The underlying \c QSplashScreen widget.
	 *
	 * This pointer remains null when the image cannot be loaded; in that case, message methods are no-ops.
	 */
	std::unique_ptr<QSplashScreen> splash;

};
