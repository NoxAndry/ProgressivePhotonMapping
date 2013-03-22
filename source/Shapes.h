#pragma once

#include "TraceLib.h"

struct BoundingBox
{
	Point minimum,maximum;

	BoundingBox(Point mini,Point maxi)
		: minimum(mini),maximum(maxi) {}	
};

class Shape
{
public:
	virtual double Intersect(const Ray &ray) const = 0;
	virtual Vector GetNormal(const Point &sphPoint) const  = 0;
	virtual Material* GetMaterial(Point &z) const = 0;
	virtual Texture*  GetTexture(Point &z) const = 0;
	virtual TextureCoords CalculateTextureCoordinates(const Ray &ray) const = 0;
	virtual	bool IsFromExternal(const Ray &,const Point &) const = 0;

	virtual ~Shape() {};
	Shape(const BoundingBox &bb, const size_t index) : BBox(bb), index(index) {}

	BoundingBox BBox;
	size_t index;

	static size_t GetUniqueID()
	{
		static size_t n = 0;
		return ++n;
	}
};

class Triangle
	: public Shape
{
public:
	Triangle(const Point& a, const Point& b, const Point& c,Material* m, size_t index,
			Texture *texture = NULL, TextureCoords tA = TextureCoords(0,0), TextureCoords tB = TextureCoords(0,0), TextureCoords tC = TextureCoords(0,0))
		:Normal(!((a-b)*(a-c))), A(a), B(b), C(c), material(m),texture(texture), tA(tA), tB(tB), tC(tC),
			Shape( BoundingBox(	Point(std::min(std::min(a.x(), b.x()), c.x()), std::min(std::min(a.y(), b.y()), c.y()), std::min(std::min(a.z(), b.z()), c.z()) ),
								Point(std::max(std::max(a.x(), b.x()), c.x()), std::max(std::max(a.y(), b.y()), c.y()), std::max(std::max(a.z(), b.z()), c.z()) ) ),
								index)
	{ }

	static bool IsValidTrangle(const Point& a, const Point& b, const Point& c);
	double Intersect(const Ray &ray) const;
	Vector GetNormal(const Point& ) const;
	Material* GetMaterial(Point &) const;
	Texture* GetTexture(Point &) const;
	TextureCoords CalculateTextureCoordinates(const Ray &ray) const;
	bool IsFromExternal(const Ray& coming,const Point &) const;
	

private:
	const Point A,B,C;
	const TextureCoords tA, tB, tC; 

	const Vector Normal;
	Material* material;
	Texture *texture;
};

class Sphere
	: public Shape
{
public:
	Sphere(const Point &c,const double &R, Material* mat, size_t index, Texture *text = NULL)
		:center(c),Radius(R),material(mat), texture(text),
		Shape( BoundingBox (	center - Point(Radius, Radius, Radius),
								center + Point(Radius, Radius, Radius) ), index )
	{
	}

	double Intersect(const Ray &ray) const;
	bool IsFromExternal(const Ray& coming,const Point &bestMatch) const;
	Vector GetNormal(const Point &sphPoint) const;
	Material* GetMaterial(Point&) const;
	Texture* GetTexture(Point &) const;
	TextureCoords CalculateTextureCoordinates(const Ray&) const;

private:
	const Point center;
	const double Radius;
	Material* material;
	Texture * texture;
};

class Plane 
{
public:
	Plane(const double a, const double b, const double c, const double d)
		: A(a), B(b), C(c), D(d)
	{
		if(
			abs(a) < GLOBAL_PROPERTIES::EPS &&
			abs(b) < GLOBAL_PROPERTIES::EPS &&
			abs(c) < GLOBAL_PROPERTIES::EPS 
		)
			throw Error("wrong plane coords, all zeros");
	}

	double Intersect_param(const Ray &ray) const {
		Point planePoint(A, B, C);
		double d = planePoint % ray.GetDirection();

		if(abs(d) < GLOBAL_PROPERTIES::EPS)return (-GLOBAL_PROPERTIES::INFINITY);

		return -(planePoint % ray.GetStartPoint() + D)/d;
	}

	Point Intersect(const Ray &ray) const  {
		Point planePoint(A, B, C);
		double d = planePoint % ray.GetDirection();

		if(abs(d) < GLOBAL_PROPERTIES::EPS)return Point((-GLOBAL_PROPERTIES::INFINITY), (-GLOBAL_PROPERTIES::INFINITY), (-GLOBAL_PROPERTIES::INFINITY));

		return ray.GetPointOnRay( -(planePoint % ray.GetStartPoint() + D)/d );
	}

private:
	double A, B, C, D;
};