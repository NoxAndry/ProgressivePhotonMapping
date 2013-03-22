#pragma once

#include "GeometryLib.h"
#include "LoadBMP.h"

//#define SUPERSAMPLING
//#define ERROR_CORRECTION
//#define ELLIPSE_CORRECTION
#define SHAPE_CORRECTION

struct Color
{
	float R,G,B;

	Color():R(0),G(0),B(0)
	{}

	Color(const float &as, const float &b,const float &c)
		:R(as),G(b),B(c)
	{
	}

	Color ScaledForOpenGL() const
	{
		return Color(std::min(1.f,R),
			std::min(1.f,G),
			std::min(1.f,B));
	}

	Color operator=(const Color &s)
	{
		if(&s == this)
			return *this;

		R = s.R;
		B = s.B;
		G = s.G;

		return *this;
	}

	Color operator*(const float &s) const
	{
		return Color(R * s, G * s, B * s);
	}

	Color operator/(const float &s) const
	{
		return Color(R / s, G / s, B / s);
	}

	Color operator*(const Color &s) const
	{
		return Color(R * s.R, G * s.G, B * s.B);
	}

	double ChanelsSum() const
	{
		return R + G + B;
	}

	Color operator+(const Color &s) const
	{
		return Color(R + s.R, G + s.G, B + s.B);
	}

	Color operator-() const
	{
		return Color(1.f - R, 1.f - G, 1.f - B);
	}

	Color operator+=(const Color &s) 
	{
		R += s.R;
		G += s.G;
		B += s.B;

		return *this;
	}
};

class KDPhotonsTree;

struct HitPoint
{
	HitPoint(const Point& position, const int x, const int y, const Color& summationCoeficient, const Color& ambient,const Color& specular, const Color& textureCoeficent, size_t shapeIndex, const Vector& normal);
	void UpdateHitPoint(KDPhotonsTree* photonsTree);

	Color CalculateRadiance()
	{
#define PHONG

#ifdef PHONG
		return (Radiance.ScaledForOpenGL()*scalingCoeficient + ambient + specular*GLOBAL_PROPERTIES::SamplingCoefficient) + textureCoeficent*0.4f;
		//return textureCoeficent;
#else
		return Radiance.ScaledForOpenGL()*scalingCoeficient + ambient;
#endif
	}

	Point position;
	Color scalingCoeficient;
	int x,y;
	Color ambient;
	Color specular;
	Vector Normal;

	float Radius;
	size_t NumberPhotons;
	Color Flux;
	Color Radiance;
	Color textureCoeficent;
	size_t shapeIndex;
};

typedef Color Ambient;
typedef Color Diffuse;
typedef Color Specular;
typedef Color Refract;
typedef Color Reflect;

struct DirectedPhoton
{
	DirectedPhoton()
		: position(0,0,0), direction(0,0,0), hitNumber(0)
	{}

	DirectedPhoton(const Point& pos, const Vector& dir, const Color& col, int s)
		: position(pos), power(col), direction(dir), hitNumber(s)
	{}

	DirectedPhoton(const Ray& pos, const Color& col)
		: position(pos.GetStartPoint()), power(col), direction(pos.GetDirection()), hitNumber(0)
	{}

	Point position;
	Color power;
	Vector direction;
	int hitNumber;
};


struct Photon
{
	Photon(const DirectedPhoton &s, const size_t index)
		: position(s.position), power(s.power), shapeIndex(index)
	{}
	Photon()
		: position(0,0,0)
	{}
	Point position;
	Color power;
	size_t shapeIndex;
};

struct Texture
{
	IMAGE image;

	Texture(const char *str)
	{
		if(!LoadBMP(str, &image))
			throw Error("Fail in loading texture");
	}

	Color GetTextureCoeficent(const TextureCoords coords) const
	{
		int u = (int)(coords.u()*image. width);
		int v = (int)(coords.v()*image.height);

		while(u<0)u+=image. width;
		while(v<0)v+=image.height;

		u %= image. width;
		v %= image.height;

		int pointer = v*image.width + u;

		int r = *(image.data + pointer*3 + 0);
		int g = *(image.data + pointer*3 + 1);
		int b = *(image.data + pointer*3 + 2);

		return Color(r/255.f,g/255.f,b/255.f);
	}
};

struct Material
{
	Material(const Ambient &amb, const Diffuse &dif, const Specular &spc, const Reflect &refl = Color(0,0,0),const Refract &refr = Color(0,0,0), double irc = 0, double erc  = 0):
		ambient(amb),specular(spc),diffuse(dif),transmit(refr),reflect(refl),internalRefCoeficient(irc),externalRefCoeficient(erc),
		spcc(Color(std::max(spc.R, refl.R), std::max(spc.G, refl.G), std::max(spc.B, refl.B)))
	{
		isRefl = reflect.R > GLOBAL_PROPERTIES::EPS || reflect.B > GLOBAL_PROPERTIES::EPS || reflect.G > GLOBAL_PROPERTIES::EPS;
		isRefr = transmit.R > GLOBAL_PROPERTIES::EPS || transmit.B > GLOBAL_PROPERTIES::EPS || transmit.G > GLOBAL_PROPERTIES::EPS;
		isDiff = diffuse.R > GLOBAL_PROPERTIES::EPS || diffuse.B > GLOBAL_PROPERTIES::EPS || diffuse.G > GLOBAL_PROPERTIES::EPS;
		isSpec = spcc.R > GLOBAL_PROPERTIES::EPS|| spcc.B > GLOBAL_PROPERTIES::EPS|| spcc.G> GLOBAL_PROPERTIES::EPS;

		Pr = std::max(std::max(diffuse.R + spcc.R + transmit.R,
			diffuse.G + spcc.G + transmit.G),
			diffuse.B + spcc.B + transmit.B);

		Pd = (diffuse.ChanelsSum())/(diffuse.ChanelsSum() + spcc.ChanelsSum() + transmit.ChanelsSum())*Pr;
		Ps = (spcc.ChanelsSum())/(diffuse.ChanelsSum() + spcc.ChanelsSum() + transmit.ChanelsSum())*Pr;
		Pt = Pr-Pd-Ps;
	}
	double Pr,Pd,Ps,Pt;

	Ambient ambient;
	Diffuse diffuse;
	Specular specular;
	Color spcc;
	
	Refract transmit;
	Reflect reflect;

	double internalRefCoeficient;
	double externalRefCoeficient;
	
	bool IsReflecting() const
	{
		return isRefl;
	}

	bool IsRefracting() const 
	{
		return isRefr;
	}

	bool IsDiffuse() const 
	{
		return isDiff;
	}

	bool IsSpecular() const 
	{
		return isSpec;
	}

private:
	bool isRefl;
	bool isRefr;
	bool isDiff;
	bool isSpec;
	
};

typedef Color Pixel;