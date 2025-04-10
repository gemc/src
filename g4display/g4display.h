#ifndef G4DISPLAY_H
#define G4DISPLAY_H 1

// Standard Library
#include <memory> // Required for std::unique_ptr

// gemc
#include "glogger.h" // Provides logging capabilities
#include "goptions.h" // Provides application options/configuration

// qt
#include <QtWidgets/QTabWidget> // Base class for the display widget
#include <QtWidgets/QWidget>   // Required for QWidget parent parameter


/**
 * @brief The main widget for controlling Geant4 visualization.
 * @class G4Display
 *
 * This class acts as a container (using QTabWidget) for different panels
 * that control various aspects of the Geant4 visualization, such as camera,
 * lighting, slicing, and other utilities. It owns the primary logger instance
 * for the display module.
 */
class G4Display : public QWidget {

public:
    /**
     * @brief Constructs the G4Display widget.
     * @param gopt Pointer to the GOptions object containing application settings. Used to configure the logger.
     * @param parent Optional parent QWidget for Qt's memory management.
     */
    explicit G4Display(GOptions *gopt, QWidget *parent = nullptr);

    /**
     * @brief Destructor. Manages the lifetime of owned resources (like the logger).
     */
    ~G4Display() override; // Use override for virtual destructors

    // Making the logger accessible (e.g., to child widgets) might be useful,
    // but passing it during construction (as currently done) is often cleaner.
    // If access is needed later, provide a getter:
    // GLogger* getLogger() const { return log.get(); }

private:
    // Using a smart pointer (unique_ptr) for automatic memory management of the logger.
    // This follows the RAII (Resource Acquisition Is Initialization) principle.
    std::unique_ptr<GLogger> log;

    // Disable copy constructor and assignment operator to prevent slicing
    // and issues with unique_ptr ownership.
    G4Display(const G4Display&) = delete;
    G4Display& operator=(const G4Display&) = delete;
};

#endif // G4DISPLAY_H