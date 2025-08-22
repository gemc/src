#pragma once

// gemc
#include "gbase.h"

// gfactory
#include "gfactory_options.h"

class Shape : public GBase<Shape> {
public:
	~Shape() override = default;

	explicit Shape(const std::shared_ptr<GOptions>& g) : GBase(g, PLUGIN_LOGGER) {
	}

	virtual void Area() = 0;

};

class Triangle : public Shape {
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using Shape::Shape;
	void Area();
};


class Box : public Shape {
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using Shape::Shape;
	void Area();
};
