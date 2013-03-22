#include "Shapes.h"


////////////////////////////////////////////////////////
// Triangle
////////////////////////////////////////////////////////
bool Triangle::IsValidTrangle(const Point& a, const Point& b, const Point& c)
{
	return ((a-b)*(a-c)).getSqLength() > GLOBAL_PROPERTIES::EPS*GLOBAL_PROPERTIES::EPS;
}

double Triangle::Intersect(const Ray &ray) const 
{
	Point E1 = B - A;
	Point E2 = C - A;
	Point T = ray.GetStartPoint() - A;
	Point D = ray.GetDirection();
	Point P = D * E2;
	Point Q = T * E1;

	double coef = P % E1;
	if(abs(coef) < GLOBAL_PROPERTIES::EPS)return (-GLOBAL_PROPERTIES::INFINITY);
	coef = 1 / coef;

	double u = (P % T) * coef;
	if(u<0 || u>1)return (-GLOBAL_PROPERTIES::INFINITY);

	double v = (Q % D) * coef;
	if(v<0 || v>1)return (-GLOBAL_PROPERTIES::INFINITY);
		
	double t1 = 1 - u - v;
	if(t1<0 || t1>1)return (-GLOBAL_PROPERTIES::INFINITY);
		
	return (Q % E2) * coef;
}

Vector Triangle::GetNormal(const Point& ) const 
{
	return Normal;
}

Material* Triangle::GetMaterial(Point &) const
{
	return material;
}

Texture* Triangle::GetTexture(Point &) const
{
	return texture;
}
	
TextureCoords Triangle::CalculateTextureCoordinates(const Ray &ray) const
{
	Point E1 = B - A;
	Point E2 = C - A;
	Point T = ray.GetStartPoint() - A;
	Point D = ray.GetDirection();
	Point P = D * E2;
	Point Q = T * E1;

	double coef = 1 / (P % E1);

	double u = (P % T) * coef;
	double v = (Q % D) * coef;

	TextureCoords tE = tB - tA;
	TextureCoords tD = tC - tA;

	double S = (tE.u()*tD.v() - tE.v()*tD.u())/2;

	if(abs(tD.u()) > GLOBAL_PROPERTIES::EPS)
	{
		double Vu = tA.u() + (u*tE.u() + v*tD.u());
		double Vv = tA.v() + ((Vu - tA.u())*tD.v() - 2*u*S) / tD.u();

		return TextureCoords(Vu,Vv);
	}
	else 
	{
		double Vu = 2*u*S / tD.v();
		double Vv = (2*v*S + tE.v()*Vu)/tE.u();

		return TextureCoords(Vu,Vv);
	}
}

bool Triangle::IsFromExternal(const Ray& coming,const Point &) const 
{
	if (Normal % (-coming.GetDirection()) >  GLOBAL_PROPERTIES::EPS)
		return true;
	return false;
}
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// Sphere
////////////////////////////////////////////////////////
double Sphere::Intersect(const Ray &ray) const
{
	Point d = ray.GetStartPoint() - center;

	double B = d % (ray.GetDirection()*2);
	double C = d % d - Radius*Radius;

	double D = B*B - 4*C;
	if(D < 0.0)return (-GLOBAL_PROPERTIES::INFINITY);
	D = sqrt(D);

	double t1 = (-B + D)/2.0;
	double t2 = (-B - D)/2.0;

	double t = std::min(t1, t2);
	if(t > GLOBAL_PROPERTIES::EPS)
		return t;

	t = std::max(t1, t2);

	return t > GLOBAL_PROPERTIES::EPS ? (-GLOBAL_PROPERTIES::INFINITY) : t;
}

bool Sphere::IsFromExternal(const Ray& coming,const Point &bestMatch)const 
{
	if (GetNormal(bestMatch)%(-!coming.GetDirection()) > GLOBAL_PROPERTIES::EPS )
		return true;
		
	return false;
}

Vector Sphere::GetNormal(const Point &sphPoint) const 
{
	return !(sphPoint - center);
}

Material* Sphere::GetMaterial(Point&) const
{
	return material;
}

Texture* Sphere::GetTexture(Point &) const
{
	return texture;
}

TextureCoords Sphere::CalculateTextureCoordinates(const Ray&) const
{
	throw Error("unimplemented error");
}
////////////////////////////////////////////////////////