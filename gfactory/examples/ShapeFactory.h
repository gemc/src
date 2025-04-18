#ifndef FIRSTFACTORY_H
#define FIRSTFACTORY_H 1

class Shape {
public:
	virtual void Area() = 0;
	virtual      ~Shape() = default;

};

class Triangle : public Shape {
public:
	void Area();
};


class Box : public Shape {
public:
	void Area();
};


#endif // FIRSTFACTORY_H
