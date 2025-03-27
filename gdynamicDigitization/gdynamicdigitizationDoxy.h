/**
 *
 * \mainpage GDynamicDigitization Module
 *
 * \section intro_sec Introduction
 * The GDynamicDigitization module defines the abstract base interface for dynamic
 * digitization in the simulation. It provides functionality for processing steps,
 * modifying touchable objects, collecting true hit information, and digitizing hits.
 *
 * \section details_sec Details
 * The module includes:
 *  - GDynamicDigitization: an abstract base class that defines the core digitization
 *    interface. Derived classes (plugins) must implement defineReadoutSpecs(), and
 *    may override processStepTime(), processTouchable(), digitizeHit(), loadConstants(),
 *    and loadTT().
 *  - GTouchableModifiers: a helper class to manage modifier weights for touchable objects.
 *
 * \section usage_sec Usage
 * - Use set_loggers() to initialize the loggers.
 * - Call instantiate() to dynamically load a derived factory from a dynamic library.
 * - Use processTouchable() to update GTouchable objects based on step time.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */