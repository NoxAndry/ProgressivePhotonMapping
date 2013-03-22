#pragma once
#include "GeometryLib.h"
#include "TraceLib.h"
#include "Shapes.h"

class LightSource
{
public:
	virtual const Ray GetRay() const = 0;
	virtual const Point GetCenter() const = 0;

	const Color GetPower() const
	{
		return color;
	}

	const float GetShiness() const 
	{ 
		return shiness;
	}

	LightSource(const Color& col, const float shiness, Shape* bBox) 
		: boundingShape(bBox), color(col), shiness(shiness){}

protected:
	Shape* boundingShape;
	Color color;
	float shiness;
};

class SphereLight 
	: public LightSource
{
public:
	SphereLight(const Point& pos, const double& rad,const Color& color,const float shiness)
		:  LightSource(color, shiness, new Sphere(pos, rad,new Material(color, color, Color(0,0,0)), Shape::GetUniqueID()  )),
			position(pos)
	{
	}

	const Point GetCenter() const
	{
		return position;
	}

	const Ray GetRay() const
	{
		return Ray(position, Vector::GetRandomVector());
	}

private:
	Point position;
};