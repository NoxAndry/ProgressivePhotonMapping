#pragma once

#include <cstdio>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <vector>
#include <cstdlib>
#include "Global_properties.h"
#include "Error.h"

#undef min
#undef max


class Vector
{
	double coords[3];

public:
	inline const double x() const {return coords[0];}
	inline const double y() const {return coords[1];}
	inline const double z() const {return coords[2];}

	inline void x(const double &a) {coords[0] = a;}
	inline void y(const double &a) {coords[1] = a;}
	inline void z(const double &a) {coords[2] = a;}

	Vector(const Vector &toCopy)
	{
		coords[0] = toCopy.coords[0];
		coords[1] = toCopy.coords[1];
		coords[2] = toCopy.coords[2];
	}

	Vector(const double a, const double b, const double c)
	{
		coords[0] = a;
		coords[1] = b;
		coords[2] = c;
	}

	inline const Vector operator=(const Vector &in)
	{
		if (&in == this)
			return (*this);
		coords[0] = in.coords[0];
		coords[1] = in.coords[1];
		coords[2] = in.coords[2];
		return *this;
	}

	// нормализация
	inline const Vector operator!() const 
	{
		return (*this)/(~(*this));
	}

	inline const Vector operator-() const 
	{
		return Vector(0,0,0) - (*this);
	}

	bool operator==(const Vector &in)  const 
	{
		return  fabs(coords[0] - in.coords[0]) < GLOBAL_PROPERTIES::EPS &&
				fabs(coords[1] - in.coords[1]) < GLOBAL_PROPERTIES::EPS &&
				fabs(coords[2] - in.coords[2]) < GLOBAL_PROPERTIES::EPS ;
	}

	// длина вектора
	inline const double operator~() const
	{
		return sqrt(coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2]);
	}

	// квадрат длины
	inline const double getSqLength() const
	{
		return coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2];
	}

	// векторное произведение
	inline const Vector operator*(const Vector &in) const
	{
		return Vector(coords[1]*in.coords[2] - coords[2]*in.coords[1],
					  coords[2]*in.coords[0] - coords[0]*in.coords[2],
					  coords[0]*in.coords[1] - coords[1]*in.coords[0]);
	}

	inline const Vector operator*(const double in) const
	{
		return Vector (coords[0]*in,coords[1]*in,coords[2]*in);
	}

	inline const Vector operator/(const double in) const
	{
		if (abs(in) < GLOBAL_PROPERTIES::EPS)
			throw  Error("Dividing by zero in vector");
		return Vector(coords[0]/in,coords[1]/in,coords[2]/in);
	}

	// скалярное произведение
	inline const double operator%(const Vector &in) const
	{
		return coords[0]*in.coords[0] + coords[1]*in.coords[1] + coords[2]*in.coords[2];
	}

	inline const Vector operator+(const Vector &in) const
	{
		return Vector (coords[0]+in.coords[0],coords[1]+in.coords[1],coords[2]+in.coords[2]);
	}

	inline const Vector operator+=(const Vector &in)
	{
		coords[0]+=in.coords[0];
		coords[1]+=in.coords[1];
		coords[2]+=in.coords[2];
		return *this;
	}

	inline const Vector operator-(const Vector &in) const
	{
		return Vector (coords[0]-in.coords[0],coords[1]-in.coords[1],coords[2]-in.coords[2]);
	}

	inline const Vector operator-=(const Vector &in)
	{	coords[0]-=in.coords[0];
		coords[1]-=in.coords[1];
		coords[2]-=in.coords[2];
		return *this;
	}

	// расстояние
	inline const double operator^(const Vector &in) const 
	{
		return ~(in - (*this));
	}

	inline const double CoordSum() const
	{
		return  coords[0] + coords[1]+ coords[2];
	}

	static const Vector GetRandomVector() {
		double x, y, z;

		do {
			x = Vector::GetRandomDouble(-1,1);
			y = Vector::GetRandomDouble(-1,1);
			z = Vector::GetRandomDouble(-1,1);
		} while(x*x + y*y + z*z > 1);

		return !Vector(x, y, z);
	}

private:
	static const double GetRandomDouble(double left, double right) 
	{
		return ((rand()^rand())/(double)RAND_MAX)*(right - left) + left;
	}
};

struct TextureCoords
{
	double coords[2];

	inline const double u() const {return coords[0];}
	inline const double v() const {return coords[1];}

	inline void u(const double a) {coords[0] = a;}
	inline void v(const double a) {coords[1] = a;}

	TextureCoords(const double a, const double b)
	{
		coords[0] = a;
		coords[1] = b;
	}

	inline const TextureCoords operator+(const TextureCoords& a) const
	{
		return TextureCoords(u() + a.u(), v() + a.v());
	}

	inline const TextureCoords operator-(const TextureCoords& a) const
	{
		return TextureCoords(u() - a.u(), v() - a.v());
	}

	inline const TextureCoords operator*=(const double& d)
	{
		coords[0] *= d; 
		coords[1] *= d; 
		return *this;
	}

	void InvertU()
	{
		u(1 - u());
	}

	void InvertV()
	{
		v(1 - v());
	}
};

typedef Vector Point;

struct Ray
{
	Ray(const Point &f1,const Vector &dir,const double mas = 0)
		:startPoint(f1),direction(dir),mass(mas)
	{
	}

	const double GetMass() const
	{
		return mass;
	}

	void SetMass(const double m)
	{
		mass = m;
	}

	const double Distance(const Point &toComp) const
	{
		if ((toComp - startPoint)%direction < 0)
			return toComp^startPoint;
		else
			return (~((toComp - startPoint)*direction))/(~direction);
			 
	}
	
	const Point GetPointOnRay(const double t) const
	{
		return direction*t + startPoint;
	}

	const Point GetStartPoint() const
	{
		return startPoint;
	}

	const Vector GetDirection() const
	{
		return direction;
	}

private:
	Point startPoint;
	Vector direction;
	double mass;
};

