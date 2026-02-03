#pragma once

// gemc
#include "gbase.h"

// gfactory
#include "gfactory_options.h"

/**
 * @class Shape
 * @brief Example base class used to demonstrate *static* factory registration.
 *
 * In the example program, concrete shapes are registered directly with \ref GManager "GManager"
 * because the application has compile-time access to both base and derived types.
 */
class Shape : public GBase<Shape>
{
public:
	/// Virtual destructor for polymorphic deletion.
	~Shape() override = default;

	/**
	 * @brief Construct the base with a shared options/configuration instance.
	 *
	 * @param g Shared options object used to initialize logging via `GBase`.
	 */
	explicit Shape(const std::shared_ptr<GOptions>& g) : GBase(g, PLUGIN_LOGGER) {
	}

	/**
	 * @brief Compute or report the shape area (example API).
	 *
	 * The example implementations print to stdout.
	 */
	virtual void Area() = 0;
};

/**
 * @class Triangle
 * @brief Example concrete `Shape` implementation used in static registration.
 */
class Triangle : public Shape
{
public:
	/// Inherit the base (const std::shared_ptr<GOptions>&) constructor.
	using Shape::Shape;

	/// Example implementation that prints a message.
	void Area();
};

/**
 * @class Box
 * @brief Example concrete `Shape` implementation used in static registration.
 */
class Box : public Shape
{
public:
	/// Inherit the base (const std::shared_ptr<GOptions>&) constructor.
	using Shape::Shape;

	/// Example implementation that prints a message.
	void Area();
};
